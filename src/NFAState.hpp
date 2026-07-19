#include "ASTNode.hpp"
#include "Unicode.hpp"
#include <cstddef>
#include <format>
#include <memory>
#include <stdexcept>
#include <unordered_set>
#include <utility>
#include <vector>

enum STATE_TYPE { CHAR, EPSILON, END };

inline size_t NFASTATE_ID = 0;

class NFAState {
  private:
    size_t id_;
    size_t ast_id_;
    STATE_TYPE type_;
    char32_t accept_ch_;

  public:
    NFAState *next1_ = nullptr;
    NFAState *next2_ = nullptr;
    NFAState(size_t ast_id) : ast_id_(ast_id), type_(EPSILON) {
        id_ = NFASTATE_ID++;
    };
    NFAState(size_t ast_id, STATE_TYPE type) : ast_id_(ast_id), type_(type) {
        id_ = NFASTATE_ID++;
    };
    NFAState(size_t ast_id, char32_t accept_ch)
        : ast_id_(ast_id), type_(CHAR), accept_ch_(accept_ch) {
        id_ = NFASTATE_ID++;
    };

    std::string nodeName() const {
        switch (type_) {
        case CHAR:
            return std::format("({}.{}).{}", id_, u32_to_utf8(accept_ch_),
                               ast_id_);
        case EPSILON:
            return std::format("({}).{}", id_, ast_id_);
        case END:
            return "(END)";
        default:
            return "";
        }
    }

    void print() const {
        std::unordered_set<size_t> printed;
        print(printed);
    }

    void print(std::unordered_set<size_t> printed) const {
        std::cout << nodeName();
        if (printed.contains(id_)) {
            std::cout << "--->(LOOP)";
            return;
        } else {
            printed.insert(id_);
        }

        if (next1_) {
            std::cout << "--->";
            next1_->print(printed);
        }

        if (next2_) {
            std::cout << std::endl;
            std::cout << nodeName();
            std::cout << "--->";
            next2_->print(printed);
        }
    }

    friend class NFABuilder;
};

struct NFAFragment {
    NFAState *entry = nullptr;
    NFAState *exit = nullptr;
};

class NFABuilder {
  private:
    ASTNode &root_;
    std::vector<std::unique_ptr<NFAState>> all_states_;

  public:
    NFABuilder(ASTNode &root) : root_(root) {}

    NFAFragment build() {
        auto end = std::make_unique<NFAState>(0, END);
        NFAState *end_ptr = end.get();

        NFAFragment frag = build(root_);
        frag.exit->next1_ = end.get();
        all_states_.push_back(std::move(end));

        return {frag.entry, end_ptr};
    }

    NFAFragment build(ASTNode &node) {
        switch (node.type) {
        case CONTACT_NODE: {
            ContactNode &contact_node = static_cast<ContactNode &>(node);
            NFAFragment first_frag;
            NFAFragment last_frag;

            for (int i = 0; i < contact_node.size(); i++) {
                ASTNode &current_node = contact_node.getChild(i);
                NFAFragment current_frag = build(current_node);
                if (i == 0) {
                    first_frag = current_frag;
                } else {
                    last_frag.exit->next1_ = current_frag.entry;
                }
                last_frag = current_frag;
            }
            return {first_frag.entry, last_frag.exit};
        }
        case ALTER_NODE: {
            AlterNode &alter_node = static_cast<AlterNode &>(node);
            auto entry = std::make_unique<NFAState>(alter_node.getId());
            auto exit = std::make_unique<NFAState>(alter_node.getId());
            NFAState *entry_ptr = entry.get();
            NFAState *exit_ptr = exit.get();

            NFAFragment left_frag = build(alter_node.getLeft());
            NFAFragment right_frag = build(alter_node.getRight());

            entry_ptr->next1_ = left_frag.entry;
            entry_ptr->next2_ = right_frag.entry;
            left_frag.exit->next1_ = exit_ptr;
            right_frag.exit->next1_ = exit_ptr;

            all_states_.push_back(std::move(entry));
            all_states_.push_back(std::move(exit));
            return {entry_ptr, exit_ptr};
        }
        case REPEAT_NODE: {
            RepeatNode &repeat_node = static_cast<RepeatNode &>(node);
            auto entry = std::make_unique<NFAState>(repeat_node.getId());
            auto exit = std::make_unique<NFAState>(repeat_node.getId());
            NFAState *entry_ptr = entry.get();
            NFAState *exit_ptr = exit.get();

            NFAFragment child_frag = build(repeat_node.getChild());
            entry_ptr->next1_ = child_frag.entry;
            child_frag.exit->next1_ = child_frag.entry;
            child_frag.exit->next2_ = exit_ptr;

            all_states_.push_back(std::move(entry));
            all_states_.push_back(std::move(exit));
            return {entry_ptr, exit_ptr};
        }
        case CHAR_NODE: {
            CharNode &char_node = static_cast<CharNode &>(node);
            auto unique_ptr = std::make_unique<NFAState>(char_node.getId(),
                                                         char_node.getChar());
            NFAState *ptr = unique_ptr.get();
            all_states_.push_back(std::move(unique_ptr));
            return {ptr, ptr};
        }
        case GROUP_NODE: {
            GroupNode &group_node = static_cast<GroupNode &>(node);
            auto entry = std::make_unique<NFAState>(group_node.getId());
            auto exit = std::make_unique<NFAState>(group_node.getId());
            NFAState *entry_ptr = entry.get();
            NFAState *exit_ptr = exit.get();

            NFAFragment child_frag = build(group_node.getChild());
            entry_ptr->next1_ = child_frag.entry;
            child_frag.exit->next1_ = exit_ptr;

            all_states_.push_back(std::move(entry));
            all_states_.push_back(std::move(exit));
            return {entry_ptr, exit_ptr};
        }
        default:
            throw std::runtime_error("Unexpected AST node");
        }
    }

    void exportToDot(std::ostream &out) {
        out << "digraph NFA {\n";
        out << "  rankdir=LR;\n";
        out << "  node [shape=circle];\n";

        for (const auto &ptr : all_states_) {
            NFAState *s = ptr.get();

            out << std::format("  {} [label=\"{}\"];\n", s->id_, s->nodeName());

            if (s->next1_) {
                out << std::format("  {} -> {};\n", s->id_, s->next1_->id_);
            }
            if (s->next2_) {
                out << std::format("  {} -> {};\n", s->id_, s->next2_->id_);
            }
        }
        out << "}\n";
    }
};

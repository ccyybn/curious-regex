#include <cstddef>
#include <format>
#include <memory>
#include <stdexcept>
#include <unordered_set>
#include <utility>
#include <vector>

#include "ASTNode.hpp"
#include "Unicode.hpp"

enum STATE_TYPE { CHAR, EPSILON, END };

inline size_t NFASTATE_ID = 0;

class NFAState {
   private:
    size_t id_;
    STATE_TYPE type_;
    char32_t accept_ch_;
    std::string name_;
    std::string ast_in_out_tag_;
    std::string ast_name_;

   public:
    NFAState* next1_ = nullptr;
    NFAState* next2_ = nullptr;
    NFAState(STATE_TYPE type, char32_t accept_ch) : type_(type), accept_ch_(accept_ch) {
        id_ = NFASTATE_ID++;
        switch (type) {
            case CHAR:
                name_ = std::format("({} Char: {})", id_, u32_to_utf8(accept_ch_));
                break;
            case EPSILON:
                name_ = std::format("({})", id_);
                break;
            case END:
                name_ = "(END)";
                break;
        }
    };

    NFAState(char32_t accept_ch) : NFAState(CHAR, accept_ch) {};

    NFAState() : NFAState(EPSILON, 0) {};

    STATE_TYPE getType() { return type_; }

    char32_t getAcceptChar() { return accept_ch_; }

    NFAState* setASTEntryInfo(const ASTNode& node) {
        ast_name_ = node.getName();
        ast_in_out_tag_ = "IN";
        return this;
    }

    NFAState* setASTExitInfo(const ASTNode& node) {
        ast_name_ = node.getName();
        ast_in_out_tag_ = "OUT";
        return this;
    }

    NFAState* setASTInfo(const ASTNode& node) {
        ast_name_ = node.getName();
        return this;
    }

    std::string displayName(const std::string& seperator = "|") const {
        std::string display_name = name_;
        if (!ast_in_out_tag_.empty()) {
            if (seperator == "\\n") {
                display_name += seperator + "[" + ast_in_out_tag_ + "]";
            } else {
                display_name += seperator + ast_in_out_tag_;
            }
        }
        if (!ast_name_.empty()) {
            if (seperator == "\\n") {
                display_name += seperator + "[" + ast_name_ + "]";
            } else {
                display_name += seperator + ast_name_;
            }
        }
        return display_name;
    }

    void print() const {
        std::unordered_set<size_t> printed;
        print(printed);
    }

    void print(std::unordered_set<size_t> printed) const {
        std::cout << displayName();
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
            std::cout << displayName();
            std::cout << "--->";
            next2_->print(printed);
        }
    }

    friend class NFABuilder;
};

struct NFAFragment {
    NFAState* entry = nullptr;
    NFAState* exit = nullptr;
};

class NFABuilder {
   private:
    ASTNode& root_;
    std::vector<std::unique_ptr<NFAState>> all_states_;

   public:
    NFABuilder(ASTNode& root) : root_(root) {}

    NFAFragment build() {
        auto end = std::make_unique<NFAState>(END, 0);
        NFAState* end_ptr = end.get();

        NFAFragment frag = build(root_);
        frag.exit->next1_ = end.get();
        all_states_.push_back(std::move(end));

        return {frag.entry, end_ptr};
    }

    NFAFragment build(ASTNode& node) {
        switch (node.type) {
            case CONTACT_NODE: {
                ContactNode& contact_node = static_cast<ContactNode&>(node);
                if (contact_node.size() == 0) {
                    auto empty = std::make_unique<NFAState>();
                    NFAState* empty_ptr = empty.get()->setASTInfo(contact_node);
                    all_states_.push_back(std::move(empty));
                    return {empty_ptr, empty_ptr};
                }

                NFAFragment first_frag;
                NFAFragment last_frag;

                for (int i = 0; i < contact_node.size(); i++) {
                    ASTNode& current_node = contact_node.getChild(i);
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
                AlterNode& alter_node = static_cast<AlterNode&>(node);
                auto entry = std::make_unique<NFAState>();
                auto exit = std::make_unique<NFAState>();
                NFAState* entry_ptr = entry.get()->setASTEntryInfo(alter_node);
                NFAState* exit_ptr = exit.get()->setASTExitInfo(alter_node);

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
                RepeatNode& repeat_node = static_cast<RepeatNode&>(node);
                auto entry = std::make_unique<NFAState>();
                auto exit = std::make_unique<NFAState>();
                NFAState* entry_ptr = entry.get()->setASTEntryInfo(repeat_node);
                NFAState* exit_ptr = exit.get()->setASTExitInfo(repeat_node);

                NFAFragment child_frag = build(repeat_node.getChild());
                entry_ptr->next1_ = child_frag.entry;
                entry_ptr->next2_ = exit_ptr;

                child_frag.exit->next1_ = child_frag.entry;
                child_frag.exit->next2_ = exit_ptr;

                all_states_.push_back(std::move(entry));
                all_states_.push_back(std::move(exit));
                return {entry_ptr, exit_ptr};
            }
            case CHAR_NODE: {
                CharNode& char_node = static_cast<CharNode&>(node);
                auto unique_ptr = std::make_unique<NFAState>(char_node.getChar());
                NFAState* ptr = unique_ptr.get();
                all_states_.push_back(std::move(unique_ptr));
                return {ptr, ptr};
            }
            case GROUP_NODE: {
                GroupNode& group_node = static_cast<GroupNode&>(node);
                auto entry = std::make_unique<NFAState>();
                auto exit = std::make_unique<NFAState>();
                NFAState* entry_ptr = entry.get()->setASTEntryInfo(group_node);
                NFAState* exit_ptr = exit.get()->setASTExitInfo(group_node);

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

    void exportGraph(std::ostream& out) {
        out << "digraph NFA {\n";
        out << "  rankdir=LR;\n";
        out << "  node [shape=circle];\n";

        for (const auto& ptr : all_states_) {
            NFAState* s = ptr.get();
            out << std::format("  {} [label=\"{}\"];\n", s->id_, s->displayName("\\n"));
            if (s->next1_) {
                out << std::format("  {} -> {};\n", s->id_, s->next1_->id_);
            }
            if (s->next2_) {
                out << std::format("  {} -> {};\n", s->id_, s->next2_->id_);
            }
        }
        out << "}\n";

        out << std::endl;
        out << "Open this graphs drawing website and paste the above code: https://dreampuf.github.io/GraphvizOnline/?engine=dot";
        out << std::endl << std::endl;
    }
};

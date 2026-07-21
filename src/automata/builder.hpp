#pragma once
#include "nfa_state.hpp"

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
        link(frag.exit, end.get());
        all_states_.push_back(std::move(end));

        return {frag.entry, end_ptr};
    }

    void link(NFAState* current, NFAState* next) {
        if (current->next1_ == nullptr) {
            current->next1_ = next;
        } else if (current->next2_ == nullptr) {
            current->next2_ = next;
        } else {
            std::runtime_error("No pointer to link");
        }
    }

    NFAFragment build(ASTNode& node) {
        switch (node.type) {
            case CONTACT_NODE: {
                ContactNode& contact_node = static_cast<ContactNode&>(node);
                if (contact_node.size() == 0) {
                    auto empty = std::make_unique<NFAState>();
                    NFAState* empty_ptr = empty.get()->setAST(contact_node);
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
                        link(last_frag.exit, current_frag.entry);
                    }
                    last_frag = current_frag;
                }
                return {first_frag.entry, last_frag.exit};
            }
            case ALTER_NODE: {
                AlterNode& alter_node = static_cast<AlterNode&>(node);
                auto entry = std::make_unique<NFAState>();
                auto exit = std::make_unique<NFAState>();
                NFAState* entry_ptr = entry.get()->setIn(alter_node);
                NFAState* exit_ptr = exit.get()->setOut(alter_node);

                NFAFragment left_frag = build(alter_node.getLeft());
                NFAFragment right_frag = build(alter_node.getRight());

                entry_ptr->next1_ = left_frag.entry;
                entry_ptr->next2_ = right_frag.entry;
                link(left_frag.exit, exit_ptr);
                link(right_frag.exit, exit_ptr);

                all_states_.push_back(std::move(entry));
                all_states_.push_back(std::move(exit));
                return {entry_ptr, exit_ptr};
            }
            case REPEAT_NODE: {
                RepeatNode& repeat_node = static_cast<RepeatNode&>(node);
                auto entry = std::make_unique<NFAState>();
                auto exit = std::make_unique<NFAState>();
                NFAState* entry_ptr = entry.get()->setIn(repeat_node);
                NFAState* exit_ptr = exit.get()->setOut(repeat_node);

                NFAFragment child_frag = build(repeat_node.getChild());

                link(entry_ptr, child_frag.entry);
                link(entry_ptr, exit_ptr);
                link(child_frag.exit, child_frag.entry);
                link(child_frag.exit, exit_ptr);

                all_states_.push_back(std::move(entry));
                all_states_.push_back(std::move(exit));
                return {entry_ptr, exit_ptr};
            }
            case CHAR_NODE: {
                CharNode& char_node = static_cast<CharNode&>(node);
                auto entry = std::make_unique<NFAState>();
                auto exit = std::make_unique<NFAState>();
                auto unique_ptr = std::make_unique<NFAState>(char_node.getChar());
                NFAState* entry_ptr = entry.get()->setIn(char_node);
                NFAState* exit_ptr = exit.get()->setOut(char_node);
                NFAState* ptr = unique_ptr.get();
                entry_ptr->next1_ = ptr;
                ptr->next1_ = exit_ptr;
                all_states_.push_back(std::move(entry));
                all_states_.push_back(std::move(exit));
                all_states_.push_back(std::move(unique_ptr));
                return {entry_ptr, exit_ptr};
            }
            case GROUP_NODE: {
                GroupNode& group_node = static_cast<GroupNode&>(node);
                auto entry = std::make_unique<NFAState>();
                auto exit = std::make_unique<NFAState>();
                NFAState* entry_ptr = entry.get()->setIn(group_node);
                NFAState* exit_ptr = exit.get()->setOut(group_node);

                NFAFragment child_frag = build(group_node.getChild());
                link(entry_ptr, child_frag.entry);
                link(child_frag.exit, exit_ptr);

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

#include "ASTNode.hpp"
#include <cstddef>
#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>

enum STATE_TYPE { CHAR, EPSILON };

inline size_t NFASTATE_ID = 0;

class NFAState {
  private:
    size_t id_;
    STATE_TYPE type_;
    char32_t accept_ch_;

  public:
    NFAState *next1_ = nullptr;
    NFAState *next2_ = nullptr;
    NFAState() : type_(EPSILON) {};
    NFAState(char32_t accept_ch) : type_(CHAR), accept_ch_(accept_ch) {};
};

struct NFAFragment {
    NFAState *entry = nullptr;
    NFAState *exit = nullptr;
};

class NFABuilder {
  private:
    ASTNode &root_;
    std::vector<std::unique_ptr<NFAState>> all_states_;

    void link(NFAState *previous, NFAState *current) {
        if (previous->next1_ == nullptr) {
            previous->next1_ = current;
        } else if (previous->next2_ == nullptr) {
            previous->next2_ = current;
        } else {
            throw std::runtime_error(
                "No free pointer to link to the next state");
        }
    }

  public:
    NFABuilder(ASTNode &root) : root_(root) {}
    NFAState *build(ASTNode &node) {
        NFAState *entry_state;
        switch (node.type) {
        case CONTACT_NODE: {
            ContactNode &contact_node = static_cast<ContactNode &>(node);
            std::unique_ptr<NFAState> ptr = std::make_unique<NFAState>();

            NFAState *previous_state = nullptr;
            ASTNode *previous_node = nullptr;
            for (int i = 0; i < contact_node.size(); i++) {
                ASTNode &current_node = contact_node.getChild(i);
                NFAState *current_state = build(current_node);
                if (i == 0) {
                    ptr->next1_ = current_state;
                }
                if (previous_node != nullptr) {
                    if (previous_node->type == ALTER_NODE) {

                    } else {
                        link(previous_state, current_state);
                    }
                }
                previous_state = current_state;
                previous_node = &current_node;
            }

            all_states_.push_back(std::move(ptr));
            entry_state = all_states_.back().get();
            break;
        }
        case ALTER_NODE: {
            AlterNode &alter_node = static_cast<AlterNode &>(node);
            std::unique_ptr<NFAState> ptr = std::make_unique<NFAState>();
            ptr->next1_ = build(alter_node.getLeft());
            ptr->next2_ = build(alter_node.getRight());
            all_states_.push_back(std::move(ptr));
            entry_state = all_states_.back().get();
            break;
        }
        case REPEAT_NODE: {
            RepeatNode &repeat_node = static_cast<RepeatNode &>(node);
            std::unique_ptr<NFAState> ptr = std::make_unique<NFAState>();
            NFAState *child_state = build(repeat_node.getChild());
            ptr->next1_ = child_state;
            all_states_.push_back(std::move(ptr));
            entry_state = all_states_.back().get();
            child_state->next1_ = entry_state;
            break;
        }
        case CHAR_NODE: {
            CharNode &char_node = static_cast<CharNode &>(node);
            auto ptr = std::make_unique<NFAState>(char_node.getChar());
            all_states_.push_back(std::move(ptr));
            entry_state = all_states_.back().get();
            break;
        }
        default:
            throw std::runtime_error("Unexpected AST node");
        }
        return entry_state;
    }
};

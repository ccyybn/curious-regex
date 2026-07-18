#pragma once
#include "StringSlider.hpp"
#include "Unicode.hpp"
#include <format>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <vector>

enum NODE_TYPE { CONTACT_NODE, GROUP_NODE, ALTER_NODE, REPEAT_NODE, CHAR_NODE };

inline size_t NODE_ID = 0;

class ASTNode {
  protected:
    size_t id_;
    uint8_t indent_incr_ = 4;

  public:
    NODE_TYPE type;
    virtual ~ASTNode() = 0;
    virtual void print(uint8_t indent = 0) const {
        for (int i = 0; i < indent; i++) {
            std::cout << " ";
        }
    };

    // virtual bool match(StringSlider &str) = 0;
    // virtual std::vector<std::unique_ptr<NFAState>> toNFA() const = 0;
};

inline ASTNode::~ASTNode() {}

class RepeatNode : public ASTNode {
  private:
    std::unique_ptr<ASTNode> child_;
    // std::vector<size_t> revert_steps_;

  public:
    RepeatNode(std::unique_ptr<ASTNode> node) : child_(std::move(node)) {
        id_ = NODE_ID++;
        type = REPEAT_NODE;
    }

    ASTNode &getChild() const { return *child_.get(); }

    void print(uint8_t indent = 0) const override {
        ASTNode::print(indent);
        std::cout << std::format("[Repeat {}]: ", id_);
        std::cout << std::endl;
        child_->print(indent + indent_incr_);
    }

    // size_t pop_revert_step() {
    //     if (revert_steps_.empty()) {
    //         return -1;
    //     }
    //
    //     auto step_length = revert_steps_.back();
    //     revert_steps_.pop_back();
    //     return step_length;
    // }
    //
    // bool match(StringSlider &str) override {
    //     size_t cursor = str.cursor();
    //     int i = 0;
    //     while (str.hasMore() && child_->match(str)) {
    //         std::cout << "[" << id_ << std::format("] repeat match [{}", i++)
    //                   << "]" << std::endl;
    //         size_t step = str.cursor() - cursor;
    //         if (step == 0) {
    //             break;
    //         }
    //         revert_steps_.push_back(step);
    //         cursor = str.cursor();
    //     }
    //     return true;
    // }
    // std::vector<std::unique_ptr<NFAState>> toNFA() const override {
    //     std::vector<std::unique_ptr<NFAState>> states;
    //     NFAState child_state = child_->toNFA();
    //     NFAState repeat_state;
    //     repeat_state.next1_ = &child_state;
    //     child_state.next1_ = &repeat_state;
    //     return states;
    // }
};

class ContactNode : public ASTNode {
  private:
    std::vector<std::unique_ptr<ASTNode>> children_;

  public:
    ContactNode() {
        id_ = NODE_ID++;
        type = CONTACT_NODE;
    }

    void add(std::unique_ptr<ASTNode> node) {
        children_.push_back(std::move(node));
    }

    size_t size() { return children_.size(); }

    ASTNode &getChild(size_t index) const {
        if (index < children_.size()) {
            return *children_[index].get();
        } else {
            throw std::runtime_error(
                "ContactNode.getChild index > children_.size()");
        }
    }

    void print(uint8_t indent = 0) const override {
        ASTNode::print(indent);
        std::cout << std::format("[Contact {}]: ", id_);
        std::cout << std::endl;
        for (const auto &ptr : children_) {
            ptr->print(indent + indent_incr_);
        }
    }

    // NFAState toNFA() const override {
    //     NFAState entry_state;
    //     for (int i = 0; i < children_.size(); i++) {
    //         NFAState state;
    //     }
    //
    //     return entry_state;
    // }
    //
    // bool match(StringSlider &str) override { return matchNext(0, str); }
    //
    // bool matchNext(size_t next_child_index, StringSlider &str) const {
    //     std::cout << "[" << id_ << "]" << " match next: " << next_child_index
    //               << std::endl;
    //     auto &ptr = children_[next_child_index];
    //     auto remainStr = str.getRemainStr();
    //     std::cout << "0 has more: " << str.cursor() << std::endl;
    //     if (!ptr->match(str)) {
    //         return false;
    //     }
    //     std::cout << "1 has more: " << str.cursor() << std::endl;
    //     if (next_child_index + 1 < children_.size() &&
    //         !matchNext(next_child_index + 1, str)) {
    //         if (ptr->type == REPEAT_NODE) {
    //             if (RepeatNode *repeat_ptr =
    //                     dynamic_cast<RepeatNode *>(ptr.get())) {
    //                 while (!matchNext(next_child_index + 1, str)) {
    //                     auto revert_step = repeat_ptr->pop_revert_step();
    //                     std::cout
    //                         << std::format("[id:{}][next_childen_index:{}] "
    //                                        "revert step:{}",
    //                                        id_, next_child_index,
    //                                        revert_step)
    //                         << std::endl;
    //                     if (revert_step == -1) {
    //                         return false;
    //                     } else {
    //                         str.revert(revert_step);
    //                     }
    //                 }
    //                 std::cout << "true 1" << std::endl;
    //                 return true;
    //             } else {
    //                 ptr->print();
    //                 throw std::runtime_error("Cannot access to repeat node");
    //             }
    //
    //         } else {
    //             return false;
    //         }
    //     } else {
    //         std::cout << "true 2" << std::endl;
    //         return true;
    //     }
    // }
};

class GroupNode : public ASTNode {
  private:
    std::unique_ptr<ASTNode> child_;

  public:
    GroupNode(std::unique_ptr<ASTNode> child) : child_(std::move(child)) {
        id_ = NODE_ID++;
        type = GROUP_NODE;
    }

    ASTNode &getChild() const { return *child_.get(); }

    void print(uint8_t indent = 0) const override {
        ASTNode::print(indent);
        std::cout << std::format("[Group {}]: ", id_);
        std::cout << std::endl;
        child_->print(indent + indent_incr_);
    }

    // bool match(StringSlider &str) override { return child_->match(str); }
};

class CharNode : public ASTNode {
  private:
    char32_t ch_;

  public:
    CharNode(char32_t ch) : ch_(ch) {
        id_ = NODE_ID++;
        type = CHAR_NODE;
    }

    char32_t getChar() { return ch_; }

    void print(uint8_t indent = 0) const override {
        ASTNode::print(indent);
        std::cout << std::format("[Char {}]: ", id_);
        std::cout << u32_to_utf8(ch_) << std::endl;
    }

    // bool match(StringSlider &str) override {
    //     if (str.hasMore() && str.getChar() == ch_) {
    //         str.consume();
    //         return true;
    //     } else {
    //         return false;
    //     }
    // }
};

class AlterNode : public ASTNode {
  private:
    std::unique_ptr<ASTNode> left_;
    std::unique_ptr<ASTNode> right_;

  public:
    AlterNode() {
        id_ = NODE_ID++;
        type = ALTER_NODE;
    }
    void setLeft(std::unique_ptr<ASTNode> left) { left_ = std::move(left); }

    void setRight(std::unique_ptr<ASTNode> right) { right_ = std::move(right); }

    ASTNode &getLeft() const { return *left_.get(); }

    ASTNode &getRight() const { return *right_.get(); }

    void print(uint8_t indent = 0) const override {
        ASTNode::print(indent);
        std::cout << std::format("[Alter {}]: ", id_) << std::endl;
        ASTNode::print(indent + indent_incr_);
        std::cout << "Left:" << std::endl;
        left_->print(indent + indent_incr_);
        ASTNode::print(indent + indent_incr_);
        std::cout << "Right:" << std::endl;
        right_->print(indent + indent_incr_);
        std::cout << std::endl;
    }

    // bool match(StringSlider &str) override {
    //     if (left_->match(str) || right_->match(str)) {
    //         return true;
    //     } else {
    //         return false;
    //     }
    // }
};

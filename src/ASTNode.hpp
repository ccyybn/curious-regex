#pragma once
#include "StringSlider.hpp"
#include "Unicode.hpp"
#include <cstddef>
#include <cstdint>
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
        std::cout << id_;
        for (int i = 0; i < indent; i++) {
            std::cout << " ";
        }
    };

    virtual bool match(StringSlider &str) = 0;
};

inline ASTNode::~ASTNode() {}

class RepeatNode : public ASTNode {
  private:
    std::unique_ptr<ASTNode> child_;
    std::vector<size_t> revert_steps_;

  public:
    RepeatNode(std::unique_ptr<ASTNode> node) : child_(std::move(node)) {
        id_ = NODE_ID++;
        type = REPEAT_NODE;
    }
    void print(uint8_t indent = 0) const override {
        ASTNode::print(indent);
        std::cout << "[Repeat]: ";
        std::cout << std::endl;
        child_->print(indent + indent_incr_);
    }

    size_t pop_revert_step() {
        if (revert_steps_.empty()) {
            return -1;
        }

        auto step_length = revert_steps_.back();
        revert_steps_.pop_back();
        return step_length;
    }

    bool match(StringSlider &str) override {
        size_t cursor = str.cursor();
        while (child_->match(str)) {
            revert_steps_.push_back(str.cursor() - cursor);
            cursor = str.cursor();
        }
        return true;
    }
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

    std::unique_ptr<ASTNode> pop() {
        if (children_.empty()) {
            return nullptr;
        }
        auto last = std::move(children_.back());
        children_.pop_back();
        return last;
    }

    void print(uint8_t indent = 0) const override {
        ASTNode::print(indent);
        std::cout << "[Contact]: ";
        std::cout << std::endl;
        for (const auto &ptr : children_) {
            ptr->print(indent + indent_incr_);
        }
    }

    bool match(StringSlider &str) override { return matchNext(0, str); }

    bool matchNext(size_t next_child_index, StringSlider &str) const {
        auto &ptr = children_[next_child_index];
        auto remainStr = str.getRemainStr();
        if (!ptr->match(str)) {
            return false;
        }

        if ((next_child_index + 1) < children_.size() &&
            !matchNext(next_child_index + 1, str)) {
            if (ptr->type == REPEAT_NODE) {
                if (RepeatNode *repeat_ptr =
                        dynamic_cast<RepeatNode *>(ptr.get())) {
                    while (!matchNext(next_child_index + 1, str)) {
                        auto revert_step = repeat_ptr->pop_revert_step();
                        if (revert_step == -1) {
                            return false;
                        } else {
                            str.revert(revert_step);
                        }
                    }
                    return true;
                } else {
                    ptr->print();
                    throw std::runtime_error("Cannot access to repeat node");
                }

            } else {
                return false;
            }
        } else {
            return true;
        }
    }
};

class GroupNode : public ASTNode {
  private:
    std::unique_ptr<ASTNode> child_;

  public:
    GroupNode(std::unique_ptr<ASTNode> child) : child_(std::move(child)) {
        id_ = NODE_ID++;
        type = GROUP_NODE;
    }

    void print(uint8_t indent = 0) const override {
        ASTNode::print(indent);
        std::cout << "[Group]: ";
        std::cout << std::endl;
        child_->print(indent + indent_incr_);
    }

    bool match(StringSlider &str) override { return child_->match(str); }
};

class CharNode : public ASTNode {
  private:
    char32_t ch_;

  public:
    CharNode(char32_t ch) : ch_(ch) {
        id_ = NODE_ID++;
        type = CHAR_NODE;
    }

    void print(uint8_t indent = 0) const override {
        ASTNode::print(indent);
        std::cout << "[Char]: ";
        std::cout << u32_to_utf8(ch_) << std::endl;
    }

    bool match(StringSlider &str) override {
        if (str.hasMore() && str.getChar() == ch_) {
            str.consume();
            return true;
        } else {
            return false;
        }
    }
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

    void print(uint8_t indent = 0) const override {
        ASTNode::print(indent);
        std::cout << "[Alter]: " << std::endl;
        ASTNode::print(indent + indent_incr_);
        std::cout << "Left:" << std::endl;
        left_->print(indent + indent_incr_);
        ASTNode::print(indent + indent_incr_);
        std::cout << "Right:" << std::endl;
        right_->print(indent + indent_incr_);
        std::cout << std::endl;
    }

    bool match(StringSlider &str) override {
        if (left_->match(str) || right_->match(str)) {
            return true;
        } else {
            return false;
        }
    }
};

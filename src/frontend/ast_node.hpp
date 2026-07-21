#pragma once
#include <format>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <vector>

#include "../utils/unicode.hpp"

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
    virtual size_t getId() { return id_; }
    virtual std::string getName() const {
        std::string type_name;
        switch (type) {
            case CONTACT_NODE:
                type_name = "Contact";
                break;
            case GROUP_NODE:
                type_name = "Group";
                break;
            case ALTER_NODE:
                type_name = "Alter";
                break;
            case REPEAT_NODE:
                type_name = "Repeat";
                break;
            case CHAR_NODE:
                type_name = "Char";
                break;
        }

        return std::format("{} {}", type_name, id_);
    }

    friend class NFAState;
};

inline ASTNode::~ASTNode() {}

class RepeatNode : public ASTNode {
   private:
    std::unique_ptr<ASTNode> child_;

   public:
    RepeatNode(std::unique_ptr<ASTNode> node) : child_(std::move(node)) {
        id_ = NODE_ID++;
        type = REPEAT_NODE;
    }

    ASTNode& getChild() const { return *child_.get(); }

    void print(uint8_t indent = 0) const override {
        ASTNode::print(indent);
        std::cout << std::format("[{}]: ", getName());
        std::cout << std::endl;
        child_->print(indent + indent_incr_);
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

    void add(std::unique_ptr<ASTNode> node) { children_.push_back(std::move(node)); }

    size_t size() { return children_.size(); }

    ASTNode& getChild(size_t index) const {
        if (index < children_.size()) {
            return *children_[index].get();
        } else {
            throw std::runtime_error("ContactNode.getChild index > children_.size()");
        }
    }

    void print(uint8_t indent = 0) const override {
        ASTNode::print(indent);
        std::cout << std::format("[{}]: ", getName());
        std::cout << std::endl;
        for (const auto& ptr : children_) {
            ptr->print(indent + indent_incr_);
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

    ASTNode& getChild() const { return *child_.get(); }

    void print(uint8_t indent = 0) const override {
        ASTNode::print(indent);
        std::cout << std::format("[{}]: ", getName());
        std::cout << std::endl;
        child_->print(indent + indent_incr_);
    }
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
        std::cout << std::format("[{}]: ", getName());
        std::cout << u32_to_utf8(ch_) << std::endl;
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

    ASTNode& getLeft() const { return *left_.get(); }

    ASTNode& getRight() const { return *right_.get(); }

    void print(uint8_t indent = 0) const override {
        ASTNode::print(indent);
        std::cout << std::format("[{}]: ", getName()) << std::endl;
        ASTNode::print(indent + indent_incr_);
        std::cout << "Left:" << std::endl;
        left_->print(indent + indent_incr_);
        ASTNode::print(indent + indent_incr_);
        std::cout << "Right:" << std::endl;
        right_->print(indent + indent_incr_);
        std::cout << std::endl;
    }
};

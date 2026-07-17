#pragma once
#include "ASTNode.hpp"
#include <cstddef>
#include <memory>
#include <stdexcept>

class Parser {
  private:
    std::u32string_view source_;
    size_t cursor_ = 0;

  public:
    Parser(std::u32string_view source) : source_(source) {}

    std::unique_ptr<ASTNode> parse() {
        std::unique_ptr<ContactNode> root = std::make_unique<ContactNode>();

        while (cursor_ < source_.length()) {
            const auto ch = source_[cursor_++];
            if (ch == U'(') {
                std::unique_ptr<GroupNode> group =
                    std::make_unique<GroupNode>(parse());
                if (source_[cursor_ - 1] != U')') {
                    throw std::runtime_error("( is not closed");
                }
                root->add(std::move(group));
            } else if (ch == U')') {
                return root;
            } else if (ch == U'|') {
                return parseAlter(std::move(root));
            } else if (ch == U'*') {
                auto last_node = root->pop();
                if (last_node == nullptr) {
                    throw std::runtime_error("* must follow some char");
                }
                root->add(std::make_unique<RepeatNode>(std::move(last_node)));
            } else {
                root->add(std::make_unique<CharNode>(ch));
            }
        }
        return root;
    }

    std::unique_ptr<AlterNode> parseAlter(std::unique_ptr<ContactNode> left) {
        std::unique_ptr<AlterNode> root = std::make_unique<AlterNode>();
        root->setLeft(std::move(left));
        root->setRight(std::move(parse()));
        return root;
    }
};

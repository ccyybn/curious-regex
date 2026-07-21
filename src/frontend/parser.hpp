#pragma once
#include <format>
#include <memory>
#include <stdexcept>

#include "ast_node.hpp"
#include "utils/unicode.hpp"

/**

expr    ::= contact ( "|" contact )*
contact ::= factor ( factor )*
factor  ::= atom [ "*" ]
atom    ::= char | "\(" expr "\)"

**/

class Parser {
   private:
    std::u32string_view source_;
    size_t cursor_ = 0;

   public:
    Parser(std::u32string_view source) : source_(source) {}

    char32_t peek() const {
        if (cursor_ < source_.length()) return source_[cursor_];
        return U'\0';
    }

    std::unique_ptr<AstNode> parseExpr(bool inner_expr = false) {
        std::unique_ptr<AstNode> left = std::make_unique<ContactNode>();

        left = parseContact(inner_expr);

        while (peek() == U'|') {
            cursor_++;
            std::unique_ptr<AlterNode> alt = std::make_unique<AlterNode>();
            alt->setLeft(std::move(left));
            alt->setRight(parseContact(inner_expr));
            left = std::move(alt);
        }

        if (inner_expr) {
            if (peek() != U')') {
                throw std::runtime_error("( is not closed");
            }
        } else {
            if (peek() != U'\0') {
                if (peek() == U')') {
                    throw std::runtime_error("More ) than expected");
                } else {
                    throw std::runtime_error("Unexpected trailing characters: " + u32_to_utf8(peek()));
                }
            }
        }

        return left;
    }

    std::unique_ptr<ContactNode> parseContact(bool inner_expr) {
        std::unique_ptr<ContactNode> root = std::make_unique<ContactNode>();
        while (cursor_ < source_.length() && peek() != U'|' && peek() != U')') {
            root->add(parseFactor());
        }
        return root;
    }

    std::unique_ptr<AstNode> parseFactor() {
        std::unique_ptr<AstNode> factor;
        factor = parseAtom();
        if (peek() == U'*') {
            cursor_++;
            factor = std::make_unique<RepeatNode>(std::move(factor));
        }
        return factor;
    }

    std::unique_ptr<AstNode> parseAtom() {
        std::unique_ptr<AstNode> atom;

        if (peek() == U'(') {
            cursor_++;
            atom = std::make_unique<GroupNode>(parseExpr(true));
            cursor_++;
        } else if (cursor_ >= source_.length()) {
            throw std::runtime_error("No more char to parse");
        } else if (peek() == U')' || peek() == U'*' || peek() == U'|') {
            throw std::runtime_error(std::format("Invalid char for parsing atom: {}", u32_to_utf8(peek())));
        } else {
            atom = std::make_unique<CharNode>(peek());
            cursor_++;
        }

        return atom;
    }
};

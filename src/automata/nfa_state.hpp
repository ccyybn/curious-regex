#pragma once
#include <format>
#include <stdexcept>
#include <unordered_set>

#include "frontend/ast_node.hpp"
#include "utils/unicode.hpp"

enum STATE_TYPE { CHAR, EPSILON, END };
enum CONTROL_TYPE { IN, OUT, NORMAL };

inline size_t NFASTATE_ID = 0;

class NfaState;

class NfaState {
   private:
    size_t id_;
    STATE_TYPE type_;
    char32_t accept_ch_;
    std::string name_;
    CONTROL_TYPE control_type_ = NORMAL;
    const AstNode* ast_node_ = nullptr;

   public:
    NfaState* next1_ = nullptr;
    NfaState* next2_ = nullptr;
    NfaState(STATE_TYPE type, char32_t accept_ch) : type_(type), accept_ch_(accept_ch) {
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

    NfaState(char32_t accept_ch) : NfaState(CHAR, accept_ch) {};

    NfaState() : NfaState(EPSILON, 0) {};

    std::string controlTypeStr() const {
        switch (control_type_) {
            case IN:
                return "IN";
            case OUT:
                return "OUT";
            case NORMAL:
                return "NORMAL";
            default:
                throw std::runtime_error("Unknow control type");
        }
    }

    CONTROL_TYPE getControlType() const { return control_type_; }

    NODE_TYPE getAstType() const { return ast_node_->type; };

    size_t getASTId() const { return ast_node_->id_; }

    STATE_TYPE getType() { return type_; }

    char32_t getAcceptChar() { return accept_ch_; }

    bool accept(char32_t ch) { return type_ == CHAR && ch == accept_ch_; }

    NfaState* setIn(const AstNode& node) {
        ast_node_ = &node;
        control_type_ = IN;
        return this;
    }

    NfaState* setOut(const AstNode& node) {
        ast_node_ = &node;
        control_type_ = OUT;
        return this;
    }

    NfaState* setAST(const AstNode& node) {
        ast_node_ = &node;
        return this;
    }

    std::string getASTName() const {
        if (ast_node_) {
            return ast_node_->getName();
        } else {
            return "";
        }
    }

    std::string displayName(const std::string& seperator = "|") const {
        std::string display_name = name_;
        if (control_type_ != NORMAL) {
            if (seperator == "\\n") {
                display_name += seperator + "[" + controlTypeStr() + "]";
            } else {
                display_name += seperator + controlTypeStr();
            }
        }
        if (seperator == "\\n" && ast_node_) {
            display_name += seperator + "[" + getASTName() + "]";
        } else {
            display_name += seperator + getASTName();
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

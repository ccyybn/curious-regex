#include <memory>
#include <string_view>

#include "automata/builder.hpp"
#include "automata/nfa_state.hpp"
#include "engine/backtrack.hpp"
#include "engine/engine.hpp"
#include "frontend/ast_node.hpp"
#include "frontend/parser.hpp"

class Pattern {
   public:
    explicit Pattern(const std::u32string_view regex_str, EngineType default_engine = EngineType::Backtrack) {
        Parser parser(regex_str);
        ast_node_ = parser.parseExpr();
        ast_node_->print();
        builder_ = std::make_unique<NFABuilder>(*ast_node_.get());
        entry_ = builder_->build().entry;
        builder_->exportGraph(std::cout);
        setEngine(default_engine);
    }

    void setEngine(EngineType type) {
        type_ = type;
        switch (type) {
            case EngineType::Backtrack:
                engine_ = std::make_unique<BacktrackEngine>(entry_);
                break;
            case EngineType::ParallelNFA:
                // engine_ = std::make_unique<ParallelNfaEngine>(entry_);
                break;
            case EngineType::DFA:
                // engine_ = std::make_unique<DfaEngine>(entry_);
                break;
        }
    }

    bool match(const std::u32string_view& str) const { return engine_->match(str); }

    bool matchWith(const std::u32string_view& str, EngineType type) const {
        switch (type) {
            case EngineType::Backtrack:
                return BacktrackEngine(entry_).match(str);
            case EngineType::ParallelNFA:
                // return ParallelNfaEngine(entry_).match(str);
                return false;
            case EngineType::DFA:
                // return DfaEngine(entry_).match(str);
                return false;
        }
        return false;
    }

    EngineType currentEngineType() const { return type_; }

   private:
    std::unique_ptr<NFABuilder> builder_;
    std::unique_ptr<ASTNode> ast_node_;
    NFAState* entry_;
    EngineType type_;
    std::unique_ptr<IMatchEngine> engine_;
};

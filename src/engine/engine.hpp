#pragma once
#include <string_view>

enum class EngineType { Backtrack, ParallelNFA, DFA };

class IMatchEngine {
   public:
    virtual ~IMatchEngine() = default;
    virtual bool match(const std::u32string_view& str) = 0;
};

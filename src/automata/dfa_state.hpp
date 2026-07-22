#pragma once
#include <unordered_map>

#include "nfa_state.hpp"

inline size_t DFASTATE_ID = 0;

struct DfaHash {
    static uint64_t mix(uint64_t x) {
        x ^= x >> 30;
        x *= 0xbf58476d1ce4e5b9ULL;
        x ^= x >> 27;
        x *= 0x94d049bb133111ebULL;
        x ^= x >> 31;
        return x;
    }

    template <typename T>
    std::size_t operator()(const std::unordered_set<T*>& s) const {
        std::size_t seed = 0;
        for (T* ptr : s) {
            std::size_t h = std::hash<T*>()(ptr);
            seed += mix(h);
        }
        return seed;
    }
};

class DfaState {
   public:
    size_t id;
    bool is_end = false;
    std::unordered_set<NfaState*> nfa_states;
    std::unordered_map<char32_t, DfaState*> transition_table;

    DfaState(std::unordered_set<NfaState*> nfa_states) {
        id = DFASTATE_ID++;
        this->nfa_states = nfa_states;
        for (const auto& nfa_state : nfa_states) {
            if (nfa_state->getType() == END) {
                is_end = true;
                break;
            }
        }
    }
};

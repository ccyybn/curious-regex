#pragma once
#include <cstddef>
#include <memory>
#include <stdexcept>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "automata/dfa_state.hpp"
#include "automata/nfa_state.hpp"
#include "engine.hpp"
#include "helper.hpp"
#include "utils/logger.hpp"
#include "utils/unicode.hpp"

namespace NfaHash {};  // namespace NfaHash

class DfaEngine : public IMatchEngine {
   public:
    explicit DfaEngine(NfaState* entry) : entry_(entry) {
        constructDfa();
        exportMermaid(std::cout);
    }

    void exportMermaid(std::ostream& out) { generateMermaid(out, dfa_entry_, all_dfa_states_); }
    bool match(const std::u32string_view& str) override {
        DfaState* current_dfa_state = dfa_entry_;
        for (const char32_t ch : str) {
            if (current_dfa_state->transition_table.contains(ch)) {
                DfaState* next_dfa_state = current_dfa_state->transition_table[ch];
                Log::debug("[{}][{}] accepted [{}] -> [{}][{}]", current_dfa_state->id, getStateNames(current_dfa_state->nfa_states), u32_to_utf8(ch),
                           next_dfa_state->id, getStateNames(next_dfa_state->nfa_states));
                current_dfa_state = next_dfa_state;
            } else {
                Log::debug("[{}][{}] refused [{}]", current_dfa_state->id, getStateNames(current_dfa_state->nfa_states), u32_to_utf8(ch));
                return false;
            }
        }
        if (!current_dfa_state->is_end) {
            Log::debug("[{}][{}] stoped at no end state, failed", current_dfa_state->id, getStateNames(current_dfa_state->nfa_states));
        }
        return current_dfa_state->is_end;
    }

   private:
    void constructDfa() {
        Log::debug("Constructing the DFA graph");
        std::unique_ptr<DfaState> dfa_entry = std::make_unique<DfaState>(searchNextStates({entry_}));
        std::vector<DfaState*> current_dfa_states;
        current_dfa_states.push_back(dfa_entry.get());
        dfa_entry_ = dfa_entry.get();
        all_dfa_states_[dfa_entry->nfa_states] = std::move(dfa_entry);

        while (!current_dfa_states.empty()) {
            std::vector<DfaState*> next_dfa_states;

            for (auto& current_dfa_state : current_dfa_states) {
                std::unordered_map<char32_t, size_t> transition_table;
                std::unordered_map<char32_t, std::unordered_set<NfaState*>> accept_chars;
                // Find out all the acceptable chars in current dfa state
                for (auto& nfa_state : current_dfa_state->nfa_states) {
                    if (nfa_state->getType() == END) continue;
                    if (accept_chars.contains(nfa_state->getAcceptChar())) {
                        accept_chars[nfa_state->getAcceptChar()].insert(nfa_state);
                    } else {
                        accept_chars[nfa_state->getAcceptChar()] = {nfa_state};
                    }
                }

                auto it = accept_chars.begin();
                while (it != accept_chars.end()) {
                    // Each different acceptable char find a next dfa state
                    std::unordered_set<NfaState*> nfa_states = searchNextStates(it->second);
                    // if (nfa_states.empty()) {
                    //     Log::debug("[{}][{}] {} accept {} -> empty", current_dfa_state->id, getStateNames(current_dfa_state->nfa_states),
                    //                getStateNames(it->second), u32_to_utf8(it->first));
                    //     throw std::runtime_error("empty next dfa");
                    // }
                    if (!all_dfa_states_.contains(nfa_states)) {
                        std::unique_ptr<DfaState> next_dfa_state = std::make_unique<DfaState>(nfa_states);
                        next_dfa_states.push_back(next_dfa_state.get());
                        Log::debug("New DFA state found: [{}][{}]", next_dfa_state->id, getStateNames(next_dfa_state->nfa_states));
                        all_dfa_states_[next_dfa_state->nfa_states] = std::move(next_dfa_state);
                    }
                    current_dfa_state->transition_table[it->first] = all_dfa_states_[nfa_states].get();

                    Log::debug("[{}][{}] accept {} -> [{}][{}]", current_dfa_state->id, getStateNames(current_dfa_state->nfa_states),
                               u32_to_utf8(it->first), current_dfa_state->transition_table[it->first]->id, getStateNames(nfa_states));
                    it++;
                }
            }
            current_dfa_states = std::move(next_dfa_states);
        }
    }

    DfaState* dfa_entry_;
    std::unordered_map<std::unordered_set<NfaState*>, std::unique_ptr<DfaState>, DfaHash> all_dfa_states_;
    NfaState* entry_;
};

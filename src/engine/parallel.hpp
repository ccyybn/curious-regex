#pragma once
#include <unordered_set>

#include "automata/nfa_state.hpp"
#include "engine.hpp"
#include "helper.hpp"
#include "utils/logger.hpp"
#include "utils/unicode.hpp"

class ParallelNfaEngine : public IMatchEngine {
   public:
    explicit ParallelNfaEngine(NfaState* entry) : entry_(entry) {}

    bool match(const std::u32string_view& str) override {
        std::unordered_set<NfaState*> current_states = searchNextStates({entry_});
        for (const char32_t ch : str) {
            Log::debug("[{}] current states [{}]", u32_to_utf8(ch), getStateNames(current_states));
            std::unordered_set<NfaState*> accepted_states;
            for (const auto& state : current_states) {
                if (state->accept(ch)) {
                    accepted_states.insert(state);
                }
            }
            Log::debug("[{}] accepted states [{}]", u32_to_utf8(ch), getStateNames(accepted_states));
            current_states = searchNextStates(accepted_states);
        }

        Log::debug("End states [{}]", getStateNames(current_states));

        for (const auto& state : current_states) {
            if (state->getType() == END) {
                return true;
            }
        }

        return false;
    }

   private:
    NfaState* entry_;
};

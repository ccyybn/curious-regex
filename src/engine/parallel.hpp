#pragma once
#include <unordered_set>
#include <vector>

#include "automata/nfa_state.hpp"
#include "engine.hpp"
#include "utils/logger.hpp"
#include "utils/unicode.hpp"

namespace {

struct SearchRecorder {
    std::unordered_set<NfaState*> visited_states;
    std::unordered_set<NfaState*> next_states;
};

void findNext(NfaState* current, SearchRecorder& recorder) {
    if (recorder.visited_states.contains(current)) return;
    NfaState* next1 = current->next1_;
    NfaState* next2 = current->next2_;
    recorder.visited_states.insert(current);
    if (next1) {
        if (next1->getType() == EPSILON) {
            findNext(next1, recorder);
        } else {
            recorder.next_states.insert(next1);
        }
    }
    if (next2) {
        if (next2->getType() == EPSILON) {
            findNext(next2, recorder);
        } else {
            recorder.next_states.insert(next2);
        }
    }
}

std::unordered_set<NfaState*> searchNextStates(const std::unordered_set<NfaState*>& current_states) {
    SearchRecorder recorder = {};
    for (const auto& state : current_states) {
        findNext(state, recorder);
    }

    return std::move(recorder.next_states);
}

std::string getStateNames(std::unordered_set<NfaState*> states) {
    std::string name;
    name.reserve(states.size() * 10);

    auto it = states.begin();
    if (it != states.end()) {
        name += (*it)->displayName(" ");
        ++it;
    }

    for (; it != states.end(); ++it) {
        name += "," + (*it)->displayName(" ");
    }
    return name;
}
};  // namespace

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

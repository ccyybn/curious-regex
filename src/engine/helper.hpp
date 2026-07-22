#pragma once
#include "automata/dfa_state.hpp"
#include "automata/nfa_state.hpp"
#include "utils/logger.hpp"

struct NfaSearchRecorder {
    std::unordered_set<NfaState*> visited_states;
    std::unordered_set<NfaState*> next_states;
};

inline void findNext(NfaState* current, NfaSearchRecorder& recorder) {
    if (recorder.visited_states.contains(current)) {
        return;
    }
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

inline std::unordered_set<NfaState*> searchNextStates(const std::unordered_set<NfaState*>& current_states) {
    NfaSearchRecorder recorder = {};
    for (const auto& state : current_states) {
        findNext(state, recorder);
    }
    return std::move(recorder.next_states);
}

inline std::string getStateNames(std::unordered_set<NfaState*> states) {
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

inline std::string escapeMermaidChar(char32_t ch) {
    std::string utf8_str = u32_to_utf8(ch);
    if (utf8_str == "\"") return "\\\"";
    if (utf8_str == "\n") return "\\n";
    if (utf8_str == "\t") return "\\t";
    if (utf8_str == "\r") return "\\r";
    if (utf8_str == " ") return "SPACE";
    return utf8_str;
}

inline void generateMermaid(std::ostream& out, DfaState* dfa_entry,
                            std::unordered_map<std::unordered_set<NfaState*>, std::unique_ptr<DfaState>, DfaHash>& all_dfa_states) {
    out << "```mermaid\n";
    out << "flowchart LR\n";

    if (dfa_entry) {
        out << "    start((*)) --> S" << dfa_entry->id << "\n";
    }

    for (const auto& [_, dfa_state_ptr] : all_dfa_states) {
        const auto* state = dfa_state_ptr.get();
        if (state->is_end) {
            out << "    S" << state->id << " --> stop((( )))\n";
        }
    }

    out << "\n";

    for (const auto& [_, dfa_state_ptr] : all_dfa_states) {
        const auto* from_state = dfa_state_ptr.get();

        std::unordered_map<size_t, std::vector<char32_t>> grouped_transitions;
        for (const auto& [ch, to_state] : from_state->transition_table) {
            grouped_transitions[to_state->id].push_back(ch);
        }

        using TransitionPair = std::pair<size_t, std::vector<char32_t>>;
        std::vector<TransitionPair> transitions(grouped_transitions.begin(), grouped_transitions.end());

        std::sort(transitions.begin(), transitions.end(), [from_id = from_state->id](const TransitionPair& a, const TransitionPair& b) {
            bool a_is_loop = (a.first == from_id);
            bool b_is_loop = (b.first == from_id);
            if (a_is_loop != b_is_loop) {
                return !a_is_loop;
            }
            return a.first < b.first;
        });

        for (const auto& [to_id, chars] : transitions) {
            out << "    S" << from_state->id << " -- \"";

            for (size_t i = 0; i < chars.size(); ++i) {
                if (i > 0) out << ", ";
                out << escapeMermaidChar(chars[i]);
            }
            out << "\" --> S" << to_id << "\n";
        }
    }
    out << "```\n\n";
}

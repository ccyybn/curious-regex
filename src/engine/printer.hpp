#include <queue>

#include "automata/nfa_state.hpp"

inline std::string remainStr(const std::u32string_view& str, size_t cursor) {
    std::string s;
    for (size_t i = cursor; i < str.length(); i++) {
        s += u32_to_utf8(str[i]);
    }
    return s;
}

namespace printer {
// LoopSegment is for printing only, has no impact on the algorithm.
struct LoopSegment {
    NfaState* state;
    size_t cursor;
};

inline void printLoopSegment(const std::u32string_view& str, std::vector<std::queue<LoopSegment>> loop_segment_recorder) {
    // This code is used to print the loop segments when using ((a*))* to match aaaab
    // For example:
    // aaaa,b
    // aaa,a,b
    // aaa,ab
    // aa,aa,b
    // aa,a,a,b
    // aa,a,ab
    // aa,aab
    // a,aaa,b
    // a,aa,a,b
    // a,aa,ab
    // a,a,aa,b
    // a,a,a,a,b
    // a,a,a,ab
    // a,a,aab
    // a,aaab
    // ,aaaa,b
    // ,aaa,a,b
    // ,aaa,ab
    // ,aa,aa,b
    // ,aa,a,a,b
    // ,aa,a,ab
    // ,aa,aab
    // ,a,aaa,b
    // ,a,aa,a,b
    // ,a,aa,ab
    // ,a,a,aa,b
    // ,a,a,a,a,b
    // ,a,a,a,ab
    // ,a,a,aab
    // ,a,aaab
    // ,aaaab
    // aaaab
    for (std::queue<LoopSegment>& loop_segment : loop_segment_recorder) {
        for (int i = 0; i < str.length(); i++) {
            while (!loop_segment.empty()) {
                LoopSegment& seg = loop_segment.front();
                if (seg.cursor == i) {
                    if (seg.state->getASTId() == 6) {
                        std::cout << ",";
                    }
                    loop_segment.pop();
                } else {
                    break;
                }
            }
            std::cout << u32_to_utf8(str[i]);
        }
        std::cout << std::endl;
        if (!loop_segment.empty()) {
            throw std::runtime_error("Cannot print loop segments");
        }
    }
}

}  // namespace printer

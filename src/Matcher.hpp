#include <cstddef>
#include <ostream>
#include <stack>
#include <string_view>

#include "NFAState.hpp"
#include "Unicode.hpp"
#include "logger.hpp"
namespace {
inline size_t POINT_ID = 0;

struct BacktrackPoint {
    NFAState* state;
    size_t cursor;
    size_t id = POINT_ID++;
};

struct ProgressContext {
    NFAState* current_state;
    const std::u32string_view& str;
    std::stack<BacktrackPoint> regression_stack;
    size_t cursor = 0;
};

inline std::string getRemainStr(const std::u32string_view& str, size_t cursor) {
    std::string s;
    for (size_t i = cursor; i < str.length(); i++) {
        s += u32_to_utf8(str[i]);
    }
    return s;
}

inline bool backtrack(ProgressContext& context) {
    if (context.regression_stack.empty()) {
        Log::debug("No regression point, match failed.");
        return false;
    } else {
        BacktrackPoint point = context.regression_stack.top();
        context.current_state = point.state;
        context.cursor = point.cursor;
        Log::debug("[{}] Pop: [remain: {}] {}", point.id, getRemainStr(context.str, context.cursor), point.state->displayName());
        context.regression_stack.pop();
        return true;
    }
}

inline void pushBacktrackPoint(ProgressContext& context) {
    if (context.current_state->next2_) {
        context.regression_stack.push({context.current_state->next2_, context.cursor});
        Log::debug("[{}] Push: [{}]", context.regression_stack.top().id, context.current_state->next2_->displayName());
    }
}

}  // namespace

inline bool backTrackMatch(NFAState* entry, const std::u32string_view& str) {
    ProgressContext context = {entry, str};
    std::stack<BacktrackPoint> regression_stack = context.regression_stack;
    while (true) {
        auto& current_state = context.current_state;
        auto& cursor = context.cursor;
        if (current_state->getType() == EPSILON) {
            Log::debug("[EPSILON][remain: {}] [{}]", getRemainStr(str, cursor), current_state->displayName());
            pushBacktrackPoint(context);
            current_state = current_state->next1_;
        } else if (current_state->getType() == CHAR) {
            if (cursor < str.length() && str[cursor] == current_state->getAcceptChar()) {
                char32_t accepted = str[cursor];
                cursor++;
                Log::debug("[CHAR][accept: {}][remain: {}] {}", u32_to_utf8(accepted), getRemainStr(str, cursor), current_state->displayName());
                pushBacktrackPoint(context);
                current_state = current_state->next1_;
            } else {
                std::string refused = cursor < str.length() ? u32_to_utf8(str[cursor]) : "END";
                Log::debug("[CHAR][refuse: {}][remain: {}] {}", refused, getRemainStr(str, cursor), current_state->displayName());
                if (!backtrack(context)) {
                    return false;
                }
            }
        } else if (current_state->getType() == END) {
            if (cursor >= str.length()) {
                std::cout << "Match successful." << std::endl;
                return true;
            } else {
                Log::debug("[END] [remain: {}]", getRemainStr(str, cursor));
                if (!backtrack(context)) {
                    return false;
                }
            }
        }
    }
}

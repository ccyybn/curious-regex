#include <cstddef>
#include <cstdlib>
#include <format>
#include <ostream>
#include <stack>
#include <stdexcept>
#include <string_view>
#include <unordered_map>

#include "ASTNode.hpp"
#include "NFAState.hpp"
#include "Unicode.hpp"
#include "logger.hpp"
namespace {
inline size_t POINT_ID = 0;

struct BacktrackPoint {
    NFAState* state;
    size_t cursor;
    std::unordered_map<NFAState*, size_t> loop_recorder;
    size_t id = POINT_ID++;
};

struct ProgressContext {
    NFAState* current_state;
    const std::u32string_view& str;
    std::stack<BacktrackPoint> regression_stack;
    std::unordered_map<NFAState*, size_t> loop_recorder;
    size_t cursor = 0;
    size_t track_id = 0;
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
        context.loop_recorder = point.loop_recorder;
        context.track_id = point.id;
        Log::debug("[{}] Pop: [{}] [remain: {}]", point.id, point.state->displayName(), getRemainStr(context.str, context.cursor));
        context.regression_stack.pop();
        return true;
    }
}

inline void pushBacktrackPoint(ProgressContext& context) {
    if (context.current_state->next2_) {
        context.regression_stack.push({context.current_state->next2_, context.cursor, context.loop_recorder});
        Log::debug("[{}] Push: [{}] [remain: {}]", context.regression_stack.top().id, context.current_state->next2_->displayName(),
                   getRemainStr(context.str, context.cursor));
    }
}

}  // namespace

inline bool backTrackMatch(NFAState* entry, const std::u32string_view& str) {
    ProgressContext context = {entry, str};
    std::stack<BacktrackPoint> regression_stack = context.regression_stack;
    int counter = 0;
    while (true) {
        // counter++;
        // if (counter > 500) {
        //     exit(1);
        // }
        auto& current_state = context.current_state;
        auto& cursor = context.cursor;
        auto& loop_recorder = context.loop_recorder;
        auto& track_id = context.track_id;
        if (current_state->getType() == EPSILON) {
            Log::debug("[{}] [EPSILON][{}] [remain: {}]", track_id, current_state->displayName(), getRemainStr(str, cursor));

            if (current_state->getControlType() == OUT && current_state->next2_) {
                if (loop_recorder.contains(current_state) && loop_recorder[current_state] == cursor) {
                    Log::debug("[{}] [Anti-infinite-loop][{}] trying to rollback, remain: {}", track_id, current_state->displayName(),
                               getRemainStr(str, context.cursor));
                    auto before_trackback = current_state;
                    if (!backtrack(context)) {
                        throw std::runtime_error(std::format("[Anti-infinite-loop] {} {} backtrack failed", current_state->displayName(), cursor));
                    } else {
                        Log::debug("[{}] [Anti-infinite-loop][{}] rollback to [{}], remain: {}", track_id, before_trackback->displayName(),
                                   context.current_state->displayName(), getRemainStr(str, context.cursor));
                        continue;
                    }
                } else {
                    Log::debug("[{}] [PUT][Anti-infinite-loop] [{}] limit:{} next2: {}", track_id, current_state->displayName(),
                               getRemainStr(str, cursor), current_state->next2_->displayName());
                    loop_recorder[current_state] = cursor;
                }
            }
            pushBacktrackPoint(context);
            current_state = current_state->next1_;
        } else if (current_state->getType() == CHAR) {
            if (cursor < str.length() && str[cursor] == current_state->getAcceptChar()) {
                char32_t accepted = str[cursor];
                cursor++;
                Log::debug("[{}] [CHAR][accept: {}] [remain: {}] {}", track_id, u32_to_utf8(accepted), getRemainStr(str, cursor),
                           current_state->displayName());
                pushBacktrackPoint(context);
                current_state = current_state->next1_;
            } else {
                std::string refused = cursor < str.length() ? u32_to_utf8(str[cursor]) : "END";
                Log::debug("[{}] [CHAR][refuse: {}] [remain: {}] {}", track_id, refused, getRemainStr(str, cursor), current_state->displayName());
                if (!backtrack(context)) {
                    return false;
                }
            }
        } else if (current_state->getType() == END) {
            if (cursor >= str.length()) {
                std::cout << "Match successful." << std::endl;
                return true;
            } else {
                Log::debug("[{}] [END] [remain: {}]", track_id, getRemainStr(str, cursor));
                if (!backtrack(context)) {
                    return false;
                }
            }
        }
    }
}

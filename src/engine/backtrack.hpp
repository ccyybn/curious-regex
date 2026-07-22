#pragma once
#include <queue>
#include <stack>
#include <unordered_map>

#include "automata/nfa_state.hpp"
#include "engine.hpp"
#include "printer.hpp"
#include "utils/logger.hpp"

namespace {
inline size_t POINT_ID = 0;

struct BacktrackPoint {
    NfaState* state;
    size_t cursor;
    std::unordered_map<NfaState*, size_t> loop_recorder;
    std::queue<printer::LoopSegment> loop_segments;
    size_t id = POINT_ID++;
};

struct ProgressContext {
    NfaState* current_state;
    const std::u32string_view& str;
    std::stack<BacktrackPoint> regression_stack;
    std::unordered_map<NfaState*, size_t> loop_recorder;
    std::queue<printer::LoopSegment> loop_segments;
    size_t cursor = 0;
    size_t track_id = 0;
};

inline bool backtrack(ProgressContext& context) {
    if (context.regression_stack.empty()) {
        Log::debug("No backtrack point, match failed.");
        return false;
    } else {
        BacktrackPoint point = context.regression_stack.top();
        context.current_state = point.state;
        context.cursor = point.cursor;
        context.loop_recorder = point.loop_recorder;
        context.track_id = point.id;
        context.loop_segments = point.loop_segments;
        Log::debug("[{}] Pop: [{}] [remain: {}]", point.id, point.state->displayName(), remainStr(context.str, context.cursor));
        context.regression_stack.pop();
        return true;
    }
}

inline void pushBacktrackPoint(ProgressContext& context) {
    if (context.current_state->next2_) {
        context.regression_stack.push({context.current_state->next2_, context.cursor, context.loop_recorder, context.loop_segments});
        Log::debug("[{}] Push: [{}] [remain: {}]", context.regression_stack.top().id, context.current_state->next2_->displayName(),
                   remainStr(context.str, context.cursor));
    }
}

}  // namespace
class BacktrackEngine : public IMatchEngine {
   public:
    explicit BacktrackEngine(NfaState* entry) : entry_(entry) {}

    bool match(const std::u32string_view& str) override {
        ProgressContext context = {entry_, str};
        std::stack<BacktrackPoint> regression_stack = context.regression_stack;
        int counter = 0;
        // used for loop slice printing, for example: aa,aa,b ; aa,a,a,b
        std::vector<std::queue<printer::LoopSegment>> loop_segment_recorder;
        bool match_result = false;
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
                Log::debug("[{}] [EPSILON][{}] [remain: {}]", track_id, current_state->displayName(), remainStr(str, cursor));

                if (current_state->getControlType() == OUT && current_state->next2_) {
                    if (loop_recorder.contains(current_state) && loop_recorder[current_state] == cursor) {
                        Log::debug("[{}] [Anti-infinite-loop][{}] trying to rollback, remain: {}", track_id, current_state->displayName(),
                                   remainStr(str, context.cursor));
                        auto before_trackback = current_state;
                        if (!backtrack(context)) {
                            throw std::runtime_error(
                                std::format("[Anti-infinite-loop] {} {} backtrack failed", current_state->displayName(), cursor));
                        } else {
                            Log::debug("[{}] [Anti-infinite-loop][{}] rollback to [{}], remain: {}", track_id, before_trackback->displayName(),
                                       context.current_state->displayName(), remainStr(str, context.cursor));
                            continue;
                        }
                    } else {
                        Log::debug("[{}] [PUT][Anti-infinite-loop] [{}] limit:{} next2: {}", track_id, current_state->displayName(),
                                   remainStr(str, cursor), current_state->next2_->displayName());
                        loop_recorder[current_state] = cursor;
                    }
                    context.loop_segments.push({current_state, cursor});
                }
                pushBacktrackPoint(context);
                current_state = current_state->next1_;
            } else if (current_state->getType() == CHAR) {
                if (cursor < str.length() && current_state->accept(str[cursor])) {
                    char32_t accepted = str[cursor];
                    cursor++;
                    Log::debug("[{}] [CHAR][accept: {}] [remain: {}] {}", track_id, u32_to_utf8(accepted), remainStr(str, cursor),
                               current_state->displayName());
                    current_state = current_state->next1_;
                } else {
                    std::string refused = cursor < str.length() ? u32_to_utf8(str[cursor]) : "END";
                    Log::debug("[{}] [CHAR][refuse: {}] [remain: {}] {}", track_id, refused, remainStr(str, cursor), current_state->displayName());
                    if (!backtrack(context)) break;
                }
            } else if (current_state->getType() == END) {
                loop_segment_recorder.push_back(std::move(context.loop_segments));
                context.loop_segments = std::queue<printer::LoopSegment>();
                if (cursor >= str.length()) {
                    std::cout << "Match successful." << std::endl;
                    match_result = true;
                    break;
                } else {
                    Log::debug("[{}] [END] [remain: {}]", track_id, remainStr(str, cursor));
                    if (!backtrack(context)) break;
                }
            }
        }
        // printLoopSegment(str, loop_segment_recorder);
        return match_result;
    }

   private:
    NfaState* entry_;
};

#include <format>
#include <iostream>
#include <source_location>
#include <string_view>
class Log {
   public:
    struct LogFormat {
        std::string_view fmt;
        std::source_location location;

        LogFormat(const char* str, const std::source_location& loc = std::source_location::current()) : fmt(str), location(loc) {}
        LogFormat(std::string_view str, const std::source_location& loc = std::source_location::current()) : fmt(str), location(loc) {}
    };

    template <typename... Args>
    static void debug(LogFormat format_container, Args&&... args) {
        std::string_view fmt = format_container.fmt;
        const auto& location = format_container.location;

        std::string message = std::vformat(fmt, std::make_format_args(static_cast<const Args&>(args)...));

        // std::cout << std::format("[DEBUG] [{}:{}] {}\n", location.file_name(), location.line(), message);
        std::cout << std::format("{}\n", message);
    }
};

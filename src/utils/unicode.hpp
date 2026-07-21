#pragma once
#include <string>

inline std::u32string utf8_to_u32(std::string_view utf8_str) {
    std::u32string result;
    result.reserve(utf8_str.size());

    const auto* p = reinterpret_cast<const unsigned char*>(utf8_str.data());
    const auto* end = p + utf8_str.size();

    while (p < end && *p) {
        char32_t code_point = 0;

        if ((*p & 0x80) == 0) {
            code_point = *p++;
        } else if ((*p & 0xE0) == 0xC0 && (p + 1 < end)) {
            code_point = (*p++ & 0x1F) << 6;
            code_point |= (*p++ & 0x3F);
        } else if ((*p & 0xF0) == 0xE0 && (p + 2 < end)) {
            code_point = (*p++ & 0x0F) << 12;
            code_point |= (*p++ & 0x3F) << 6;
            code_point |= (*p++ & 0x3F);
        } else if ((*p & 0xF8) == 0xF0 && (p + 3 < end)) {
            code_point = (*p++ & 0x07) << 18;
            code_point |= (*p++ & 0x3F) << 12;
            code_point |= (*p++ & 0x3F) << 6;
            code_point |= (*p++ & 0x3F);
        } else {
            p++;
            continue;
        }
        result.push_back(code_point);
    }
    return result;
}

inline std::u32string utf8_to_u32(const char* cstr) {
    if (!cstr) return {};
    return utf8_to_u32(std::string_view(cstr));
}

inline void u32_to_utf8_append(char32_t cp, std::string& out) {
    if (cp <= 0x7F) {
        out.push_back(static_cast<char>(cp));
    } else if (cp <= 0x7FF) {
        out.push_back(static_cast<char>(0xC0 | ((cp >> 6) & 0x1F)));
        out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    } else if (cp <= 0xFFFF) {
        out.push_back(static_cast<char>(0xE0 | ((cp >> 12) & 0x0F)));
        out.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
        out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    } else if (cp <= 0x10FFFF) {
        out.push_back(static_cast<char>(0xF0 | ((cp >> 18) & 0x07)));
        out.push_back(static_cast<char>(0x80 | ((cp >> 12) & 0x3F)));
        out.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
        out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    }
}

inline std::string u32_to_utf8(char32_t cp) {
    std::string result;
    u32_to_utf8_append(cp, result);
    return result;
}

inline std::string u32_to_utf8(std::u32string_view u32_str) {
    std::string result;
    result.reserve(u32_str.size() * 2);
    for (char32_t cp : u32_str) {
        u32_to_utf8_append(cp, result);
    }
    return result;
}

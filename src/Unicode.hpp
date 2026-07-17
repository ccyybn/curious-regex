#pragma once
#include <string>

inline std::u32string utf8_to_u32(const char *cstr) {
    if (!cstr)
        return {};

    std::u32string result;
    const unsigned char *p = reinterpret_cast<const unsigned char *>(cstr);

    while (*p) {
        char32_t code_point = 0;
        if ((*p & 0x80) == 0) {
            code_point = *p++;
        } else if ((*p & 0xE0) == 0xC0) {
            code_point = (*p++ & 0x1F) << 6;
            code_point |= (*p++ & 0x3F);
        } else if ((*p & 0xF0) == 0xE0) {
            code_point = (*p++ & 0x0F) << 12;
            code_point |= (*p++ & 0x3F) << 6;
            code_point |= (*p++ & 0x3F);
        } else if ((*p & 0xF8) == 0xF0) {
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

inline std::string u32_to_utf8(char32_t cp) {
    std::string result;
    if (cp <= 0x7F) {
        result.push_back(static_cast<char>(cp));
    } else if (cp <= 0x7FF) {
        result.push_back(static_cast<char>(0xC0 | ((cp >> 6) & 0x1F)));
        result.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    } else if (cp <= 0xFFFF) {
        result.push_back(static_cast<char>(0xE0 | ((cp >> 12) & 0x0F)));
        result.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
        result.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    } else if (cp <= 0x10FFFF) {
        result.push_back(static_cast<char>(0xF0 | ((cp >> 18) & 0x07)));
        result.push_back(static_cast<char>(0x80 | ((cp >> 12) & 0x3F)));
        result.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
        result.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    }
    return result;
}

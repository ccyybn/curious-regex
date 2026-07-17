#include "Unicode.hpp"
#include <cstddef>
#include <cstdio>
#include <format>
#include <iostream>
#include <stdexcept>
#include <string_view>

class StringSlider {
  private:
    std::u32string_view str_;
    size_t cursor_ = 0;

  public:
    StringSlider(const std::u32string_view str) : str_(str) {}
    bool hasMore() const { return cursor_ < str_.length(); }

    void revert(size_t step) {
        if (step > cursor_) {
            throw std::runtime_error(std::format(
                "Cannot revert when step > cursor {} > {}", step, cursor_));
        }
        cursor_ -= step;
    }

    void consume() {
        if (!hasMore()) {
            throw std::runtime_error("No char can be consumed");
        }
        cursor_++;
    }

    char32_t getChar() { return str_[cursor_]; }

    size_t cursor() { return cursor_; }

    std::u32string_view getRemainStr() { return str_.substr(cursor_); }

    void print(const std::u32string_view str) {
        for (char32_t c : str) {
            std::cout << u32_to_utf8(c);
        }
        std::cout << std ::endl;
    }
};

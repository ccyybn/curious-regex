
#include <catch2/catch_test_macros.hpp>

#include "pattern.hpp"
#include "utils/unicode.hpp"

TEST_CASE("Engine - Backtrack Matching", "[engine]") {
    SECTION("Simple Concatenation") {
        Pattern pattern(utf8_to_u32("hello"));
        REQUIRE(pattern.match(utf8_to_u32("hello")) == true);
        REQUIRE(pattern.match(utf8_to_u32("world")) == false);
    }

    SECTION("Quantifiers (*)") {
        Pattern pattern(utf8_to_u32("a*"));
        REQUIRE(pattern.match(utf8_to_u32("")) == true);
        REQUIRE(pattern.match(utf8_to_u32("a")) == true);
        REQUIRE(pattern.match(utf8_to_u32("aaaa")) == true);
    }
}

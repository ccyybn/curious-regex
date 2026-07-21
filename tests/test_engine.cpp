#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include "engine/engine.hpp"
#include "pattern.hpp"
#include "utils/unicode.hpp"

TEST_CASE("Engine - Matching", "[engine]") {
    auto engine_type = GENERATE(EngineType::Backtrack, EngineType::ParallelNFA);

    auto make_pattern = [engine_type](const std::string& regex) {
        Pattern pattern(utf8_to_u32(regex));
        pattern.setEngine(engine_type);
        return pattern;
    };

    SECTION("Literal & Simple Concatenation") {
        auto pattern = make_pattern("hello");
        REQUIRE(pattern.match(utf8_to_u32("hello")) == true);
        REQUIRE(pattern.match(utf8_to_u32("world")) == false);
        REQUIRE(pattern.match(utf8_to_u32("hell")) == false);
        REQUIRE(pattern.match(utf8_to_u32("helloo")) == false);
    }

    SECTION("Alternation (|)") {
        auto pattern = make_pattern("cat|dog");
        REQUIRE(pattern.match(utf8_to_u32("cat")) == true);
        REQUIRE(pattern.match(utf8_to_u32("dog")) == true);
        REQUIRE(pattern.match(utf8_to_u32("pig")) == false);
        REQUIRE(pattern.match(utf8_to_u32("catdog")) == false);
    }

    SECTION("Multiple Alternations") {
        auto pattern = make_pattern("a|b|c|d");
        REQUIRE(pattern.match(utf8_to_u32("a")) == true);
        REQUIRE(pattern.match(utf8_to_u32("c")) == true);
        REQUIRE(pattern.match(utf8_to_u32("e")) == false);
    }

    SECTION("Star Quantifier (*)") {
        auto pattern = make_pattern("a*");
        REQUIRE(pattern.match(utf8_to_u32("")) == true);
        REQUIRE(pattern.match(utf8_to_u32("a")) == true);
        REQUIRE(pattern.match(utf8_to_u32("aaaa")) == true);
        REQUIRE(pattern.match(utf8_to_u32("b")) == false);
    }

    SECTION("Quantifier Precedence & Concat") {
        auto pattern = make_pattern("ab*c");
        REQUIRE(pattern.match(utf8_to_u32("ac")) == true);
        REQUIRE(pattern.match(utf8_to_u32("abc")) == true);
        REQUIRE(pattern.match(utf8_to_u32("abbbc")) == true);
        REQUIRE(pattern.match(utf8_to_u32("abb")) == false);
    }

    SECTION("Parentheses Grouping") {
        auto pattern = make_pattern("a(bc)d");
        REQUIRE(pattern.match(utf8_to_u32("abcd")) == true);
        REQUIRE(pattern.match(utf8_to_u32("ad")) == false);
        REQUIRE(pattern.match(utf8_to_u32("abd")) == false);
    }

    SECTION("Group with Quantifier") {
        auto pattern = make_pattern("(ab)*");
        REQUIRE(pattern.match(utf8_to_u32("")) == true);
        REQUIRE(pattern.match(utf8_to_u32("ab")) == true);
        REQUIRE(pattern.match(utf8_to_u32("ababab")) == true);
        REQUIRE(pattern.match(utf8_to_u32("aba")) == false);
    }

    SECTION("Group with Alternation") {
        auto pattern = make_pattern("a(b|c)d");
        REQUIRE(pattern.match(utf8_to_u32("abd")) == true);
        REQUIRE(pattern.match(utf8_to_u32("acd")) == true);
        REQUIRE(pattern.match(utf8_to_u32("ad")) == false);
        REQUIRE(pattern.match(utf8_to_u32("abcd")) == false);
    }

    SECTION("Complex Nested Grammar") {
        auto pattern = make_pattern("(a|b)*c");
        REQUIRE(pattern.match(utf8_to_u32("c")) == true);
        REQUIRE(pattern.match(utf8_to_u32("ac")) == true);
        REQUIRE(pattern.match(utf8_to_u32("bc")) == true);
        REQUIRE(pattern.match(utf8_to_u32("ababaabac")) == true);
        REQUIRE(pattern.match(utf8_to_u32("ababaaba")) == false);
    }

    SECTION("Nested Parentheses") {
        auto pattern = make_pattern("((a*|b)*c)*");
        REQUIRE(pattern.match(utf8_to_u32("")) == true);
        REQUIRE(pattern.match(utf8_to_u32("c")) == true);
        REQUIRE(pattern.match(utf8_to_u32("aaac")) == true);
        REQUIRE(pattern.match(utf8_to_u32("aababc")) == true);
        REQUIRE(pattern.match(utf8_to_u32("aaacbc")) == true);
        REQUIRE(pattern.match(utf8_to_u32("aaacbcb")) == false);
    }

    SECTION("Consecutive Quantifiers on Groups") {
        auto pattern = make_pattern("(a*)*");
        REQUIRE(pattern.match(utf8_to_u32("")) == true);
        REQUIRE(pattern.match(utf8_to_u32("a")) == true);
        REQUIRE(pattern.match(utf8_to_u32("aaaaa")) == true);
        REQUIRE(pattern.match(utf8_to_u32("b")) == false);
    }
}

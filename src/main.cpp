#include <iostream>
#include <string>

#include "engine/engine.hpp"
#include "pattern.hpp"

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <Regular Expression> <Target String>" << std::endl;
        return 1;
    }

    std::u32string regex = utf8_to_u32(argv[1]);
    std::u32string str = utf8_to_u32(argv[2]);

    try {
        Pattern pattern(regex);
        pattern.setEngine(EngineType::DFA);
        bool result = pattern.match(str);
        std::cout << "Result: " << result << std::endl;

        // auto result = (expr->match(slider) && !slider.hasMore());
        // std::cout << "Match result: " << result << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}

#include <iostream>
#include <string>

#include "engine/backtrack_engine.hpp"
#include "frontend/parser.hpp"

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <Regular Expression> <Target String>" << std::endl;
        return 1;
    }

    std::u32string regex = utf8_to_u32(argv[1]);
    std::u32string str = utf8_to_u32(argv[2]);

    // for (char32_t c : regex) {
    //     std::cout << u32_to_utf8(c) << std::endl;
    // }
    try {
        Parser parser(regex);
        auto expr = parser.parseExpr();

        expr->print();
        std::cout << std::endl;

        NFABuilder builder(*expr.get());
        NFAFragment frag = builder.build();
        // frag.entry->print();
        std::cout << std::endl << std::endl;
        builder.exportGraph(std::cout);
        BacktrackEngine engine(frag.entry);
        bool result = engine.match(str);
        std::cout << "Result: " << result << std::endl;

        // auto result = (expr->match(slider) && !slider.hasMore());
        // std::cout << "Match result: " << result << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}

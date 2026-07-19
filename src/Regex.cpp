#include "ASTNode.hpp"
#include "NFAState.hpp"
#include "Parser.hpp"
#include "Unicode.hpp"
#include <iostream>
#include <string>

int main(int argc, char *argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0]
                  << " <Regular Expression> <Target String>" << std::endl;
        return 1;
    }

    std::u32string regex = utf8_to_u32(argv[1]);
    std::u32string str = utf8_to_u32(argv[2]);

    // for (char32_t c : regex) {
    //     std::cout << u32_to_utf8(c) << std::endl;
    // }

    Parser parser(regex);
    auto expr = parser.parseExpr();

    expr->print();
    std::cout << std::endl;

    StringSlider slider(str);

    NFABuilder builder(*expr.get());
    NFAFragment frag = builder.build();
    frag.entry->print();
    std::cout << std::endl << std::endl;
    builder.exportToDot(std::cout);

    // auto result = (expr->match(slider) && !slider.hasMore());
    // std::cout << "Match result: " << result << std::endl;
}

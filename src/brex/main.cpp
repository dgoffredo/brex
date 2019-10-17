#include <brex/expand.h>
#include <brex/options.h>
#include <brex/parse.h>

#include <iostream>  // cout, cerr
#include <memory>
#include <ostream>  // ostream::traits_type
#include <string>
#include <utility>

int main(int, char* argv[]) {
    brex::Options options;

    if (brex::parseCommandLine(options, argv, std::cerr)) {
        std::cerr << "\n";
        brex::printUsage(std::cerr, argv[0]);
        return 1;
    }

    if (options.help) {
        brex::printUsage(std::cout, argv[0]);
        return 0;
    }

    std::ostream* const errors = options.verbose ? &std::cerr : nullptr;

    std::string input;
    std::getline(std::cin, input);

    if (std::cin.peek() != std::ostream::traits_type::eof()) {
        // There are characters after the first newline, which violates the
        // specification.
        if (errors) {
            *errors << "Encountered input after initial newline.\n";
        }
        return 2;
    }

    brex::ParseTreeNode parseTree;
    const auto          result = brex::parse(parseTree, input, errors);
    if (result != brex::ParseResult::SUCCESS) {
        return int(result);
    }

    if (options.parse) {
        std::cout << parseTree << "\n";
        return 0;
    }

    const char* const delimiter = options.lines ? "\n" : " ";

    const auto expanderPtr = brex::expander(parseTree);
    brex::expand(std::cout, *expanderPtr, delimiter);
    std::cout << "\n";
}

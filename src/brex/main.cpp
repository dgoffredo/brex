#include <brex/expand.h>
#include <brex/parse.h>

#include <iostream>
#include <memory>
#include <string>
#include <utility>

int main() {
    std::string input;
    std::getline(std::cin, input);

    brex::ParseTreeNode parseTree;
    const auto          result = brex::parse(parseTree, input, &std::cerr);
    if (result != brex::ParseResult::SUCCESS) {
        return int(result);
    }

    std::cout << parseTree << "\n";

    const auto expanderPtr = brex::expander(parseTree);
    brex::expand(std::cout, *expanderPtr, "\n");
    std::cout << "\n";
}

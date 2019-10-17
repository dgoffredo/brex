#include <brex/expand.h>
#include <brex/parse.h>

#include <iostream>
#include <memory>
#include <string>
#include <utility>

int main() {
    /*
        using Ptr = std::unique_ptr<brex::Expander>;

        std::unique_ptr<brex::Alternation> prefixes(new brex::Alternation);
        prefixes->appendChild(Ptr(new brex::String("Well, ")));
        prefixes->appendChild(Ptr(new brex::String("So, ")));

        std::unique_ptr<brex::Sequence> words(new brex::Sequence);
        words->appendChild(Ptr(new brex::String("hello")));
        words->appendChild(Ptr(new brex::String("fried")));
        words->appendChild(Ptr(new brex::String("fish")));

        std::unique_ptr<brex::Alternation> punctuation(new brex::Alternation);
        punctuation->appendChild(Ptr(new brex::String("!")));
        punctuation->appendChild(Ptr(new brex::String("?")));

        brex::Sequence sentence;
        sentence.appendChild(std::move(prefixes));
        sentence.appendChild(std::move(words));
        sentence.appendChild(std::move(punctuation));

        brex::expand(std::cout, sentence, "\n");
        std::cout << "\n";

        brex::ParseTreeNode node;
        brex::parse(node, "dummy", &std::cerr);
    */
    std::string input;
    std::getline(std::cin, input);

    brex::ParseTreeNode parseTree;
    brex::parse(parseTree, input, &std::cerr);

    // std::cout << parseTree << "\n";

    const auto expanderPtr = brex::expander(parseTree);
    brex::expand(std::cout, *expanderPtr, "\n");
    std::cout << "\n";
}

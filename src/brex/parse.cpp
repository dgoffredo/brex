#include <brex/parse.h>

#include <cassert>
#include <ostream>

namespace brex {

// free functions
// --------------

namespace {

const char* toJson(ParseTreeNode::Type type) {
    switch (type) {
        case ParseTreeNode::Type::STRING:
            return "\"STRING\"";
        case ParseTreeNode::Type::SEQUENCE:
            return "\"SEQUENCE\"";
        default:
            assert(type == ParseTreeNode::Type::ALTERNATION);
            return "\"ALTERNATION\"";
    }
}

}  // namespace

void toJson(std::ostream& stream, const ParseTreeNode& node) {
    // clang-format off

    stream << "{"
                  "\"type\": " << toJson(node.type) << ", "
                  "\"byteOffset\": " << node.byteOffset << ", "
                  // Note that we don't need to escape quotes in `node.source`,
                  // because there are none.  Parsing will have failed if any
                  // quotes were encountered, since quotes are not accepted by
                  // the grammar.
                  "\"source\": \"" << node.source << "\"";

    if (!node.children.empty()) {
        assert(node.children[0]);

        stream << ", \"children\": [";
        toJson(stream, *node.children[0]);

        for (int i = 1; i < int(node.children.size()); ++i) {
            const auto& childPointer = node.children[i];
            assert(childPointer);

            stream << ", ";
            toJson(stream, *childPointer);
        }

        stream << "]";
    }

    stream << "}";

    // clang-format on
}

std::ostream& operator<<(std::ostream& stream, const ParseTreeNode& node) {
    toJson(stream, node);
    return stream;
}

ParseResult parse(ParseTreeNode&     output,
                  const std::string& input,
                  std::ostream*      errors) {
    // TODO

    (void)output;
    (void)input;
    (void)errors;

    return ParseResult::SUCCESS;
}

}  // namespace brex

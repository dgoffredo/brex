#include <brex/parse.h>

#include <cassert>
#include <cctype>   // isalpha
#include <cstddef>  // size_t
#include <limits>
#include <ostream>
#include <sstream>  // ostringstream

namespace brex {
namespace {

// This exception does _not_ derive from `std::exception`.  `ParseError` never
// escapes this translation unit, and instead is used only to build a
// diagnostic should parsing fail.
struct ParseError {
    ParseResult code;
    int         byteOffset;
    std::string message;

    explicit ParseError(ParseResult        code,
                        int                byteOffset,
                        const std::string& message)
    : code(code)
    , byteOffset(byteOffset)
    , message(message) {
    }
};

// `ERROR_STREAM` expands to the name of an identifier used in the
// `THROW_ERROR` macro.  It's obfuscated to avoid name collisions.
#define ERROR_STREAM eRrOrStReAm

// `THROW_ERROR` is syntactic sugar for assembling an error diagnostic message
// using a `std::ostringstream` and then copying the constructed string into an
// exception and throwing the exception.
#define THROW_ERROR(ERROR_CODE, BYTE_OFFSET)                            \
    for (std::ostringstream ERROR_STREAM; true;                         \
         throw ParseError(ERROR_CODE, BYTE_OFFSET, ERROR_STREAM.str())) \
    ERROR_STREAM

}  // namespace

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

namespace {

// This function returns a `ParseError` rather than throwing it.  The reason
// for this is that `g++`'s `-Werror=return-type` can't deduce across a
// function call that control flow doesn't continue due to a `throw` statement.
// So, this silly looking `try`-`catch`-`return` is a workaround.
ParseError invalidCharacter(int byteOffset, char offendingCharacter) try {
    THROW_ERROR(ParseResult::INVALID_CHARACTER, byteOffset)
        << "Encountered the character \"" << offendingCharacter << "\" (ASCII "
        << int(offendingCharacter)
        << "), which is not in the allowed character set.  Only upper and "
           "lower case English letters are allowed, and the punctutation "
           "characters \"{\", \"}\", and \",\".";
}
catch (const ParseError& error) {
    return error;
}

bool isAlpha(char ch) {
    // Per the C++ Standard, `std::isalpha` invoked with an argument of type
    // `char` would be undefined behavior, so we cast to `unsigned char` first.
    return std::isalpha(static_cast<unsigned char>(ch));
}

std::unique_ptr<ParseTreeNode> parse(const std::string& input, int byteOffset);

std::unique_ptr<ParseTreeNode> parseAlternation(const std::string& input,
                                                int byteOffset) {
    const int inputSize = input.size();

    assert(byteOffset < inputSize);

    std::unique_ptr<ParseTreeNode> node(new ParseTreeNode);
    node->type       = ParseTreeNode::Type::ALTERNATION;
    node->byteOffset = byteOffset;

    assert(input[byteOffset] == '{');

    ++byteOffset;
    if (byteOffset == inputSize) {
        THROW_ERROR(ParseResult::UNCLOSED_ALTERNATION, byteOffset)
            << "The alternation starting at the last character (byte offset "
            << (byteOffset - 1) << ") was not closed before the end of input.";
    }

    if (input[byteOffset] == '}') {
        THROW_ERROR(ParseResult::EMPTY_ALTERNATION, byteOffset)
            << "An alternation must contain at least one child (e.g. "
               "\"{foo}\"), but encountered one that had none (e.g. \"{}\").";
    }

    // Parse the children of the alternation.
    for (;;) {
        std::unique_ptr<ParseTreeNode> child = parse(input, byteOffset);

        assert(child);
        assert(child->byteOffset == byteOffset);

        byteOffset += child->source.size();

        assert(byteOffset <= inputSize);

        if (byteOffset == inputSize) {
            THROW_ERROR(ParseResult::UNCLOSED_ALTERNATION, byteOffset)
                << "Encountered an alternation that was not closed before the "
                   "end of input.  The alternation began at byte offset "
                << node->byteOffset << ".";
        }

        const char ch = input[byteOffset];
        // If `ch` is ',', then we read the next child.  If it's '}', then
        // we're done.  It can't be anything else (this is a fact about the
        // `parse` routine).

        // Either way, we add the most recently read child to the alternation.
        node->children.push_back(std::move(child));

        if (ch == '}') {
            // We're done.
            ++byteOffset;  // consume the closing brace

            const int endOffset    = byteOffset;
            const int sourceLength = endOffset - node->byteOffset;

            node->source = input.substr(node->byteOffset, sourceLength);
            break;
        }
        else {
            // Loop along to read the next child.
            assert(ch == ',');
            ++byteOffset;  // consume the comma

            // It could be that this comma is the last thing in the input, so
            // report that error.
            if (byteOffset == inputSize) {
                THROW_ERROR(ParseResult::UNCLOSED_ALTERNATION, byteOffset)
                    << "Encountered an alternation that was not closed before "
                       "the end of input.  The alternation began at byte "
                       "offset "
                    << node->byteOffset << ".";
            }
        }
    }

    const int endOffset    = byteOffset;
    const int sourceLength = endOffset - node->byteOffset;

    node->source = input.substr(node->byteOffset, sourceLength);

    return std::move(node);
}

std::unique_ptr<ParseTreeNode> parseString(const std::string& input,
                                           int                byteOffset) {
    const int inputSize = input.size();

    assert(byteOffset < inputSize);

    std::unique_ptr<ParseTreeNode> node(new ParseTreeNode);
    node->type       = ParseTreeNode::Type::STRING;
    node->byteOffset = byteOffset;

    while (byteOffset < inputSize && isAlpha(input[byteOffset])) {
        ++byteOffset;
    }

    // Reached the end of the string, or reached some punctuation, or reached
    // an invalid character.
    if (byteOffset != inputSize) {
        const char ch = input[byteOffset];
        if (ch != '{' && ch != '}' && ch != ',') {
            throw invalidCharacter(byteOffset, ch);
        }
    }

    const int sourceLength = byteOffset - node->byteOffset;
    node->source           = input.substr(node->byteOffset, sourceLength);

    return std::move(node);
}

// Return a node parsed from the specified `input` starting at the specified
// `byteOffset`.  Throw a `ParseError` if an error occurs.  The returned value
// will not be null.  Note that there is sufficient information in the returned
// value to deduce the byte offset just beyond the returned node.
std::unique_ptr<ParseTreeNode> parseOne(const std::string& input,
                                        int                byteOffset) {
    assert(byteOffset >= 0);
    assert(byteOffset < int(input.size()));

    const char ch = input[byteOffset];

    if (ch == '{') {
        return parseAlternation(input, byteOffset);
    }

    if (isAlpha(ch)) {
        return parseString(input, byteOffset);
    }

    // TODO: Consider creating specific diagnostics for empty alternation
    //       options.  Right now the error that is triggered is
    //       "unexpected ','" or "unexpected '}'".  It would be better to
    //       notice the case where you have ",," or ",}".

    // Otherwise it's a parse error.  First check for misplaced special chars.
    if (ch == '}') {
        THROW_ERROR(ParseResult::MISPLACED_CHARACTER, byteOffset)
            << "Encountered an unexpected \"}\" character.  \"}\" can be used "
               "only for alternations, and must have somewhere before it a "
               "matching \"{\".  Also, empty alternation options are not "
               "permitted.";
    }

    if (ch == ',') {
        THROW_ERROR(ParseResult::MISPLACED_CHARACTER, byteOffset)
            << "Encountered an unexpected \",\" character.  \",\" can be used "
               "only to separate options inside of an alternation, and empty "
               "options are not permitted.";
    }

    // `ch` is not any allowed character.
    throw invalidCharacter(byteOffset, ch);
}

std::unique_ptr<ParseTreeNode> parse(const std::string& input,
                                     int                byteOffset) {
    std::unique_ptr<ParseTreeNode> node = parseOne(input, byteOffset);
    assert(node);
    assert(node->byteOffset == byteOffset);

    byteOffset += node->source.size();
    assert(byteOffset <= int(input.size()));

    if (byteOffset == int(input.size())) {
        // reached the end of input
        return std::move(node);
    }

    const char ch = input[byteOffset];
    if (ch == ',' || ch == '}') {
        // reached the middle or end of something
        return std::move(node);
    }

    // The only remaining case is one of the following:
    //
    //     IwasJustParsed{and,now,here}...
    //
    // or
    //
    //     {this,was,parsed}andNowHere
    //
    // Either case is a sequence.  So, keep parsing the rest of the sequence's
    // children.

    const int beginOffset = node->byteOffset;

    std::unique_ptr<ParseTreeNode> sequence(new ParseTreeNode);
    sequence->type       = ParseTreeNode::Type::SEQUENCE;
    sequence->byteOffset = beginOffset;

    sequence->children.push_back(std::move(node));

    for (;;) {
        std::unique_ptr<ParseTreeNode> node = parseOne(input, byteOffset);
        assert(node);
        assert(node->byteOffset == byteOffset);

        byteOffset += node->source.size();
        assert(byteOffset <= int(input.size()));

        sequence->children.push_back(std::move(node));

        if (byteOffset == int(input.size())) {
            // reached the end of input
            break;
        }

        const char ch = input[byteOffset];
        if (ch == ',' || ch == '}') {
            // reached the middle or end of something
            break;
        }
    }

    const int endOffset    = byteOffset;
    const int sourceLength = endOffset - beginOffset;

    sequence->source = input.substr(beginOffset, sourceLength);

    return std::move(sequence);
}

void printDiagnostic(std::ostream&      stream,
                     const ParseError&  error,
                     const std::string& input) {
    const int offset      = error.byteOffset;
    const int inputLength = input.size();

    stream << error.message << "  Error occurred at byte offset " << offset
           << ":\n";

    // Print an excerpt of the `input` (or perhaps the whole thing), with a
    // caret below it pointing at the character at `offset`.  Use "... " and
    // " ..." when eliding the beginning or end of `input`, respectively.
    const int border   = 25;  // how many characters on either side
    const int maxWidth = border + 1 + border;

    if (inputLength <= maxWidth) {
        stream << input << "\n" << std::string(offset, ' ') << '^';
    }
    else {
        // Figure out what to elide.  Prefer to keep the pointed-to character
        // in the center, even if that means eliding more than would otherwise
        // be necessary.
        const std::string elideLeft("... ");
        std::string       prefix;

        if (offset > border) {
            const int leftLength = border - elideLeft.size();
            prefix = elideLeft + input.substr(offset - leftLength, leftLength);
        }
        else {
            prefix = input.substr(0, offset);
        }

        stream << prefix << input[offset];

        const std::string elideRight(" ...");
        std::string       suffix;

        if (inputLength - offset > border) {
            const int rightLength = border - elideRight.size();
            suffix = input.substr(offset + 1, rightLength) + elideRight;
        }
        else {
            suffix = input.substr(offset + 1, inputLength - offset - 1);
        }

        stream << suffix << "\n" << std::string(prefix.size(), ' ') << '^';
    }

    stream << "\n";
}

}  // namespace

ParseResult parse(ParseTreeNode&     output,
                  const std::string& input,
                  std::ostream*      errors) try {
    // We use `int` for byte offsets.  If you're trying to expand gigabytes of
    // curly braces, then someone needs to tell you to stop, and it might as
    // well be me.
    const std::size_t MAX_INPUT_SIZE = std::numeric_limits<int>::max();
    const int         byteOffset     = 0;

    if (input.size() > MAX_INPUT_SIZE) {
        THROW_ERROR(ParseResult::INPUT_TOO_LARGE, byteOffset)
            << "Input is too large (having size " << input.size()
            << " bytes).  Maximum allowed input size is " << MAX_INPUT_SIZE
            << " bytes.";
    }

    if (input.empty()) {
        THROW_ERROR(ParseResult::EMPTY_INPUT, byteOffset)
            << "Cannot parse empty input.";
    }

    // Call an overload of `parse` that returns a
    // `unique_ptr<ParseTreeNode>` (that is also never null), and if all went
    // well, then move the referred-to `ParseTreeNode` into `output`.
    std::unique_ptr<ParseTreeNode> tree(parse(input, byteOffset));
    assert(tree);
    assert(tree->source.size() <= input.size());

    const auto parsedSize = tree->source.size();

    if (parsedSize != input.size()) {
        const char ch = input[parsedSize];
        THROW_ERROR(ParseResult::MISPLACED_CHARACTER, parsedSize)
            << "Parsing ended prematurely at the character \"" << ch
            << "\" (ASCII " << int(ch) << ").";
    }

    output = std::move(*tree);

    return ParseResult::SUCCESS;
}
catch (const ParseError& error) {
    if (errors != nullptr) {
        printDiagnostic(*errors, error, input);
    }

    return error.code;
}

}  // namespace brex

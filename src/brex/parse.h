#ifndef INCLUDED_BREX_PARSE
#define INCLUDED_BREX_PARSE

#include <iosfwd>  // ostream&
#include <memory>  // unique_ptr
#include <string>
#include <vector>

namespace brex {

// `ParseResult` is returned by `parse`.  On success, `ParseResult::SUCCESS` is
// returned.  All other values of `ParseResult` indicate some parsing error.
// Note that the integral values of these constants must be stable, because
// the brex command line program will use these values as the process status
// code when invoked with the `--parse` option.  These status codes are then
// used by white box "integration" tests that drive the brex command line tool.
enum class ParseResult {
    SUCCESS                 = 0,
    INVALID_CHARACTER       = 1,
    EMPTY_ALTERNATION_CHILD = 2,
    EMPTY_ALTERNATION       = 3,
    UNCLOSED_ALTERNATION    = 4,
    MISPLACED_CHARACTER     = 5,
    INPUT_TOO_LARGE         = 7,
    EMPTY_INPUT             = 8
};

struct ParseTreeNode {
    enum class Type {
        STRING,      // e.g. foo
        SEQUENCE,    // e.g. foo{bar,baz}y
        ALTERNATION  // e.g. {bar,baz}
    };

    Type type;

    // where in the input string this node appears, zero-based
    int byteOffset;  // zero-based

    // A copy of the substring within the input from which this node is parsed.
    std::string source;

    // Evidently C++17 can handle `std::vector<ParseTreeNode>` here (and
    // probably many standard libraries handle it as an extension), but for
    // portability I'll use a `std::vector<std::unique_ptr<ParseTreeNode>>`.
    std::vector<std::unique_ptr<ParseTreeNode>> children;
};

// Insert into the specified `stream` a JSON representation of the specified
// `node`.
void toJson(std::ostream& stream, const ParseTreeNode& node);

// Insert into the specified `stream` a JSON representation of the specified
// `node`.  Return a reference providing modifiable access to `stream`.
std::ostream& operator<<(std::ostream& stream, const ParseTreeNode& node);

// Populate the specified `output` with the parse tree of the specified
// `input` shell bracket expression.  Return `ParseResult::SUCCESS` on success
// or another `ParseResult` value if an error occurs.  If an error occurs,
// insert a diagnostic into the optionally specified `errors`.  Also if an
// error occurs, `output` is not modified.
ParseResult parse(ParseTreeNode&     output,
                  const std::string& input,
                  std::ostream*      errors = nullptr);

}  // namespace brex

#endif

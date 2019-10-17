#ifndef INCLUDED_BREX_OPTIONS
#define INCLUDED_BREX_OPTIONS

#include <iosfwd>  // ostream&

namespace brex {

struct Options {
    bool help;     // Print usage instructions to standard output and exit.
    bool verbose;  // Print error diagnostics to standard error.
    bool parse;    // Print a JSON representation of the parse tree to standard
                   // output and exit.
    bool lines;    // Delimit terms of the expansion with a line feed rather
                   // than with a space.

    Options()
    : help(false)
    , verbose(false)
    , parse(false)
    , lines(false) {
    }
};

// Load into the specified `options` program options parsed from the specified
// command line `argv`.  Return zero on success or a nonzero value if an error
// occurs.  If an error occurs, write a diagnostic to the specified `errors`.
// Also if an error occurs, `options` will not be modified.  The behavior is
// undefined if `argv` is null or if `*argv` is null.
int parseCommandLine(Options&           options,
                     const char* const* argv,
                     std::ostream&      errors);

// The behavior is undefined if `programName` is null.
void printUsage(std::ostream& stream, const char* programName);

}  // namespace brex

#endif

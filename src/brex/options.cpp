#include <brex/options.h>

#include <cassert>
#include <ostream>  // operator<<

namespace brex {

int parseCommandLine(Options&           output,
                     const char* const* argv,
                     std::ostream&      errors) {
    assert(argv);

    Options options;

    for (++argv; *argv; ++argv) {
        const std::string arg(*argv);

        if (arg == "--help") {
            options.help = true;
        }
        else if (arg == "--verbose") {
            options.verbose = true;
        }
        else if (arg == "--parse") {
            options.parse   = true;
            options.verbose = true;
        }
        else if (arg == "--lines") {
            options.lines = true;
        }
        else {
            errors << "Unknown command line option: " << arg << "\n";
            return 1;
        }
    }

    output = options;
    return 0;
}

void printUsage(std::ostream& stream, const char* programName) {
    assert(programName);

    stream << "brex - shell brace expander\n\n"
              "Usage:  "
           << programName << " [options ...]\n"
           << R"(
Brex reads a shell brace expression from standard input and prints its
space-delimited expansion to standard output.

Options:

--help       Print this message to standard output and exit.

--verbose    Print error diagnostics to standard output.

--parse      Rather than printing the brace expression's
             expansion, instead print its parse tree as
             JSON to standard output.  This also implies the
             --verbose option.

--lines      Delimit expansion terms using a line feed
             instead of a space.

)";
}

}  // namespace brex

Testing
=======
Types of tests:

- input -> output (core functionality)
- input -> parse tree (tests the parser)
- command line options
    - `--debug` prints errors to standard error.
    - `--parse` dumps the parse tree as JSON to standard output.
    - `--help` prints instructions to standard output and returns zero.

Based on this, there's no need for C++ unit tests.  Functionality can be
tested using the command line driver alone, since its options expose internal
functionality for testing.

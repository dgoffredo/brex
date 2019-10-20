<img src="images/brex.jpg" width="300"/>

brex
====
shell **br**ace **ex**pander

<a href="https://travis-ci.com/dgoffredo/brex">
  <img src="https://travis-ci.com/dgoffredo/brex.svg?branch=master"/>
</a>

Why
---
They want to know whether I can code.

What
----
`brex` is a command line tool that expands somewhat-bash-style
[curly brace expressions][1].  It reads an expression from standard input and
prints the space-separated (or line-separated, if you like) terms of the
expansion to standard output.

```console
$ echo 'ha{x,foo{bar,baz{zy,z}}}{a,b}' | ./brex --lines
haxa
haxb
hafoobara
hafoobarb
hafoobazzya
hafoobazzyb
hafoobazza
hafoobazzb
```

The language understood by `brex` is a simplification of that understood by
bash.

In the following grammar, terminals appearing in forward slashes (`/`) are
Perl-compatible regular expressions, while terminals appearing in double quotes
(`"`) are string literals.  Terms next to each other are concatenated together,
parentheses (`(` and `)`) are used for logical grouping, `*` is the Kleene
star, `?` denotes zero or one of what precedes it, `+` denotes one or more of
what precedes it, and the pipe character (`|`) denotes unordered alternation.

    Expression  ::=  Sequence | Alternation | STRING

    Alternation  ::=  "{" Expression ("," Expression)* "}"

    Sequence  ::=  STRING Alternation+ (STRING Alternation+)* STRING?
                 | Alternation+ STRING (Alternation+ STRING)* Alternation*

    STRING  ::=  /[A-Za-z]+/

How
---
### Command Line Invocation
```console
$ ./brex --help
brex - shell brace expander

Usage:  ./brex [options ...]

Brex reads a shell brace expression from standard input and prints its
space-delimited expansion to standard output.

Options:

--help       Print this message to standard output and exit.

--verbose    Print error diagnostics to standard error.

--parse      Rather than printing the brace expression's
             expansion, instead print its parse tree as
             JSON to standard output.  This also implies the
             --verbose option.

--lines      Delimit expansion terms using a line feed
             instead of a space.
```

### Building
Run `make` in the repository to produce the executable `./brex`.  The build
is in-place, which I know is archaic, but it's the default for `make`.

There's no easy way to build brex on Windows, either, but that's okay.  `cmake`
seems a bit much for this project.

The [Makefile](./Makefile) understands two values for the `BUILD_TYPE`
variable:

- `Release`, the default, enables all of the optimizations and omits debugging
  information.
- `Debug` disables all optimizations and includes debugging information.

`make clean` deletes all build artifacts and targets.

### Testing
`make test` will run all of the tests in [test/](test/) using Python 3.7.  The
tests invoke the `brex` binary and examine its output and status code.

The tests can also be run individually.  The path to the `brex` binary can be
specified by setting the "BREX" environment variable.  The default path is
"./brex".

More
----
### Build Dependencies
- GNU make
- a `g++`-like C++11 compiler (in terms of command line options)
- `find`
- `sed`
- POSIX `sh`

### Test Dependencies
- Python 3.7
- `/usr/bin/env`

### Other Dependencies
- [bin/format](bin/format) requires `clang-format-9`, `dirname`, `realpath`,
  and `find`.

### Hacking
Please keep all source code automatically formatted using the
[bin/format](bin/format) script.  It uses `clang-format-9` to format all source
code according to the style in the [.clang-format](./.clang-format) file.

I run the formatter before every commit.

[1]: https://www.gnu.org/software/bash/manual/html_node/Brace-Expansion.html
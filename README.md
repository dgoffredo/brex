<img src="images/brex.jpg" width="300"/>

brex
====
shell **br**ace **ex**pander

Why
---
They want to know whether I can code.

What
----
`brex` is a command line tool that expands somewhat-bash-style
[curly brace expressions][1].  It reads an expression from standard input and
prints the space-separated terms of the expansion to standard output.

```console
$ echo 'ha{x,foo{bar,baz{zy,z}}}{a,b}' | ./brex | tr ' ' '\n'
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
TODO

### Building
TODO

### Testing
TODO

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

[1]: https://www.gnu.org/software/bash/manual/html_node/Brace-Expansion.html
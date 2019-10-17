#!/usr/bin/env python3.7

import common

import json
import unittest


# These error code values are copied from brex's source code.
# See `enum class ParseResult` in  `src/brex/parse.h`.
class Errors:
    SUCCESS                 = 0
    INVALID_CHARACTER       = 1
    EMPTY_ALTERNATION_CHILD = 2
    EMPTY_ALTERNATION       = 3
    UNCLOSED_ALTERNATION    = 4
    MISPLACED_CHARACTER     = 5
    INPUT_TOO_LARGE         = 7
    EMPTY_INPUT             = 8


class TestParsing(unittest.TestCase):
    def assert_parse_error(self, error_code, input):
        status, stdout, stderr = common.brex(input, ['--parse'])

        # The status code will be the parser error code.
        self.assertEqual(status, error_code)

        # --parse implies --verbose, so a diagnostic will be in stderr
        self.assertNotEqual(stderr, '')

        # Since parsing failed, there will be no standard output
        self.assertEqual(stdout, '')

    def test_invalid_character(self):
        alpha = 'abcdefghijklmnopqrstuvwxyz'
        allowed = alpha + alpha.upper() + '{},'
        
        # Take an arbitrarily chosen valid input, and for each invalid ASCII
        # character, insert the invalid character a few positions within the
        # input, and verify that parsing fails with the `INVALID_CHARACTER`
        # error each time.
        valid_input = 'a{b,{c{zy,z}}}d'
        positions = [0, 2, 10, len(valid_input)]  # just a few, to save time

        for ascii_code in range(128):
            char = chr(ascii_code)
            if char in allowed or char == '\n':
                continue

            for i in positions:
                bad_input = valid_input[:i] + char + valid_input[i:] + '\n'

                self.assert_parse_error(Errors.INVALID_CHARACTER, bad_input)

    def test_empty_alternation(self):
        self.assert_parse_error(Errors.EMPTY_ALTERNATION, 'foo{}bar')

    def test_unclosed_alternation(self):
        for input in ['a{{b}c', 'd{']:
            self.assert_parse_error(Errors.UNCLOSED_ALTERNATION, input)

    def test_misplaced_character(self):
        for input in ['a,b', '}c']:
            self.assert_parse_error(Errors.MISPLACED_CHARACTER, input)

    def test_empty_input(self):
        self.assert_parse_error(Errors.EMPTY_INPUT, '')

    def test_parse_tree(self):
        # example from the readme file (as of this writing)
        input = 'ha{x,foo{bar,baz{zy,z}}}{a,b}\n'
        expected_output = {"type": "SEQUENCE", "byteOffset": 0, "source": "ha{x,foo{bar,baz{zy,z}}}{a,b}", "children": [{"type": "STRING", "byteOffset": 0, "source": "ha"}, {"type": "ALTERNATION", "byteOffset": 2, "source": "{x,foo{bar,baz{zy,z}}}", "children": [{"type": "STRING", "byteOffset": 3, "source": "x"}, {"type": "SEQUENCE", "byteOffset": 5, "source": "foo{bar,baz{zy,z}}", "children": [{"type": "STRING", "byteOffset": 5, "source": "foo"}, {"type": "ALTERNATION", "byteOffset": 8, "source": "{bar,baz{zy,z}}", "children": [{"type": "STRING", "byteOffset": 9, "source": "bar"}, {"type": "SEQUENCE", "byteOffset": 13, "source": "baz{zy,z}", "children": [{"type": "STRING", "byteOffset": 13, "source": "baz"}, {"type": "ALTERNATION", "byteOffset": 16, "source": "{zy,z}", "children": [{"type": "STRING", "byteOffset": 17, "source": "zy"}, {"type": "STRING", "byteOffset": 20, "source": "z"}]}]}]}]}]}, {"type": "ALTERNATION", "byteOffset": 24, "source": "{a,b}", "children": [{"type": "STRING", "byteOffset": 25, "source": "a"}, {"type": "STRING", "byteOffset": 27, "source": "b"}]}]}

        status, stdout, stderr = common.brex(input, ['--parse', '--verbose'])

        self.assertEqual(status, 0)  # success
        self.assertEqual(stderr, '')  # no diagnostics, even with --verbose

        parsed_output = json.loads(stdout)
        self.assertEqual(parsed_output, expected_output)


if __name__ == '__main__':
    unittest.main()

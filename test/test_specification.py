#!/usr/bin/env python3.7

import common
import unittest


pass_examples = {
    '{A,B,C}':    'A B C',
    '{A,B}{C,D}': 'AC AD BC BD',
    '{A,B{C,D}}': 'A BC BD',
    '{ABC}':      'ABC',
    'ABC':        'ABC'
}


fail_examples = [
    '}ABC',
    '{ABC',
    '}{',
    '{}',
    'A,B,C',
    '{A{B,C}',
    '{A,}'
]


class TestSpecification(unittest.TestCase):
    def test_pass_examples(self):
        for input, expected_output in pass_examples.items():
            status, stdout, stderr = common.brex(input + '\n')
            self.assertEqual(status, 0)   # success
            self.assertEqual(stderr, '')  # --verbose not indicated
            self.assertEqual(stdout, expected_output + '\n')

    def test_fail_examples(self):
        for input in fail_examples:
            status, stdout, stderr = common.brex(input + '\n')
            self.assertNotEqual(status, 0)  # _not_ success
            self.assertEqual(stderr, '')    # --verbose not indicated
            self.assertEqual(stdout, '')    # error means no standard output


if __name__ == '__main__':
    unittest.main()

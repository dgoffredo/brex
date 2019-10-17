# This is code shared by the other test drivers.  It is meant to be imported
# as a python module, not to be executed as a script.

import os
import subprocess

def brex(input, flags=[], executable_path=None):
    """Run the brex command line tool at the optionally specified
    `executable_path`, supplying the specified standard `input` and command
    line `flags`.  If `executable_path` is `None`, then use the value of the
    "BREX" environment variable.  If "BREX" is not in the environment, then
    use "./brex".  Return a tuple of three elements:

    - the integer status code returned by the brex subprocess,
    - a string containing the contents of the subprocess's standard output,
    - a string containing the contents of the subprocess's standard error.
    """
    if executable_path is not None:
        path = executable_path
    elif 'BREX' in os.environ:
        path = os.environ['BREX']
    else:
        path = './brex'

    result = subprocess.run([path] + flags,
                            input=input,
                            capture_output=True,
                            encoding='utf8')

    return result.returncode, result.stdout, result.stderr

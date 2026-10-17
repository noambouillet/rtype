#!/usr/bin/env python3
"""
Runs the R-Type functional tests against the built binaries.

    python3 tests/functional/run_tests.py                     # everything, binaries in ./build
    python3 tests/functional/run_tests.py --build-dir build-tests
    python3 tests/functional/run_tests.py test_session        # one file
    python3 tests/functional/run_tests.py test_session.LeaveTest.test_free_slot_is_reused
    python3 tests/functional/run_tests.py -k latency          # tests whose name contains "latency"
"""

import argparse
import os
import sys
import unittest
from pathlib import Path

HERE = Path(__file__).resolve().parent


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--build-dir", help="directory containing r-type_server and r-type_client (default: build)")
    parser.add_argument("-k", dest="patterns", action="append", help="only run tests matching this substring")
    parser.add_argument("-q", "--quiet", action="store_true", help="one character per test instead of one line")
    parser.add_argument("-x", "--failfast", action="store_true", help="stop at the first failure")
    parser.add_argument("tests", nargs="*", help="modules, classes or tests, e.g. test_session.JoinTest")
    args = parser.parse_args()

    if args.build_dir:
        os.environ["RTYPE_BUILD_DIR"] = str(Path(args.build_dir).resolve())
    sys.path.insert(0, str(HERE))
    loader = unittest.TestLoader()
    if args.patterns:
        loader.testNamePatterns = [f"*{pattern}*" for pattern in args.patterns]
    if args.tests:
        suite = loader.loadTestsFromNames(args.tests)
    else:
        suite = loader.discover(str(HERE), top_level_dir=str(HERE))
    runner = unittest.TextTestRunner(verbosity=1 if args.quiet else 2, failfast=args.failfast)
    return 0 if runner.run(suite).wasSuccessful() else 1


if __name__ == "__main__":
    sys.exit(main())

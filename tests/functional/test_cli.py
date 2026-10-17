"""The binaries start, print their usage and reject bad arguments."""

import signal
import subprocess
import sys
import unittest

from rtype.harness import ServerProcess, binary


def run(name, *args):
    return subprocess.run([str(binary(name)), *args], capture_output=True, text=True, timeout=5)


class ServerCommandLineTest(unittest.TestCase):
    def test_help_prints_usage(self):
        result = run("r-type_server", "--help")
        self.assertEqual(result.returncode, 0)
        self.assertIn("Usage: r-type_server", result.stdout)

    def test_invalid_arguments_exit_with_code_2(self):
        for args in (["0"], ["65536"], ["abc"], ["6767", "6768"], ["--unknown"]):
            with self.subTest(args=args):
                result = run("r-type_server", *args)
                self.assertEqual(result.returncode, 2)
                self.assertIn("Usage", result.stderr)

    def test_port_already_in_use_exits_with_error(self):
        first = ServerProcess().start()
        self.addCleanup(first.cleanup)
        second = run("r-type_server", str(first.port))
        self.assertEqual(second.returncode, 1, second.stdout + second.stderr)
        self.assertTrue(first.is_running())

    @unittest.skipIf(sys.platform == "win32", "SIGTERM does not exist on Windows")
    def test_stops_cleanly_on_sigterm(self):
        server = ServerProcess().start()
        self.addCleanup(server.cleanup)
        self.assertEqual(server.stop(), 0, server.log())
        self.assertIn("Goodbye", server.log())


class ClientCommandLineTest(unittest.TestCase):
    def test_help_prints_usage(self):
        result = run("r-type_client", "--help")
        self.assertEqual(result.returncode, 0)
        self.assertIn("Usage: r-type_client", result.stdout)

    def test_invalid_arguments_exit_with_code_2(self):
        for args in (["localhost", "0"], ["localhost", "70000"], ["--unknown"], ["host", "6767", "extra"]):
            with self.subTest(args=args):
                self.assertEqual(run("r-type_client", *args).returncode, 2)


if __name__ == "__main__":
    unittest.main()

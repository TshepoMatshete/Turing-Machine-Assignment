#!/usr/bin/env python3
"""
run_tests.py

Usage:
    python3 run_tests.py <source.cpp> <tests_dir> [--timeout SECONDS] [--ignore-ws]

Test directory format:
    For each test case provide an input file and an expected output file.
    The script will pair files by basename. Allowed input extensions: .in, .input, .txt
    Allowed expected-output extensions: .out, .ans, .expected

Example:
    tests/
      case1.in
      case1.out
      case2.in
      case2.out
"""

import argparse
import subprocess
import tempfile
import shutil
import os
import sys
import time
import difflib

# ---------- Terminal colors ----------
class TermColor:
    RESET = "\033[0m"
    BOLD = "\033[1m"
    RED = "\033[31m"
    GREEN = "\033[32m"
    YELLOW = "\033[33m"
    CYAN = "\033[36m"
    MAGENTA = "\033[35m"

def colored(text, color):
    if sys.stdout.isatty():
        return f"{color}{text}{TermColor.RESET}"
    return text

# ---------- Helpers ----------
INPUT_EXTS = (".in", ".input", ".txt")
OUTPUT_EXTS = (".out", ".ans", ".expected", ".exp")

def find_test_pairs(tests_dir):
    files = sorted(os.listdir(tests_dir))
    inputs = {}
    outputs = {}
    for f in files:
        name, ext = os.path.splitext(f)
        if ext.lower() in INPUT_EXTS:
            inputs[name] = os.path.join(tests_dir, f)
        if ext.lower() in OUTPUT_EXTS:
            outputs[name] = os.path.join(tests_dir, f)
    pairs = []
    # prefer matching basenames that appear in either inputs or outputs
    all_names = sorted(set(list(inputs.keys()) + list(outputs.keys())))
    for name in all_names:
        in_path = inputs.get(name)
        out_path = outputs.get(name)
        if in_path and out_path:
            pairs.append((name, in_path, out_path))
        elif in_path and not out_path:
            print(colored(f"Warning: input '{name}' has no matching expected output. Skipping.", TermColor.YELLOW))
        elif out_path and not in_path:
            print(colored(f"Warning: expected output '{name}' has no matching input. Skipping.", TermColor.YELLOW))
    return pairs

def read_file(path):
    with open(path, "r", encoding="utf-8", errors="replace") as f:
        return f.read()

def normalize(text, ignore_ws=False):
    if ignore_ws:
        # split into lines and strip trailing whitespace; remove trailing blank lines
        lines = [line.rstrip() for line in text.splitlines()]
        # optionally collapse repeated blank lines? For now keep as-is
        # strip final blank lines
        while lines and lines[-1] == "":
            lines.pop()
        return "\n".join(lines)
    else:
        # trim final newline(s) for stable comparison
        return text.rstrip()

# ---------- Main ----------
def main():
    parser = argparse.ArgumentParser(description="Compile C++ and run testcases.")
    parser.add_argument("source", help="C++ source file (e.g. main.cpp)")
    parser.add_argument("tests", help="Directory containing test case .in/.out pairs")
    parser.add_argument("--timeout", "-t", type=float, default=2.0, help="Seconds per test (default: 2.0)")
    parser.add_argument("--ignore-ws", action="store_true", help="Ignore trailing whitespace differences")
    parser.add_argument("--flags", default="-std=c++17 -O2 -Wall -Wextra", help="Extra flags passed to g++ (quoted)")
    args = parser.parse_args()

    src = args.source
    tests_dir = args.tests

    if not os.path.isfile(src):
        print(colored(f"Error: source file '{src}' not found.", TermColor.RED))
        sys.exit(2)
    if not os.path.isdir(tests_dir):
        print(colored(f"Error: tests directory '{tests_dir}' not found.", TermColor.RED))
        sys.exit(2)

    pairs = find_test_pairs(tests_dir)
    if not pairs:
        print(colored("No test pairs found. Make sure files have matching basenames (e.g. case1.in & case1.out).", TermColor.RED))
        sys.exit(3)

    # compile
    tmpdir = tempfile.mkdtemp(prefix="cpp_test_")
    exe_name = os.path.join(tmpdir, "prog")
    if os.name == "nt":
        exe_name += ".exe"

    compile_cmd = f"g++ {args.flags} -o {exe_name} {src}"
    print(colored("Compiling:", TermColor.CYAN), compile_cmd)
    comp_proc = subprocess.run(compile_cmd, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
    if comp_proc.returncode != 0:
        print(colored("Compilation failed:", TermColor.RED))
        print(comp_proc.stderr)
        shutil.rmtree(tmpdir)
        sys.exit(4)
    else:
        print(colored("Compilation succeeded.\n", TermColor.GREEN))

    # run tests
    total = len(pairs)
    passed = 0
    results = []

    header = colored("═" * 60, TermColor.MAGENTA)
    print(header)
    start_all = time.time()
    for idx, (name, in_path, out_path) in enumerate(pairs, start=1):
        print(colored(f"Test {idx}/{total} · {name}", TermColor.BOLD + TermColor.CYAN))
        inp = read_file(in_path)
        expected_raw = read_file(out_path)

        try:
            run_start = time.time()
            proc = subprocess.run([exe_name], input=inp, text=True, capture_output=True, timeout=args.timeout)
            run_time = time.time() - run_start
            stdout = proc.stdout
            stderr = proc.stderr
            exitcode = proc.returncode
            runtime_error = (exitcode != 0)
        except subprocess.TimeoutExpired as ex:
            run_time = args.timeout
            stdout = ex.stdout or ""
            stderr = ""
            runtime_error = True
            exitcode = None
            timed_out = True
        else:
            timed_out = False

        got = normalize(stdout, ignore_ws=args.ignore_ws)
        want = normalize(expected_raw, ignore_ws=args.ignore_ws)

        if (not runtime_error) and (got == want):
            passed += 1
            results.append((name, True, run_time, exitcode, None))
            print(colored("  ✔ Passed", TermColor.GREEN), f"({run_time:.3f}s)")
        else:
            results.append((name, False, run_time, exitcode, {"got": got, "want": want, "stderr": stderr, "timed_out": timed_out}))
            print(colored("  ✖ Failed", TermColor.RED), f"({run_time:.3f}s)")
            if timed_out:
                print(colored(f"    Reason: timed out after {args.timeout:.2f}s", TermColor.YELLOW))
            else:
                if exitcode is not None and exitcode != 0:
                    print(colored(f"    Program exited with code {exitcode}", TermColor.YELLOW))
                if stderr:
                    print(colored("    stderr:", TermColor.YELLOW))
                    for line in stderr.splitlines():
                        print("     ", colored(line, TermColor.YELLOW))

            # show a compact unified diff
            want_lines = want.splitlines()
            got_lines = got.splitlines()
            diff = list(difflib.unified_diff(want_lines, got_lines, fromfile="expected", tofile="got", lineterm=""))
            if diff:
                print(colored("    Diff (expected -> got):", TermColor.MAGENTA))
                # limit the length of diff to keep output readable
                max_lines = 60
                for i, line in enumerate(diff):
                    if i >= max_lines:
                        print("    " + colored("... diff truncated ...", TermColor.YELLOW))
                        break
                    if line.startswith("---") or line.startswith("+++"):
                        print("    " + colored(line, TermColor.BOLD))
                    elif line.startswith("@@"):
                        print("    " + colored(line, TermColor.CYAN))
                    elif line.startswith("-"):
                        print("    " + colored(line, TermColor.GREEN))  # expected (removed)
                    elif line.startswith("+"):
                        print("    " + colored(line, TermColor.RED))    # got (added)
                    else:
                        print("    " + line)
            else:
                print(colored("    (Outputs differ but no diff could be produced.)", TermColor.YELLOW))
        print("-" * 60)

    total_time = time.time() - start_all
    # summary
    print(header)
    passed_text = colored(f"{passed}/{total} tests passed", TermColor.BOLD + (TermColor.GREEN if passed==total else TermColor.YELLOW))
    print(passed_text, f" — total time {total_time:.3f}s")
    if passed != total:
        fails = [r for r in results if not r[1]]
        print(colored(f"\nFailed tests ({len(fails)}):", TermColor.RED))
        for name, ok, rt, code, extra in fails:
            note = []
            if extra and extra.get("timed_out"):
                note.append("timeout")
            elif code is None:
                note.append("no exit code")
            elif code != 0:
                note.append(f"exit {code}")
            print(" -", colored(name, TermColor.BOLD), f"({', '.join(note)})")
    else:
        print(colored("\nAll tests passed. Nice! 🎉", TermColor.GREEN + TermColor.BOLD))

    # cleanup
    try:
        shutil.rmtree(tmpdir)
    except Exception:
        pass

if __name__ == "__main__":
    main()
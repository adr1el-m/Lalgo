"""Check a LAlgo source file against the proposal's EBNF."""

import argparse
from pathlib import Path
import sys

from grammar import lex_file, load_rules, recognize


def main():
    cli = argparse.ArgumentParser(description=__doc__)
    cli.add_argument("source", type=Path)
    args = cli.parse_args()
    try:
        tokens = lex_file(args.source)
    except (OSError, ValueError) as error:
        print(error, file=sys.stderr)
        return 1

    farthest = 0
    expected = set()

    def record_mismatch(position, terminal):
        nonlocal farthest
        if position > farthest:
            farthest = position
            expected.clear()
        if position == farthest:
            expected.add(terminal)

    try:
        trees = recognize(load_rules(), tokens, "Program", record_mismatch)
    except RecursionError:
        print("Syntax checker reached its nesting limit.", file=sys.stderr)
        return 1
    if len(trees) == 1:
        print(f"Syntax OK: {args.source}")
        return 0
    if len(trees) > 1:
        print("Syntax check failed: multiple parse trees for the complete program.", file=sys.stderr)
        return 1

    kind, spelling, line, column = tokens[min(farthest, len(tokens) - 1)]
    found = "end of file" if kind == "EOF" else repr(spelling)
    options = ", ".join(sorted(expected)) or "a valid statement"
    print(f"Syntax error on line {line}, column {column}: found {found}. Expected {options}.",
          file=sys.stderr)
    return 1


if __name__ == "__main__":
    raise SystemExit(main())

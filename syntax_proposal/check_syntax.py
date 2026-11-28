"""Check a LAlgo source file against the proposal's EBNF."""

import argparse
from pathlib import Path
import sys

from grammar import Node, lex_file, load_rules, recognize


def print_tree(root, out):
    print("Parse tree", file=out)
    pending = [(root, "", "")]
    while pending:
        item, prefix, branch = pending.pop()
        if isinstance(item, Node):
            print(f"{prefix}{branch}{item.name}", file=out)
            child_prefix = prefix + {"": "", "|-- ": "|   ", "`-- ": "    "}[branch]
            for index in range(len(item.children) - 1, -1, -1):
                child_branch = "`-- " if index == len(item.children) - 1 else "|-- "
                pending.append((item.children[index], child_prefix, child_branch))
        else:
            label = item if item.isprintable() else repr(item)
            print(f"{prefix}{branch}{label}", file=out)


def main():
    cli = argparse.ArgumentParser(description=__doc__)
    cli.add_argument("source", type=Path)
    cli.add_argument("--tree-file", type=Path, help="write the parse tree to a new file")
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
        if args.tree_file is not None:
            try:
                with args.tree_file.open("x", encoding="utf-8") as out:
                    print_tree(trees[0], out)
            except OSError as error:
                print(f"Cannot write parse tree: {error}", file=sys.stderr)
                return 2
        print(f"Syntax OK: {args.source}")
        if args.tree_file is None:
            print_tree(trees[0], sys.stdout)
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

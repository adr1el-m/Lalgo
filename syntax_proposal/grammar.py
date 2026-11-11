"""Load LAlgo EBNF and recognize a sequence of lexer terminals."""

from dataclasses import dataclass
from functools import cache
from pathlib import Path
import os
import re
import subprocess
import tempfile

HERE = Path(__file__).resolve().parent
ROOT = HERE.parent

TOKEN_CLASSES = {
    "IDENT": "IDENT",
    "INT_LIT": "INTEGER",
    "DEC_LIT": "DECIMAL",
    "CHAR_LIT": "CHAR",
    "STR_LIT": "STRING",
    "EOF": "EOF",
}


class Ebnf:
    """Read the EBNF subset used in lalgo.ebnf."""

    def __init__(self, text):
        self.tokens = re.findall(r'"[^"\n]*"|\?[^?]+\?|[A-Za-z_]\w*|[=;|,(){}\[\]-]|\S', text)
        self.pos = 0

    def peek(self):
        return self.tokens[self.pos] if self.pos < len(self.tokens) else None

    def take(self, expected=None):
        actual = self.peek()
        if actual is None or (expected is not None and actual != expected):
            raise ValueError(f"Expected {expected!r}, found {actual!r}")
        self.pos += 1
        return actual

    def list_of(self, parse, separator, kind):
        items = [parse()]
        while self.peek() == separator:
            self.take()
            items.append(parse())
        return items[0] if len(items) == 1 else (kind, tuple(items))

    def choice(self):
        return self.list_of(self.sequence, "|", "choice")

    def sequence(self):
        return self.list_of(self.factor, ",", "sequence")

    def factor(self):
        left = self.atom()
        if self.peek() == "-":
            self.take()
            return ("except", (left, self.atom()))
        return left

    def atom(self):
        token = self.take()
        if token in ("(", "[", "{"):
            inner = self.choice()
            self.take({"(": ")", "[": "]", "{": "}"}[token])
            return inner if token == "(" else ("optional" if token == "[" else "repeat", inner)
        if token.startswith('"'):
            return ("literal", token[1:-1])
        if token.startswith("?"):
            return ("special", token[1:-1].strip())
        if re.fullmatch(r"[A-Za-z_]\w*", token):
            return ("name", token)
        raise ValueError(f"Unexpected atom {token!r}")

    def rules(self):
        result = {}
        while self.peek() is not None:
            name = self.take()
            if name in result or not re.fullmatch(r"[A-Z][a-zA-Z]+", name):
                raise ValueError(f"Invalid or duplicate rule {name}")
            self.take("=")
            result[name] = self.choice()
            self.take(";")
        return result


@dataclass(frozen=True)
class Node:
    name: str
    children: tuple


def recognize(rules, tokens, start, on_mismatch=None):
    """Explore top-down, preserving all candidate trees until the span is known."""

    def mismatch(pos, terminal):
        if on_mismatch is not None:
            on_mismatch(pos, terminal)
        return ()

    @cache
    def rule(name, pos):
        return tuple((end, (Node(name, children),)) for end, children in match(rules[name], pos))

    @cache
    def match(expr, pos):
        kind, value = expr
        if kind == "name":
            if value not in TOKEN_CLASSES:
                return rule(value, pos)
            if pos < len(tokens) and tokens[pos][0] == TOKEN_CLASSES[value]:
                spelling = value if value == "EOF" else f"{value}:{tokens[pos][1]}"
                return ((pos + 1, (spelling,)),)
            return mismatch(pos, value)
        if kind == "literal":
            if pos < len(tokens) and tokens[pos][1] == value:
                return ((pos + 1, (repr(value),)),)
            return mismatch(pos, repr(value))
        if kind == "choice":
            return tuple(item for option in value for item in match(option, pos))
        if kind == "sequence":
            states = ((pos, ()),)
            for part in value:
                states = tuple(
                    (end, left + right)
                    for middle, left in states
                    for end, right in match(part, middle)
                )
            return states
        if kind == "optional":
            return ((pos, ()),) + match(value, pos)
        if kind == "repeat":
            states = [(pos, ())]
            for middle, left in states:
                for end, right in match(value, middle):
                    if end == middle:
                        raise ValueError("A repeated expression must consume input")
                    states.append((end, left + right))
            return tuple(states)
        if kind == "except":
            included, excluded = value
            if excluded == ("special", "span contains no WHILE keyword token"):
                return tuple(
                    (end, tree)
                    for end, tree in match(included, pos)
                    if any(token[0] == "KW_WHILE" for token in tokens[pos:end])
                )
            forbidden_ends = {end for end, _ in match(excluded, pos)}
            return tuple((end, tree) for end, tree in match(included, pos) if end not in forbidden_ends)
        raise ValueError(f"Unknown expression {expr!r}")

    return [children[0] for end, children in rule(start, 0) if end == len(tokens)]


def load_rules():
    return Ebnf((HERE / "lalgo.ebnf").read_text()).rules()


def lex_file(source, *, cwd=None):
    """Read the lexer stream in source order, preserving full lexemes and locations."""
    lexer = ROOT / ("build/lalgo.exe" if os.name == "nt" else "build/lalgo")
    if not lexer.is_file():
        raise OSError("Build the lexer first with make lexer.")
    process = subprocess.run(
        [str(lexer), str(source.resolve()), "p"],
        cwd=cwd, capture_output=True, text=True,
    )
    if process.returncode:
        raise ValueError(process.stderr.strip() or "The lexer failed.")
    rows = []
    for line in process.stdout.split("\n"):
        if line:
            kind, row, column, spelling = line.split("\t", 3)
            rows.append((kind, spelling, int(row), int(column)))
    if not rows or rows[-1][0] != "EOF":
        raise ValueError("The lexer stream is missing its EOF terminal.")
    return tuple(rows)


def lex(source, with_eof=False):
    """Lex a generated paper example without overwriting the user's symbol table."""
    with tempfile.TemporaryDirectory(prefix="lalgo-example-") as directory:
        path = Path(directory) / "example.lal"
        path.write_text(source, encoding="ascii")
        rows = lex_file(path, cwd=directory)
        return rows if with_eof else rows[:-1]

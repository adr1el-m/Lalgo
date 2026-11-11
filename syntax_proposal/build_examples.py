"""Generate Chapter 3 and verify its example trees against the unified lexer."""

from dataclasses import dataclass
from functools import cache
import json
from pathlib import Path
import re
import sys

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "tests"))
from test_lexer import run

HERE = Path(__file__).resolve().parent
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


def recognize(rules, tokens, start):
    """Explore top-down, preserving all candidate trees until the span is known."""

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
            return ()
        if kind == "literal":
            if pos < len(tokens) and tokens[pos][1] == value:
                return ((pos + 1, (repr(value),)),)
            return ()
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


def numbered_tree(root, rightmost):
    """Replay leftmost or rightmost expansions, then label the tree with the order."""
    frontier = [((), root)]
    numbers = {}
    while any(isinstance(node, Node) for _, node in frontier):
        expandable = [i for i, (_, node) in enumerate(frontier) if isinstance(node, Node)]
        index = expandable[-1] if rightmost else expandable[0]
        path, node = frontier[index]
        numbers[path] = len(numbers) + 1
        frontier[index:index + 1] = [
            (path + (i,), child) for i, child in enumerate(node.children)
        ]

    def render(node, path=()):
        if not isinstance(node, Node):
            yield node
            return
        yield f"{node.name}#{numbers[path]}["
        for i, child in enumerate(node.children):
            yield from render(child, path + (i,))
        yield "]"

    return format_tree(render(root)), [leaf for _, leaf in frontier]


def format_tree(tokens, width=52):
    """Wrap between complete labels and preserve the bracket hierarchy."""
    parts = []
    for token in tokens:
        if token == "]" and parts and parts[-1].endswith("]"):
            parts[-1] += token
        else:
            parts.append(token)
    lines = []
    line = ""
    for part in parts:
        if not line:
            line = part
        elif len(line) + 1 + len(part) <= width:
            line += " " + part
        else:
            lines.append(line)
            line = "  " + part
    if line:
        lines.append(line)
    return "\n".join(lines)


def lex(source, with_eof=False):
    process, rows, _ = run(ROOT / "build/lalgo", source)
    if process.returncode or not rows or rows[-1][0] != "EOF":
        raise ValueError(f"Lexer failed on {source!r}: {process.stdout}\n{process.stderr}")
    return tuple(
        (kind, spelling) for kind, spelling, *_ in rows
        if kind not in {"LINE_COMMENT", "BLOCK_COMMENT"} and (with_eof or kind != "EOF")
    )


def listing(text, options):
    return f"\\begin{{lstlisting}}[{options}]\n{text}\n\\end{{lstlisting}}\n"


def main():
    ebnf = (HERE / "lalgo.ebnf").read_text()
    rules = Ebnf(ebnf).rules()
    examples = json.loads((HERE / "examples.json").read_text())
    if list(rules) != list(examples) or len(set(examples.values())) != len(examples):
        raise ValueError("Each rule needs exactly one distinct example in grammar order")
    report = []
    for index, (name, source) in enumerate(examples.items(), 1):
        matches = recognize(rules, lex(source, with_eof=name == "Program"), name)
        if len(matches) != 1:
            raise ValueError(f"{name}: expected one complete tree for {source!r}, got {len(matches)}")
        left, left_leaves = numbered_tree(matches[0], False)
        right, right_leaves = numbered_tree(matches[0], True)
        if left_leaves != right_leaves:
            raise ValueError(f"{name}: the two tree views have different yields")
        heading = f"\\textbf{{R{index:02d} \\quad \\texttt{{{name}}}}}\\par\n"
        sample = listing(source, "aboveskip=3pt,belowskip=4pt")
        trees = []
        for label, view in [("Leftmost tree", left), ("Rightmost tree", right)]:
            trees.append(
                "\\begin{minipage}[t]{.485\\linewidth}\n"
                f"{{\\small\\bfseries\\color{{pupmaroon}} {label}}}\\par\n"
                + listing(view, "language={},basicstyle=\\scriptsize\\ttfamily,framesep=3pt,"
                          "breaklines=false,"
                          "xleftmargin=0pt,xrightmargin=0pt,aboveskip=2pt,belowskip=0pt")
                + "\\end{minipage}\n"
            )
        report.append(
            "\\par\\noindent\\begin{minipage}{\\linewidth}\n"
            + heading + sample + "\\noindent\n" + "\\hfill\n".join(trees)
            + "\\end{minipage}\\par\\medskip\n"
        )

    for path in [HERE / "binary_search.lal", HERE / "checking_sample.lal"]:
        source = path.read_text()
        if len(recognize(rules, lex(source, with_eof=True), "Program")) != 1:
            raise ValueError(f"{path.name} must have exactly one complete program tree")

    for source in [
        "FOR (TRUE) {}",
        "OUTPUT x",
        "OUTPUT \"text\", 42;",
        "INTEGER x, y = 1;",
        "IF (a) {} ELSE IF (b) {}",
        "WHILE (a) { INPUT x; ",
        "TRACE x;",
    ]:
        if recognize(rules, lex(source, with_eof=True), "Program"):
            raise ValueError(f"Invalid program accepted: {source!r}")

    # A loop nested under an IF still counts, whereas the word inside a string does not.
    for source, form in [
        ('WHILE (active) { OUTPUT "WHILE"; }', "PlainWhile"),
        ("WHILE (active) { IF (ready) { WHILE (again) {} } }", "NestedWhile"),
    ]:
        if len(recognize(rules, lex(source), form)) != 1:
            raise ValueError(f"Incorrect loop classification: {source}")

    output = ROOT / "build/syntax-proposal"
    output.mkdir(parents=True, exist_ok=True)
    (output / "examples.tex").write_text("\n".join(report))
    numbered_rules = "\n".join(
        f"(* R{i:02d} *) {line}" if line else ""
        for i, line in _number_lines(ebnf)
    )
    (output / "numbered.ebnf").write_text(numbered_rules + "\n")
    print(f"Verified {len(rules)} designated examples, {2 * len(rules)} numbered trees, "
          "2 complete programs, 7 rejection cases, and 2 loop classification checks.")


def _number_lines(ebnf):
    index = 0
    for line in ebnf.splitlines():
        if line:
            index += 1
        yield index, line


if __name__ == "__main__":
    main()

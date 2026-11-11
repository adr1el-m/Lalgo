"""Generate Chapter 3 and verify its derivations against the unified lexer."""

import json

from grammar import Ebnf, HERE, Node, ROOT, lex, recognize


def derivation(root, rightmost):
    """Write every sentential form, expanding one nonterminal at each step."""
    frontier = [root]
    lines = [root.name]
    while True:
        expandable = [i for i, node in enumerate(frontier) if isinstance(node, Node)]
        if not expandable:
            return "\n".join(lines), frontier
        index = expandable[-1] if rightmost else expandable[0]
        node = frontier[index]
        frontier[index:index + 1] = node.children
        symbols = [item.name if isinstance(item, Node) else item for item in frontier]
        lines.append(wrap_symbols(symbols or ["epsilon"], prefix="=> "))


def tree_symbols(node):
    """Yield the symbols of a single bracketed parse tree."""
    if not isinstance(node, Node):
        yield node
        return
    yield f"{node.name}["
    for child in node.children:
        yield from tree_symbols(child)
    yield "]"


def wrap_symbols(symbols, prefix="", width=100):
    """Wrap only between symbols, keeping names and quoted lexemes intact."""
    lines = []
    line = prefix
    for symbol in symbols:
        separator = "" if not line.strip() or line.endswith(" ") else " "
        if len(line) + len(separator) + len(symbol) <= width:
            line += separator + symbol
        else:
            lines.append(line)
            line = "   " + symbol
    if line:
        lines.append(line)
    return "\n".join(lines)


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
        root = matches[0]
        left, left_leaves = derivation(root, False)
        right, right_leaves = derivation(root, True)
        if left_leaves != right_leaves:
            raise ValueError(f"{name}: the derivations have different terminal yields")
        heading = f"\\textbf{{R{index:02d} \\quad \\texttt{{{name}}}}}\\par\n"
        sample = listing(source, "aboveskip=3pt,belowskip=4pt")
        views = []
        for label, view in [
            ("Leftmost derivation", left),
            ("Rightmost derivation", right),
            ("Parse tree", wrap_symbols(tree_symbols(root))),
        ]:
            views.append(
                "\\Needspace{4\\baselineskip}\n"
                f"{{\\small\\bfseries\\color{{pupmaroon}} {label}}}\\par\n"
                + listing(view, "language={},basicstyle=\\scriptsize\\ttfamily,framesep=3pt,"
                          "breaklines=false,frame=none,"
                          "xleftmargin=0pt,xrightmargin=0pt,aboveskip=2pt,belowskip=4pt")
            )
        report.append(
            "\\par\\medskip\\Needspace{9\\baselineskip}\n"
            + heading + sample + "\n".join(views)
        )

    programs = ["binary_search.lal", "checking_sample.lal", "switch_sample.lal"]
    for filename in programs:
        path = HERE / filename
        source = path.read_text()
        if len(recognize(rules, lex(source, with_eof=True), "Program")) != 1:
            raise ValueError(f"{path.name} must have exactly one complete program tree")

    invalid_programs = [
        "FOR (TRUE) {}",
        "OUTPUT x",
        "OUTPUT \"text\", 42;",
        "INTEGER x, y = 1;",
        "IF (a) {} ELSE IF (b) {}",
        "WHILE (a) { INPUT x; ",
        "TRACE x;",
        "SWITCH (x) { DEFAULT: BREAK; }",
        "SWITCH (x) { CASE 1: BREAK; }",
        "SWITCH (x) { CASE 1: OUTPUT x; DEFAULT: BREAK; }",
        "SWITCH (x) { CASE 1 BREAK; DEFAULT: BREAK; }",
        "SWITCH (x) { CASE label: BREAK; DEFAULT: BREAK; }",
        "SWITCH (x) { CASE 1: BREAK; DEFAULT: BREAK; DEFAULT: BREAK; }",
        "SWITCH (x) { CASE 1: BREAK; DEFAULT: BREAK; CASE 2: BREAK; }",
        "BREAK;",
    ]
    for source in invalid_programs:
        if recognize(rules, lex(source, with_eof=True), "Program"):
            raise ValueError(f"Invalid program accepted: {source!r}")

    # A loop nested under an IF still counts, whereas the word inside a string does not.
    for source, form in [
        ('WHILE (active) { OUTPUT "WHILE"; }', "PlainWhile"),
        ("WHILE (active) { IF (ready) { WHILE (again) {} } }", "NestedWhile"),
        ("WHILE (active) { SWITCH (x) { CASE 1: WHILE (again) {} BREAK; DEFAULT: BREAK; } }",
         "NestedWhile"),
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
    print(f"Verified {len(rules)} designated examples, {len(rules)} parse trees, "
          f"{2 * len(rules)} complete derivations, "
          f"{len(programs)} complete programs, {len(invalid_programs)} rejection cases, "
          "and 3 loop classification checks.")


def _number_lines(ebnf):
    index = 0
    for line in ebnf.splitlines():
        if line:
            index += 1
        yield index, line


if __name__ == "__main__":
    main()

"""Run the public syntax checker on valid and malformed switch programs."""

from pathlib import Path
import subprocess
import sys
import tempfile

ROOT = Path(__file__).resolve().parents[1]
CHECKER = ROOT / "syntax_proposal/check_syntax.py"

CASES = [
    ("checking sample", (ROOT / "syntax_proposal/checking_sample.lal").read_text(), None),
    ("switch sample", (ROOT / "syntax_proposal/switch_sample.lal").read_text(), None),
    ("nested switches",
     "SWITCH (outer) { CASE 1: SWITCH (inner) { CASE 'A': BREAK; DEFAULT: BREAK; } "
     "BREAK; DEFAULT: BREAK; }", None),
    ("full string and tab in a case label",
     'SWITCH (text) { CASE "a long literal with a tab\tand spaces": BREAK; DEFAULT: BREAK; }', None),
    ("signed integer and Boolean labels",
     "SWITCH (x) { CASE -1: BREAK; DEFAULT: BREAK; }\n"
     "SWITCH (flag) { CASE TRUE: BREAK; CASE FALSE: BREAK; DEFAULT: BREAK; }", None),
    ("indexed selector", "SWITCH (items[i]) { CASE 1: BREAK; DEFAULT: BREAK; }", None),
    ("missing case colon", "SWITCH (x) {\n CASE 1 BREAK;\n DEFAULT: BREAK;\n}",
     "line 2, column 9: found 'BREAK'. Expected ':'."),
    ("missing break", "SWITCH (x) { CASE 1: OUTPUT x; DEFAULT: BREAK; }", "found 'DEFAULT'"),
    ("missing default", "SWITCH (x) { CASE 1: BREAK; }", "found '}'"),
    ("duplicate default", "SWITCH (x) { CASE 1: BREAK; DEFAULT: BREAK; DEFAULT: BREAK; }",
     "found 'DEFAULT'"),
    ("decimal label", "SWITCH (x) { CASE 1.5: BREAK; DEFAULT: BREAK; }", "found '1.5'"),
    ("break outside a switch", "BREAK;", "found 'BREAK'"),
    ("lexer errors before parsing", "SWITCH (x) { CASE 1: @ BREAK; DEFAULT: BREAK; }",
     "Lexical error on line 1"),
]


def main():
    for label, source, error in CASES:
        with tempfile.TemporaryDirectory(prefix="lalgo-syntax-test-") as directory:
            path = Path(directory) / "program.lal"
            path.write_text(source, encoding="ascii")
            result = subprocess.run([sys.executable, str(CHECKER), str(path)], cwd=directory,
                                    capture_output=True, text=True)
            if error is None:
                assert result.returncode == 0 and "Syntax OK:" in result.stdout, (label, result)
                listing = (Path(directory) / "Symbol Table.txt").read_text()
                assert "KW_SWITCH" in listing and "KW_BREAK" in listing, label
            else:
                assert result.returncode == 1 and error in result.stderr, (label, result)
            print(f"PASS syntax: {label}")
    print(f"{len(CASES)}/{len(CASES)} syntax CLI cases passed")


if __name__ == "__main__":
    main()

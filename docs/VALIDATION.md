# Local lexer validation

Snapshot: **explicit C keyword-initial dispatch revision**. Target: `lexical_analyzer/lalgo_unified.c`.

## Result

- **39/39 black-box cases passed.**
- Compiled on Windows with `gcc -std=c99 -Wall -Wextra -Wpedantic -Werror`.
- Trace output confirms **288 explicit states and 83 input classes**.
- The supplied `sample.lal` produces **99 listing entries**, plus EOF on screen, with **0 lexical errors**.
- The supplied `bad.lal` produces **28 listing entries**, including **7 invalid entries**, and reports **7 lexical errors** (expected exit code 1).
- The paper's worked token stream is checked exactly, including positions and metadata.

Source SHA-256:

```text
f84d06926048dd5dd22e0feb4d08bef3a452dd9d191626622ed13d70b4bc61eb
```

## Reproduce

```sh
make test
./publish.sh --build-only
```

The tests invoke the executable in a temporary working directory per case, preventing its `Symbol Table.txt` output from colliding across cases. The runner checks exit codes, diagnostics, EOF, and listing behavior, adding token-sequence and metadata assertions where relevant. The generated JSON report is written to `build/tests/results.json`; the PDF's measured case counts are read from `build/tests/summary.tex` during compilation.

## Cases

| # | Contract case | Result |
| --- | --- | --- |
| 1 | all 28 reserved spellings | PASS |
| 2 | case diagnostics and identifier suffixes | PASS |
| 3 | switch keyword boundaries and capitalization | PASS |
| 4 | switch keyword classes and colon | PASS |
| 5 | all keyword initials lowercase | PASS |
| 6 | all keyword initials mixed case | PASS |
| 7 | I branch whole words and boundaries | PASS |
| 8 | I branch capitalization and incomplete prefixes | PASS |
| 9 | I branch exact EOF boundary | PASS |
| 10 | I branch trace follows shared DFA | PASS |
| 11 | identifier alphabet | PASS |
| 12 | invalid identifier start | PASS |
| 13 | integer range and leading zeros | PASS |
| 14 | decimal and dot backtracking | PASS |
| 15 | number identifier adjacency | PASS |
| 16 | separate unary sign and exponent | PASS |
| 17 | symbolic operators and punctuation | PASS |
| 18 | lone exclamation recovery | PASS |
| 19 | strings empty and tab | PASS |
| 20 | one character and tab | PASS |
| 21 | bad character lengths | PASS |
| 22 | backslash rejected in string | PASS |
| 23 | unterminated string line recovery | PASS |
| 24 | comments retained | PASS |
| 25 | unterminated block commits past slash | PASS |
| 26 | block comments do not nest | PASS |
| 27 | newline preserved across auxiliary comment | PASS |
| 28 | CRLF normalization and positions | PASS |
| 29 | isolated carriage return invalid | PASS |
| 30 | non ASCII comment diagnostic | PASS |
| 31 | empty source EOF | PASS |
| 32 | wrong extension rejected | PASS |
| 33 | uppercase extension accepted | PASS |
| 34 | NUL input rejected | PASS |
| 35 | DFA trace dimensions | PASS |
| 36 | paper worked example exact stream | PASS |
| 37 | binary search sample | PASS |
| 38 | supplementary demo | PASS |
| 39 | negative example seven errors | PASS |

## Limits

These are local contract checks of the selected implementation. They are not an exhaustive automaton proof, performance benchmark, parser or runtime validation, instructor approval, or external acceptance. Very large files and resource-failure handling are outside this suite. The historical `lalgo_lexer.c` is retained but is not covered by these results.

The build regenerates the proposal’s DFA appendix and Chapter 3 examples. The separate syntax CLI suite covers 13 valid and invalid inputs, including nested switches, keyword placement, and source-position diagnostics. See the [switch checking notes](SWITCH-CHECKING.md) for the demonstration commands.


The six additional checks cover all 28 spellings in lowercase and mixed case,
the shared I branch, delimiter and EOF boundaries, incomplete prefixes and
suffixes, capitalization flags, and the actual `I -> IF` trace. DFA export also
asserts that runtime dispatch matches all 52 letter edges from `q0`.
`make syntax` accepts `syntax_proposal/switch_sample.lal`. The Chapter 3 build
checks 45 examples, 90 derivations, 45 trees, three complete programs,
15 rejections, and three loop classifications.

Native Windows verification uses GNU Make with Git Bash, `CC=gcc`, and Python 3.
The existing signed-long range check was corrected to reject overflow before
multiplication, and the syntax CLI resolves the Windows `.exe` suffix.

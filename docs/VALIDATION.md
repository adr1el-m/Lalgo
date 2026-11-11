# Local lexer validation

Snapshot: **switch and case revision**. Target: `lexical_analyzer/lalgo_unified.c`.

## Result

- **33/33 black-box cases passed.**
- Compiled with `cc -std=c99 -Wall -Wextra -Wpedantic -Werror`.
- Trace output confirms **288 explicit states and 83 input classes**.
- The supplied `sample.lal` produces **99 listing entries**, plus EOF on screen, with **0 lexical errors**.
- The supplied `bad.lal` produces **28 listing entries**, including **7 invalid entries**, and reports **7 lexical errors** (expected exit code 1).
- The paper's worked token stream is checked exactly, including positions and metadata.

Source SHA-256:

```text
95d4c8066a8fd8d4f1519faa749746710a056242f3b3811ca6ac6afaa66c98f4
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
| 5 | identifier alphabet | PASS |
| 6 | invalid identifier start | PASS |
| 7 | integer range and leading zeros | PASS |
| 8 | decimal and dot backtracking | PASS |
| 9 | number identifier adjacency | PASS |
| 10 | separate unary sign and exponent | PASS |
| 11 | symbolic operators and punctuation | PASS |
| 12 | lone exclamation recovery | PASS |
| 13 | strings empty and tab | PASS |
| 14 | one character and tab | PASS |
| 15 | bad character lengths | PASS |
| 16 | backslash rejected in string | PASS |
| 17 | unterminated string line recovery | PASS |
| 18 | comments retained | PASS |
| 19 | unterminated block commits past slash | PASS |
| 20 | block comments do not nest | PASS |
| 21 | newline preserved across auxiliary comment | PASS |
| 22 | CRLF normalization and positions | PASS |
| 23 | isolated carriage return invalid | PASS |
| 24 | non ASCII comment diagnostic | PASS |
| 25 | empty source EOF | PASS |
| 26 | wrong extension rejected | PASS |
| 27 | uppercase extension accepted | PASS |
| 28 | NUL input rejected | PASS |
| 29 | DFA trace dimensions | PASS |
| 30 | paper worked example exact stream | PASS |
| 31 | binary search sample | PASS |
| 32 | supplementary demo | PASS |
| 33 | negative example seven errors | PASS |

## Limits

These are local contract checks of the selected implementation. They are not an exhaustive automaton proof, performance benchmark, parser or runtime validation, instructor approval, or external acceptance. Very large files and resource-failure handling are outside this suite. The historical `lalgo_lexer.c` is retained but is not covered by these results.

The build regenerates the proposal’s DFA appendix and Chapter 3 examples. The separate syntax CLI suite covers 13 valid and invalid inputs, including nested switches, keyword placement, and source-position diagnostics. See the [switch checking notes](SWITCH-CHECKING.md) for the demonstration commands.

# Local lexer validation

Snapshot: **30 October 2026 (Asia/Manila)**. Target: `lexical_analyzer/lalgo_unified.c`.

## Result

- **31/31 black-box cases passed.**
- Compiled with `cc -std=c99 -Wall -Wextra -Wpedantic -Werror`.
- Trace output confirms **254 explicit states and 83 input classes**.
- The supplied `sample.lal` produces **99 listing entries**, plus EOF on screen, with **0 lexical errors**.
- The supplied `bad.lal` produces **28 listing entries**, including **7 invalid entries**, and reports **7 lexical errors** (expected exit code 1).
- The paper's worked token stream is checked exactly, including positions and metadata.

Source SHA-256:

```text
e41df98e6577a0c187be11ab3ff50c491ca60a7e2e3458194c309884b0b558fa
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
| 1 | all 24 reserved spellings | PASS |
| 2 | case diagnostics and identifier suffixes | PASS |
| 3 | identifier alphabet | PASS |
| 4 | invalid identifier start | PASS |
| 5 | integer range and leading zeros | PASS |
| 6 | decimal and dot backtracking | PASS |
| 7 | number identifier adjacency | PASS |
| 8 | separate unary sign and exponent | PASS |
| 9 | symbolic operators and punctuation | PASS |
| 10 | lone exclamation recovery | PASS |
| 11 | strings empty and tab | PASS |
| 12 | one character and tab | PASS |
| 13 | bad character lengths | PASS |
| 14 | backslash rejected in string | PASS |
| 15 | unterminated string line recovery | PASS |
| 16 | comments retained | PASS |
| 17 | unterminated block commits past slash | PASS |
| 18 | block comments do not nest | PASS |
| 19 | newline preserved across auxiliary comment | PASS |
| 20 | CRLF normalization and positions | PASS |
| 21 | isolated carriage return invalid | PASS |
| 22 | non ASCII comment diagnostic | PASS |
| 23 | empty source EOF | PASS |
| 24 | wrong extension rejected | PASS |
| 25 | uppercase extension accepted | PASS |
| 26 | NUL input rejected | PASS |
| 27 | DFA trace dimensions | PASS |
| 28 | paper worked example exact stream | PASS |
| 29 | binary search sample | PASS |
| 30 | supplementary demo | PASS |
| 31 | negative example seven errors | PASS |

## Limits

These are local contract checks of the selected implementation. They are not an exhaustive automaton proof, performance benchmark, parser or runtime validation, instructor approval, or external acceptance. Very large files and resource-failure handling are outside this suite. The historical `lalgo_lexer.c` is retained but is not covered by these results.

Both PDFs compile successfully through the build-only script. The new lexer paper is visually reviewed after rendering. The companion specification retains its existing contents and regenerates its implementation-derived DFA appendix.

# Lalgo

**LAlgo** is a programming-language project for Principles of Programming Languages (COSC 303), BSCS 3-1, Group 4, at the Polytechnic University of the Philippines. Its current working component is a **C99 lexical analyzer built around one unified DFA**.

The repository brings together the original proposal, evolving language specification, selected lexer, examples, validation suite, and a separate implementation paper. The timeline below records the documented milestones leading to the current snapshot.

## What we have now

| Deliverable | Current state |
| --- | --- |
| [Lexer paper](LAlgo_Lexer_Paper.pdf) | Design, implementation, worked token stream, recovery rules, local validation, and limitations; [editable LaTeX](lalgo_lexer_paper.tex). |
| [Language specification](LAlgo_Language_Design_Specification.pdf) | Lexical rules, examples, diagrams, and a complete generated DFA transition appendix; [editable LaTeX](lalgo_language_specification.tex). |
| [Unified lexer](lexical_analyzer/lalgo_unified.c) | Selected implementation: one start state, 254 explicit states, 83 input classes, integrated keyword paths, trace mode, and token listing. |
| [Earlier lexer](lexical_analyzer/lalgo_lexer.c) | Retained for historical comparison; the default build and validation target the unified implementation. |
| [Examples](lexical_analyzer/) | `sample.lal` (binary search), `demo.lal` (additional forms), and `bad.lal` (intentional lexical errors). |
| [Validation](docs/VALIDATION.md) | 31 local black-box cases; reproducible with `make test`. |

The project currently covers **lexical analysis**. Parsing, semantic analysis, statement-boundary rules, and runtime execution remain future work. Reserved words and illustrative programs describe language forms; they do not establish an executable compiler or interpreter.

## Project timeline

### Original proposal — retained starting point

The [Group 4 proposal](%5BGROUP-4%5D%20PRINCIPLES%20OF%20PROGRAMMING%20LANGUAGE%20PROPOSAL.pdf) records the initial language concept and intended algorithm-oriented direction. It is preserved as a reference artifact; later lexical decisions are recorded in the specification and changelog.

### 25 September 2026 — Language and lexical foundations

- Developed the language specification, character classes, identifiers, literals, operators, delimiters, and explanatory DFA diagrams.
- Added exponentiation (`^`) and clarified relational lookahead, ASCII character literals, and CRLF handling.
- Settled on uppercase reserved spellings: 18 current spellings, four future-reserved words, and two noise words (`SET`, `THE`).
- Narrowed implemented scope to lexical recognition, with parser and runtime behavior deferred.
- Refined the binary-search illustration, authorship, course details, and Group 4 document layout.

### 27 October 2026 — Lexical review and reporting contract

- Applied the [lexical review fixes](LAlgo-Lexical-Spec-Fixes-and-Summary.pdf).
- Defined `suspectedKeyword` for lowercase or mixed-case reserved spellings and `precededByNewline` metadata.
- Clarified malformed number/name adjacency, integer limits, decimal-dot fallback, token positions, and error-recovery boundaries.
- Added a worked binary-search token stream to the specification.

### 28 October 2026 — One unified lexical automaton

- Selected [`lalgo_unified.c`](lexical_analyzer/lalgo_unified.c): every token family branches from the same `q0` and uses `DELTA[state][class]`.
- Integrated keyword-prefix paths into the automaton; scanning does not compare completed lexemes against a keyword list.
- Added the complete, implementation-generated appendix for 254 explicit states and 83 classes.
- Aligned comments, invalid-token output, newline metadata, optional DFA tracing, and the `Symbol Table.txt` contract.

### 30 October 2026 — Lexer paper and GitHub baseline

- Created the separate [lexer implementation paper](LAlgo_Lexer_Paper.pdf), grounded in the selected C source.
- Added a strict C99 build and 31 contract cases covering vocabulary, token boundaries, literals, comments, recovery, source positions, file handling, trace dimensions, and supplied examples.
- Verified the paper's worked token stream against the executable.
- Added reproducible build commands and documented the measured local results.
- Prepared this repository baseline with the source, papers, historical reference PDFs, examples, and project timeline. Temporary builds and unrelated website deployment copies are excluded.

Earlier milestones summarize the existing [detailed changelog](CHANGELOG.md). They describe the project's recorded development, rather than separate commits reconstructed after the fact.

## Build and run

Requirements for the lexer: a C99 compiler, Make, and Python 3 for validation. No external Python packages are needed for the tests.

```sh
make test
./build/lalgo lexical_analyzer/sample.lal
./build/lalgo lexical_analyzer/demo.lal t
```

`make test` compiles the selected source with `-std=c99 -Wall -Wextra -Wpedantic -Werror` and runs the checks. `make lexer` builds just the executable.

Each completed scan prints tokens and diagnostics, and overwrites **`Symbol Table.txt` in the current working directory**. It lists token occurrences, including comments and invalid lexemes, with classifications and positions. It is a lexical listing; declaration/scope resolution is future work. EOF appears in console output and is omitted from the listing file.

Exit codes: `0` = no lexical errors, `1` = lexical errors reported, `2` = usage/input/output failure. The optional `t` argument enables state-transition tracing. Input filenames must end in `.lal` (case-insensitive).

### Build both papers

Install a LaTeX distribution with `latexmk`, `pdflatex`, and the packages used in the `.tex` sources, then run:

```sh
./publish.sh --build-only
```

This runs validation, regenerates the specification's full DFA appendix, and builds both PDFs. Intermediates stay under `build/`. Generated validation details are written to `build/tests/results.json` and tied to the C source by SHA-256.

The optional website-publishing path in `publish.sh` uses the author's adjacent `../papers-site` workspace. A standalone GitHub clone supports `--build-only`; website publication requires that separate project. This repository baseline does not publish the new paper to the website.

## Repository map

```text
Lalgo/
├── README.md
├── CHANGELOG.md
├── Makefile
├── publish.sh
├── lalgo_lexer_paper.tex
├── LAlgo_Lexer_Paper.pdf
├── lalgo_language_specification.tex
├── LAlgo_Language_Design_Specification.pdf
├── LAlgo-Lexical-Spec-Fixes-and-Summary.pdf
├── [GROUP-4] PRINCIPLES OF PROGRAMMING LANGUAGE PROPOSAL.pdf
├── img/PUPLogo.png
├── lexical_analyzer/
│   ├── lalgo_unified.c
│   ├── lalgo_lexer.c
│   ├── export_unified_dfa.py
│   ├── sample.lal
│   ├── demo.lal
│   └── bad.lal
├── tests/test_lexer.py
├── docs/VALIDATION.md
└── build/                       # generated locally; ignored by Git
```

## Next milestones

- Define a grammar and parser contract, including statement boundaries and noise-word handling.
- Add semantic rules, scope/type checking, and expression evaluation.
- Specify runtime behavior for algorithm execution and the future-reserved vocabulary.
- Expand validation beyond the current local contract suite.

## Group 4

Buenaventura, Harvey L.; De Mesa, Joshua C.; Dela Rosa, Mico Von Adriel A.; Magalona, Adriel M.; Morales, Timothy Gabriel; Nevado, Sean Andrew M.; Salvacion, Dean James G.

Submitted to **Prof. Ria A. Sagum**, Principles of Programming Languages (COSC 303).

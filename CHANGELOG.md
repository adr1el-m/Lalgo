# LAlgo specification change log

Changes requested from September 25, 2026 onward are recorded here. Each entry describes what was added or altered in the editable LaTeX source and its built PDF.

## 2026-10-30 — Lexer implementation paper and GitHub baseline

- Added a separate implementation-focused lexer paper in editable LaTeX and PDF, covering the selected unified DFA, token contracts, committed recovery, a worked token stream, validation, and limitations.
- Added a Makefile and 31 black-box contract cases for the unified lexer; recorded measured results and source identity in `docs/VALIDATION.md`.
- Extended `publish.sh --build-only` to validate the lexer and build both papers, with generated material under `build/`.
- Added the repository README as a timeline of the documented project milestones and current deliverables.
- Prepared the Lalgo GitHub repository baseline, excluding temporary build outputs and unrelated website working copies.

## 2026-10-28 — Unified lexical automaton

- Aligned the paper with the selected `lexical_analyzer/lalgo_unified.c` implementation: one start state, one transition table, and integrated keyword recognition.
- Added a complete, build-generated specification of all 254 explicit states and 83 input classes, including accepting actions and committed-state metadata.
- Retained category diagrams as explanatory projections and included whitespace in the unified overview.
- Updated comment and INVALID output, newline metadata, the worked token stream, and the `Symbol Table.txt` input/output contract.
- Documented optional DFA tracing, lexical-only scope, and deferred parser/runtime behavior.

## 2026-10-27 — Lexical-spec review fixes

- Applied all eight fixes from `LAlgo-Lexical-Spec-Fixes-and-Summary.pdf`, adopting uppercase-only keyword recognition with `suspectedKeyword` metadata and newline tracking with `precededByNewline`.
- Made number/identifier adjacency a lexical error; documented dot backtracking, leading zeros, integer limits, and double-precision decimal values.
- Clarified character classes, comment contents, tabs and quotes in literals, implicit DFA error states, and the master dispatch procedure.
- Added Section 2.3.12 with the complete token vocabulary, source positions, diagnostic metadata, and recovery boundaries.
- Moved NOT below relational comparisons, retained exponentiation above unary minus, and clarified the intended parser scope.
- Added the existing binary-search example's complete 99-token stream, including EOF, source positions, and newline flags.
- Built and visually reviewed the revised 21-page PDF before publication.

## 2026-09-25 — Exponentiation as the seventh arithmetic operation

- Added `^` to the arithmetic character set and the arithmetic-operator table.
- Updated the arithmetic count from six to seven and added `2 ^ 3` as an example.
- Added exponentiation to the precedence table above unary negation and multiplication, with right-to-left associativity.
- Kept the division example heading with its code block and shortened three arithmetic table descriptions to avoid awkward page and word breaks.
- Rebuilt `LAlgo_Language_Design_Specification.pdf` from `lalgo_language_specification.tex`.
- The added content increased the A4 PDF from 15 to 16 pages.

## 2026-09-25 — Relational characters and master lexer dispatch

- Listed only the single characters `=`, `!`, `<`, and `>` in the relational/assignment character set. Explained that `==`, `!=`, `<=`, and `>=` are compound operators recognized by lookahead, and that `!` alone is invalid.
- Separated underscore and quote characters from delimiters in the character-set partition so each class has a distinct role.
- Redrew the master lexer dispatch graph to show routes into identifiers, numbers, relational operators, strings, comments/division, delimiters, arithmetic symbols, and character literals. Kept whitespace as a self-loop at the start state.
- Kept the scan-trace introduction with its table after the expanded diagram and moved the string-literal DFA start to the next page to avoid awkward page breaks.
- Kept the relational-operator DFA heading and diagram together after the preceding lexer examples.

## 2026-09-25 — Consistency and character-literal recognition

- Removed `TO` from the reserved noise-word list and syntax highlighting. Changed the assignment example to `SET index = 0`, where discarding `SET` leaves the valid assignment `index = 0`. Clarified that the parser accepts noise words only in the shown positions and omits them from the AST.
- Made `CHARACTER` consistently ASCII-only and added a four-state DFA for one printable ASCII character between single quotes. Specified that quote and backslash escapes are not defined and that empty, multi-character, and unterminated literals are lexical errors.
- Added a literal-only ASCII symbol class so the stated alphabet covers printable characters that can appear inside quoted literals; aligned the string-literal DFA with the same printable-ASCII and no-escape policy.
- Standardized whitespace on space (ASCII 32), tab (ASCII 9), and line feed (ASCII 10). Clarified that CRLF is normalized to LF before scanning and an isolated carriage return is invalid.
- Clarified the count of 22 language keywords and two separate reserved noise words (`SET`, `THE`). Updated the shared-prefix note so it no longer mentions `TO` or implies the wrong remaining-keyword count.
- Kept the delimiter-DFA heading, explanation, and diagram on the same page after the new character-literal section.
- Adjusted page breaks so the comments/division, string, and keyword DFA diagrams stay with their subsections, the keyword count follows its table, and the conditional and loop examples are not split across pages.
- Rebuilt and visually checked the revised PDF; it now has 17 A4 pages, with no LaTeX warnings or overfull boxes.

## 2026-09-25 — Site publication

- Updated the project publish script's author metadata to credit all seven named student authors, matching the PDF cover.
- Published the 17-page PDF under the public slug `lalgo-language-design-specification` in the papers site (site commit `0caf01f`). The live detail and reader routes passed the site smoke test, and the live PDF's SHA-256 matched the local output.

## 2026-09-25 — Review corrections in progress

- Corrected the final binary-search `WHILE` example so it moves `low` right only when the midpoint value is below the target and otherwise moves `high` left.
- Removed the unsupported comparison-count claim from the introduction and replaced the operation-cost problem statement with a source-level state-visibility concern.
- Removed the undefined `method calls` expression claim and described the shown indexed values instead.
- Renamed the keyword diagram as a representative DFA to match its pedagogical role and the stated table-lookup implementation.
- Removed unsupported commitments to short-circuit evaluation, default values for uninitialized variables, and input casting; these remain outside the lexical proposal. Moved the binary-search boundary comment before the comparison it describes.
- Rebuilt and visually checked the 17-page local PDF with no LaTeX warnings or overfull boxes. At this stage, the corrections remained local while the language-feature scope choice was pending.

## 2026-09-25 — Scope decision: lexical proposal with future-reserved words

- Kept `ALGORITHM`, `RETURN`, `TRACE`, and `INVARIANT` as reserved lexemes only. Reclassified the table as 18 current language keywords, four future-reserved words, and two separate noise words; their parser and runtime behavior is explicitly unspecified.
- Reframed the introduction and objectives around token recognition, with tracing and verification described only as future directions rather than implemented features.
- Retained the representative keyword DFA, noting that `INVARIANT` is recognized as a future-reserved word and actual lookup uses the reserved-word table.
- Removed executable uses of `RETURN`, `TRACE`, and `INVARIANT`. The conditional and binary-search loop examples now use a `found` flag and update both search boundaries correctly.
- Scoped operator descriptions, expression examples, I/O, assignment, and free-field formatting as illustrative lexical forms. Statement-boundary rules remain future parser work.
- Updated the site's publish metadata abstract to reflect the narrower lexical-analyzer scope.
- Renamed the main content section to "Lexical Elements," clarified that brace scoping is parser work, and adjusted page breaks around the objectives and final examples.
- Anchored the master lexer figure and scan-trace table in source order after a pagination check found them floating past the identifier DFA.
- Made the illustrative binary search self-contained with a sorted array, target, bounds, midpoint, and `found` flag; clarified that line-feed equivalence excludes comments and quoted literals.
- Rebuilt and visually checked the final 17-page A4 PDF after the scope decision; the LaTeX log has no warnings, undefined references, or overfull boxes.
- Added the published LAlgo slug and source to the workspace README catalog.
- Republished the revised PDF and narrower abstract to the public papers site in commit `00c03d1`. The site smoke test passed, the prior PDF was retained as one archived version, and the live PDF matched the local build byte-for-byte (SHA-256 `0afd00b5f9e866b59f82fce6e9ca994dfe98d2a02ce01d842f97125cebc931c9`).

## 2026-09-25 — Cover details and website reading experience

- Replaced the generic "Submitted to" line on the cover with Prof. Ria A. Sagum.
- Added Principles of Programming Languages (COSC 303) as the course and subject code on the cover.
- Tightened only the cover spacing, rebuilt the PDF, and verified that the new details fit on page one while the document remains 17 A4 pages.
- Corrected the website PDF viewer so pages without side notes are centered across the full reading area.
- Added an optional structured change-log timeline to the paper's website details and a change-log link in the reader tools menu.

## 2026-09-25 — Group 4 identity and running page furniture

- Added Group 4 to the cover's "Submitted by" heading without changing the supplied student names.
- Added a running header on the pages after the cover: "LAlgo - Language Design Specification" on the left and "BSCS 3-1 | Group 4" on the right, separated from the content by a thin maroon rule.
- Removed "Formal Syntax and Operational Lexicon" from the footer and raised the page number by reducing the footer distance from 18 mm to 11 mm.
- Reduced the running header height from 36 pt to the valid 22 pt minimum and shifted the maroon rule upward by 8 pt to bring it closer to the header text; kept this refinement local without republishing.

# 12 November 2026 proposal checklist

Use the [revised proposal PDF](../LAlgo_Language_Design_Specification.pdf) for the print and soft-copy versions. The representative announced an individual activity or quiz on the same day. Coverage was unspecified. The representative said a Google Drive folder would be created for soft copies. A printed copy is also required.

| Requirement | Where to check |
| --- | --- |
| Updated Chapters 1 and 2 | Chapter 1 describes the current scope and goals. Chapter 2 aligns terminology, vocabulary, punctuation, examples, and precedence with the grammar. The known lexical corrections in the existing fix sheet are reflected in the specification, with the current lexer as the authority for emitted comments and invalid tokens. |
| Chapter 3 EBNF proposal | Numbered rules R01–R41, with the start symbol, terminal mapping, statement boundaries, and syntax notation defined. |
| One example derived from each rule | Section 3.7 has exactly one designated example for each of the 41 rules, with separate leftmost and rightmost derivation sequences and one bracketed parse tree. |
| One parsing approach | Chapter 3 specifies top-down recursive descent and an in-memory cursor that consumes one terminal at a time. |
| Input, output, assignment, conditions, iteration, declarations | The coverage table maps the required 1 input, 3 output, 3 plain-assignment, 3 conditional, 2 loop, and 3 declaration forms to the designated rule examples. Both loop forms use `WHILE`. |
| Recovery and messages | Chapter 3 specifies panic recovery, continuation, and a concise line/column diagnostic with a reason. |
| Optional parser artifacts | The checking guide and recording describe the transition diagram and parsing table as optional for this approach. The existing DFA appendix documents the lexer. |

Design choices made in this revision are `WHILE` as the only grammar loop kind, semicolons on simple statements, braces on control bodies, and comma-separated mixed output and declaration lists. The C lexer retains its existing legacy `FOR` token. The grammar rejects it as a statement.

The build checks 31 existing lexer cases, 41 designated rule examples, 82 complete derivations, 41 parse trees, two full programs, seven syntax rejections, and two loop-classification cases. This is verification of the proposal and the lexer. The integrated C syntax analyzer is scheduled for later checking.

Compare any individual instructor markup that was circulated outside the supplied announcement, recordings, and repository against the revised Chapters 1 and 2. Use the representative's Drive folder when its link is shared.

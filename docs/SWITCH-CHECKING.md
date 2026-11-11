# Switch and case checking

The team reported 70% at its last check and identified switch/case as the professor's required missing construct. The reference package from the group reported to have scored 100% contains Nautilus source, a lexical listing, test inputs, and a grammar. The package does not contain a marked rubric or a score breakdown.

| Area | Nautilus reference package | LAlgo checking revision |
| --- | --- | --- |
| Multiway selection | `match (identifier)`, one or more `is value:` arms, a final `default:`, and `stop;` in each arm | `SWITCH (reference)`, one or more `CASE literal:` arms, a final `DEFAULT:`, and `BREAK;` in each arm |
| Keyword case | Lowercase spellings, such as `match` and `is` | Uppercase spellings through the existing unified DFA. Mixed or lowercase spellings produce identifier tokens with a case hint. |
| Source and comments | Lowercase `.naut` extension, `##` and block comments | Case-insensitive `.lal` extension, `//` and block comments |
| Symbols and categories | `Symbol_Table.txt` records token kinds, categories, lexemes, and positions. Its type and modifier words are classified as reserved words. | `Symbol Table.txt` records occurrence, token, classification, position, and lexeme. The four switch spellings are control-flow keywords and `:` is a delimiter. |
| Iteration | Grammar lists repeat, while, and do-while | The proposal uses single and nested `WHILE` forms, per LAlgo's stated scope. |
| Executable stages | The supplied C sources perform lexical analysis. The switch structure appears in the BNF. | The C lexer emits switch tokens. The Python checker uses those terminals and the proposal EBNF to recognize full programs. |

The comparison uses the reference package's README, Makefile, grammar, lexer/token/symbol-table sources, and sample and test inputs. LAlgo's implementation uses its own lexer and grammar.

## Demonstrate from the repository root

```sh
make test
./build/lalgo syntax_proposal/switch_sample.lal
make syntax
make syntax SOURCE=syntax_proposal/checking_sample.lal
```

Show the four keyword tokens, the case colons, the lexical symbol table, and a successful syntax check. To demonstrate diagnostics, remove a case colon in a scratch copy and run the checker on that copy. Rules R42–R45 provide the switch examples, their complete leftmost and rightmost derivations, and their parse trees.

The checker covers literal label syntax, nested switches, arm termination, and default placement. Type matching, duplicate case labels, execution, and panic recovery are future stages. Local results are 33 lexer checks and 13 syntax CLI checks. The proposal build also checks 45 designated examples, 90 derivations, 45 trees, three complete programs, 15 rejections, and three loop classifications.

# Meeting 01 — Introduction, deadlines, and syntax-analyzer requirements

**Meeting date:** 5 November 2026 (Asia/Manila)

**Recording:** `1.mp4` · approximately 5 minutes 57 seconds

**Sequence:** First recording / meeting introduction. Additional recordings should follow as separate numbered notes.

This document has two parts: a chronological transcript in the original Tagalog/English, followed by an English explanation and a comparison with LAlgo's current repository. Section 2.5 separately records a mid-meeting instruction supplied by Adriel after this introduction was transcribed. The explanations and suggested tasks are editorial notes, not additional instructions from Ma'am.

**Transcript status:** Machine-assisted, lightly edited for punctuation and technical spellings; checked against a second speech-recognition pass, not independently verified by human listening. This is a readable transcript, not a certified verbatim record. `[unclear]` identifies wording that could not be recovered confidently. Speaker labels are inferred from the exchange; timestamps are approximate. Pauses and repeated fillers are shortened.

## 1. Chronological transcript

### 00:00–00:32 — Faculty workload and accreditation

**Ma'am:** “Since this week is [unclear opening phrase about the week's work schedule]. Pero itong week na 'to, according to the schedule of CCIS, [unclear acronym/phrase], lahat kami halos [unclear phrase about the faculty], for the accreditation that will be happening sa college natin.

“So I have only this week and this week to do this. Under ako ng research eh, tsaka internationalization. So, medyo busy ang mga lola n'yo pagdating dito.”

### 00:32–00:55 — Continue individual work and the lexical analyzer

**Ma'am:** “So I suggest dito na lang, at kung ano man 'yung individual work na pagagawa ko sa inyo, pagagawa ko dito. At the same time, gawin n'yo na rin 'yung inyong lexical analyzer.

“As I have mentioned last time, dapat nung Tuesday, ginawa n'yo na 'yung ibang ginagawa for this one.”

### 00:55–01:40 — November 12 submission and documentation Part 3

**Ma'am:** “Ayan, alam ko na. For November 12, I would like for you to submit the syntax grammar rule, or the proposal for the syntax rule for your programming language.

“So 'yung inyong documentation, malalagyan ng Part 3. 'Di ba? Ang Part 1 ng documentation n'yo is Introduction. Part 2 is the syntactic elements. Tama? Part 3 will be the syntax rule of your programming language.

“You have to define the set of terminals, non-terminal symbols, the production and the start symbols. Also, you need to do your grammar rules. Okay?”

> Transcription note: Both main recognition passes render the Part 2 phrase as “syntactic elements.” Our existing specification calls its second section **Lexical Elements**. Preserve that distinction for now; this recording alone does not establish a required rename.

### 01:40–02:22 — Face-to-face default; derivations for every syntax rule

**Ma'am:** “So in every grammar rules, okay, so basta 'yan muna. Sa 12 'yan [unclear short phrase]. Pakitake note 'to ha, 12, 'yun ang isasubmit. At the same time, kung ano man 'yung gusto ko ipagawa sa inyo dito during this time.

“I'm not saying this is online. This can be face-to-face. Okay? By default, face-to-face tayo sa 12. So submission of the syntax rules with derivations for every syntax rule.

“Okay? With derivation ha. Ang derivation ay parse tree, leftmost and rightmost. Bahala kayo kung top-down or bottom-up 'yung gagamitin n'yo.”

### 02:22–03:18 — Remaining checking schedule and final exam

**Ma'am:** “19, async tayo. 26, checking of lexical analyzer 2 and syntax 1.

“For December, this week ay ano tayo? Vacant. I mean, asynchronous tayo. You do the project because on the 10th, [we/you — unclear pronoun] will be checking the syntax 2 and semantic 1, which is the final checking of your project and the final exam. Maliwanag po?

“Kasi ang departmental exam ng CCIS is scheduled on 15, 16, 17. So uunahan ko siya ng 10. Hanggang 22 lang ang first semester natin, so we don't have any chance. Kailangan dito natin ma-check na 'to. Kaya sabi ko kailangan ko na talagang iklase na rin 'yung dalawang topics. Na-gets ba? Malinaw?”

### 03:18–04:03 — Student repeats the dates for clarification

**Student:** “Ma'am, clarify ko lang po.”

**Ma'am:** “Yes, [student's name].”

**Student:** “If tama po ba 'yung nalagay ko. November 12 po is pagpasa po ng proposal for syntax grammar.”

**Ma'am:** “Oo, 'yung parang documentation n'yo.”

**Student:** “November 19 po is asynchronous. November 26 po is lexical checking 2 and syntax 1. Then December 3 is asynchronous po. Then December 10 po is syntax 2 and semantics 1. [Unclear short closing phrase.] Thank you po.”

**Ma'am:** “Kasi December 22, tapos na tayo ng first semester, according to our university calendar. Okay? Na-gets na? Okay na? Sige.”

### 04:03–04:22 — Fix the previously identified lexer issues

**Ma'am:** “Kung ano man 'yung sinasabi ko sa inyong dapat n'yong gawin doon sa lexical analyzer n'yo, the last time we checked, kailangan maayos na 'yun pag nag-check ako next meeting or next checking. Nagkakaintindihan ba tayo?”

### 04:22–04:54 — Syntax analyzer input and derivation output

**Ma'am:** “Ngayon, sa syntax analyzer, ano ang requirement ko? Well, the syntax analyzer, kailangan magre-read siya from the symbol table, one token at a time, to check for the construct or to check for the syntax.

“Ang magiging output n'yo would be a parse tree. ‘Ma'am, paano ang parse tree?’ Actually, hindi naman tree. Parang ano lang, may arrow lang na gano'n para makita na nade-derive n'yo. In every rule, nakaka-derive kayo. Sa bawat rule, may derivation na nangyayari.”

> Speaker note: The “Ma'am, paano…” phrase appears to be Ma'am anticipating a question; it is not confidently identifiable as a separate student's turn.

### 04:54–05:22 — Syntax errors, panic mode, and a later demonstration

**Ma'am:** “Pag tama sila, makikita ko lang 'yung parse tree. Pag may error siya, sasabihin ‘syntax error, [likely: at line …]’. At meron akong nakasulat doon na mode of… mode of… parang error generation. Merong panic mode kang gagawin.

“Okay? Papakita ko sa inyo mamaya kung paano natin naman i-check ang syntax analyzer after the discussion that we're going to have.”

> Transcription note: The exact error-message wording and the phrase before “panic mode” are uncertain. “Panic mode” is clear in both main passes. The promised demonstration is outside this introduction.

### 05:22–05:57 — One example per rule; leftmost and rightmost derivations

**Ma'am:** “Okay, hanggang doon lang tayo. Sa documentation naman, ganito. Kasi may required ako ng statements. For every statement, may required ako kung ano 'yun.

“So for every example of that rule—for every rule, magkakaroon kayo ng isang example lang. And that example should be derived using your grammar rule. And the derivation will be leftmost and rightmost, parse tree. Bahala kayo kung ito ay bottom-up or top-down. Na-gets?”

## 2. What Ma'am wants — English digest

### 2.1 The immediate deliverable is a syntax proposal

For **November 12**, add a **Part 3 covering the language's syntax rules** to the documentation. Define the grammar's terminals, nonterminals, productions, and start symbol. Give **one example for every rule**, and show how that example follows from the grammar using **leftmost and rightmost derivations**, with a parse-tree/derivation presentation.

Ma'am says the default meeting arrangement for November 12 is **face-to-face**. The recording does not specify a submission time, upload platform, or required file format.

### 2.2 The syntax analyzer must consume the lexer output

The stated input is the **symbol table**, read **one token at a time**. In practical terms, our next phase must read the token records produced by the lexer and check whether their order matches the language's grammar.

For valid input, show the grammar derivation. Ma'am calls this a “parse tree,” then explains that an **arrow-based derivation display** can show the rule applications; this introduction does not clearly require a graphical tree interface. It also does not fully specify the display format, so retain the later demonstration as the reference for that detail.

For invalid input, report a **syntax error**. The wording appears to include its line, but the exact phrase is unclear. She explicitly mentions **panic mode**. In compiler terminology, panic-mode recovery normally skips input until a chosen synchronization point, then resumes parsing; that explanation is an editorial clarification. The synchronization tokens and exact recovery behavior are not supplied in this clip.

### 2.3 Fix the old lexer feedback before the next checking

Ma'am expects the corrections from the previous lexical-analyzer checking to be complete. This introduction does **not** repeat those individual corrections, so it cannot establish that our group has satisfied every item from her earlier feedback.

### 2.4 Several details belong to the remaining recordings

She refers to required statement forms and says she will demonstrate syntax-analyzer checking later. This clip does not enumerate the full statement checklist, grading rubric, or semantic-analysis requirements. Do not treat our existing examples as her complete required list.

### 2.5 Additional mid-meeting requirement — only one iterative statement

**Source: Adriel's report of Ma'am's mid-meeting instruction, supplied on 5 November 2026; not part of the introduction transcript above.** No recording timestamp was supplied.

> “isang iterative statement langang gagamitin sa PL”

**Working interpretation:** The PL should provide **only one kind of iterative statement (loop construct)**. Choose either `WHILE` or `FOR` for LAlgo's supported syntax. This concerns the available loop form; the supplied wording does not establish a limit of one loop occurrence per program.

**Current mismatch:** The selected lexer recognizes both `WHILE` and `FOR`, and the specification illustrates both. We therefore still need to select one loop form and align the grammar, keyword policy, lexer, documentation, examples, and tests with that decision. Whether the other spelling is removed from the reserved vocabulary or retained as unsupported must be explicit. The team has not selected which loop to keep, and this notes update does not change the language implementation.

## 3. Schedule stated in the recording

All dates below use **2026**, based on the meeting date. They are meeting notes, not an independently checked university calendar. The student's recap at 03:23–03:45 supplies the explicit December 3 date.

| Date | Activity stated in the recording | What our team should prepare |
| --- | --- | --- |
| November 12 | Submit syntax-grammar proposal/documentation; face-to-face by default | Part 3, formal grammar components, one example per rule, and derivations |
| November 19 | Asynchronous | Continue project work |
| November 26 | Lexical analyzer checking 2 and syntax checking 1 | Corrected lexer and the first syntax-analyzer checking milestone; exact rubric is not given here |
| December 3 | Asynchronous | Continue project work |
| December 10 | Syntax checking 2 and semantics checking 1; final project checking and final exam | Prepare for these milestones; detailed semantics scope is not given here |
| December 15–17 | CCIS departmental examinations, mentioned as scheduling context | Keep separate from Ma'am's December 10 checking/exam |
| December 22 | End of the first semester, according to Ma'am | Scheduling context |

## 4. Do we already have these in LAlgo?

**Repository checked:** `be60a71f2979f69632cecda7e223f46206f988b5`, plus a fresh local `make test` on 5 November 2026. **31/31 lexer cases passed.** These are our local checks, not instructor acceptance.

Status meanings: **Present** = supported by the current source/documentation; **Partial** = some groundwork exists; **Missing** = the requested deliverable or implementation is absent; **Needs revision** = the current design conflicts with a reported requirement; **Unverified** = this recording and repository cannot settle it.

| Requirement from this introduction or the separately attributed mid-meeting note | Current status | Evidence and remaining work |
| --- | --- | --- |
| Introduction and existing language-element documentation | **Present** | The [specification source](../../lalgo_language_specification.tex) has `Introduction` and `Lexical Elements`. The recording's Part 2 wording is noted above. |
| Working lexical analyzer | **Present** | [Selected unified lexer](../../lexical_analyzer/lalgo_unified.c); the [validation suite](../../tests/test_lexer.py) passes 31/31 local cases. |
| Only one iterative statement form in the PL (mid-meeting instruction reported by Adriel; Section 2.5) | **Needs revision** | The selected lexer recognizes `KW_WHILE` and `KW_FOR`; the specification's iteration examples include both. Choose one supported loop form and align the grammar, keyword policy, lexer, examples, documentation, and tests. No choice has been made in this update. |
| Corrections from the last instructor checking | **Unverified** | [CHANGELOG](../../CHANGELOG.md) records earlier fixes, but this clip does not list the feedback to reconcile against it. Local test passes cannot establish Ma'am's approval. |
| Part 3 dedicated to syntax rules | **Missing** | The current specification's third main section is **Worked Binary-Search Token Stream**, not a syntax-grammar proposal. Keep that worked example when adding/reorganizing the new section. |
| Formal terminal set for the syntax grammar | **Partial** | The specification and `TOKEN_LIST` define token categories. A parser-facing terminal set is not yet formally selected. Token categories must be distinguished from the lexer's character alphabet. |
| Nonterminals, productions, and a grammar start symbol | **Missing** | No complete syntax grammar is defined. The lexer's `q0` is an automaton start state, not a grammar start symbol. |
| One example per grammar rule, with leftmost/rightmost derivations | **Missing** | Existing declaration, I/O, conditional, loop, and expression examples are illustrative. The worked token stream is lexical output, not a grammar derivation. |
| Symbol-table file produced by the lexer | **Present** | `main()` writes `Symbol Table.txt` with token type, classification, position, and lexeme. It is a token-occurrence listing, not a variable scope/type table. |
| Syntax analyzer reads that file one token at a time | **Missing** | The lexer writes the listing, but no parser or listing-reader implementation exists in this repository. |
| Validate statement structure using grammar rules | **Missing** | The current scanner recognizes lexemes; it does not enforce full statements, matching block structure, or expression grammar. |
| Parse-tree / arrow-based grammar derivation output | **Missing** | Lexer `t` mode traces **DFA state transitions**. It does not show productions applied to a sentence. |
| Syntax-error diagnostics | **Missing** | Current diagnostics cover **lexical** errors with positions. There is no syntax-analysis stage. |
| Panic-mode syntax-error recovery | **Missing** | Lexical recovery exists, but parser synchronization and panic-mode recovery are not implemented. |
| Choice of top-down or bottom-up parsing | **Missing** | No parser strategy is selected or implemented. Ma'am allows a choice; this note does not select one for the team. |
| Semantic-analysis checking milestone | **Missing; detailed scope pending** | The [README](../../README.md) identifies semantic analysis and execution as future work. The introduction names the milestone without specifying all its requirements. |

### 4.1 Decisions needed when connecting the parser

These are implementation recommendations based on the current code, not additional quoted requirements:

1. **Agree on the symbol-table reader contract.** The existing file contains a header, numbered rows, and a footer. Comments and `INVALID` entries are included; EOF is omitted. Define how the reader handles each of these while preserving line/column information. The printed lexeme field can be escaped or shortened, so treat the current listing as an interface to review before relying on it as a lossless token format.
2. **Define statement boundaries.** The lexer recognizes semicolons and braces and tracks newline metadata internally, but the specification deliberately leaves statement-ending rules to the future parser. The text listing does not serialize `precededByNewline` or `suspectedKeyword`. If the grammar needs that information, the interface will need to carry it.
3. **Define treatment of comments, noise words, and future-reserved words.** `SET` and `THE` are tokenized without positional grammar rules. `ALGORITHM`, `RETURN`, `TRACE`, and `INVARIANT` are reserved for future work; token recognition does not make them valid statements. Decide their syntax status explicitly.
4. **Keep the different traces distinct.** A DFA trace explains how characters become tokens. The new derivation output must explain how grammar productions account for those tokens.

## 5. Suggested team checklist, in order

This is a proposed work sequence derived from the introduction and the separately attributed mid-meeting instruction in Section 2.5. It does not change the language or assign tasks to individual members.

- [ ] Read the remaining meeting recordings for the required statement list, demonstration, and any corrections to this introduction.
- [ ] Reconcile Ma'am's previous lexer feedback with the current implementation and changelog.
- [ ] Enforce Ma'am's reported **one iterative statement form** requirement: choose `WHILE` or `FOR`, define the other spelling's status, and align the grammar, keyword policy, lexer, documentation, examples, and tests. The current design includes both.
- [ ] Prepare the new syntax section for November 12, preserving the existing worked token stream.
- [ ] Define terminals, nonterminals, productions, and the start symbol.
- [ ] Settle statement boundaries, operator precedence/associativity, blocks, and the roles of noise/future-reserved words.
- [ ] Give one example for each grammar rule and its leftmost and rightmost derivations; include the requested derivation/tree presentation.
- [ ] Select top-down or bottom-up parsing and agree on the symbol-table input contract.
- [ ] Implement the token reader, syntax checking, and derivation output for the November 26 milestone.
- [ ] Add syntax-error reporting and panic-mode recovery, using the later demonstration to settle format and behavior.
- [ ] Prepare syntax checking 2 and semantics checking 1 for December 10 after their detailed requirements are known.

## 6. Source and verification notes

- Supplemental source: Adriel supplied the exact mid-meeting quote in Section 2.5 on 5 November 2026. It has no verified timestamp and was not inserted into the introduction's chronological transcript. The `WHILE`/`FOR` mismatch was rechecked in the selected lexer and specification when adding this note; implementation files were unchanged.
- Local recording: `mp3 recording/1.mp4`; duration reported by `ffprobe`: **357.266667 seconds**.
- Recording SHA-256: `81fe45c84ff4ae9836a9c7e0f5a812dc580a6b4850e180ba92fe471c83b5fab1`.
- Transcription: local multilingual Whisper large-v3-turbo, automatic-language pass plus a Tagalog-language recheck with technical vocabulary. Agreement between passes helps review but does not prove exact wording. Obvious machine spellings such as “parse 3” were normalized to “parse tree”; repetitive generated text beyond the recording was excluded.
- Unresolved wording is marked in Section 1. Opening administrative remarks are less clear than the project requirements. Exact error-message wording and the complete required statement list remain unresolved.
- Repository comparison used the current specification, selected C lexer, README, changelog, file inventory, and a fresh `make test`; it did not add a parser or change the language specification.
- Raw recognition output and local processing files stay under ignored `build/transcription/2026-11-05/`. The source recording is not included in this documentation commit.

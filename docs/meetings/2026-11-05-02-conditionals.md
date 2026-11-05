# Meeting 02 — Conditional statements and one example per rule

**Meeting date:** 5 November 2026 (Asia/Manila)

**Recording:** `2.mp3` · approximately 1 minute 44 seconds

**Sequence:** Recording 02, following [Recording 01 — Introduction](2026-11-05-01-introduction.md). Timestamps restart at the beginning of this file; the gap between recordings is unknown.

The transcript retains the original Tagalog/English. The explanation and repository comparison below are in English and are separate from Ma'am's words.

**Transcript status:** Machine-assisted and lightly edited for punctuation and technical spellings. Two local speech-recognition passes agree on the main requirements. The audio has not been independently verified by human listening, so this is a readable transcript rather than a certified verbatim record. Timestamps are approximate; repeated fillers are shortened.

## 1. Chronological transcript

### 00:00–00:22 — Three required IF forms

**Ma'am:** “In your programming language, ang required ko lang ay IF statements. Pero tatlo, nabanggit ko na 'yan sa inyo. Tatlo. Ano 'yung tatlo na 'yun? 'Yung single IF, 'yung IF–ELSE, at saka 'yung nested IF, ELSE IF, ELSE IF, ELSE. Right?”

### 00:22–00:42 — Three rules and three examples

**Ma'am:** “So in your IF statement sa inyong programming language design, kailangan meron kayong tatlong rules for that. Sa single IF, sa IF–ELSE, at saka IF–ELSE IF. So dapat meron kayong tatlong examples. Naintindihan?”

### 00:42–01:08 — Give an example for every rule

**Ma'am:** “If you have more than rules—more than three rules in your IF statements—well, I need to have one example for every statement. Kasi napansin ko sa mga students minsan, one example for IF statement. Eh, mali. One example for every rule in that statement.”

> Wording note: The first sentence contains a spoken restart. The following sentences explicitly clarify the requirement as **one example per rule**.

### 01:08–01:34 — Different examples must cover different rules

**Ma'am:** “If you have three rules, then I have three different examples based on each rule. Gets? Meron kasi ang nangyayari, three examples nga, pero isang rule lang naman ang ginagawa. No. If you have three rules, I have three different statements for that rule. Each rule should have one example. Gets?”

### 01:34–01:44 — Derive the examples using your own syntax

**Ma'am:** “Okay? Tapos ide-derive n'yo 'yan using the syntax that you have developed or constructed.”

## 2. What this means for our team

### 2.1 Cover all three conditional forms

| Required form | Meaning | What its example must demonstrate |
| --- | --- | --- |
| Single `IF` | Execute a body when a condition holds, without an alternative branch | An `IF` with no `ELSE` |
| `IF–ELSE` | Select between two alternative bodies | An `IF` body and a plain `ELSE` body |
| `IF–ELSE IF–ELSE` | Test additional conditions before a final alternative | An `IF`, one or more `ELSE IF` branches, and a final `ELSE` |

Ma'am calls the third form “nested IF” and then describes an `ELSE IF` chain. These notes use **IF–ELSE IF–ELSE chain** for that form to make the intended example clear. In general programming terminology, an IF inside another IF's body is also called a nested IF; this clip does not clearly establish that as a separate fourth required form.

Her opening “required ko lang ay IF statements” is understood here in the context of **conditional statements**. It does not cancel the separate instruction about choosing one iterative-statement form recorded in [Recording 01, Section 2.5](2026-11-05-01-introduction.md#25-additional-mid-meeting-requirement--only-one-iterative-statement).

### 2.2 Match each rule to its own example

Three examples that all use a single IF do not cover the three required forms. Make the mapping explicit: **rule → matching example → derivation**. If our conditional syntax has additional rules requiring examples, the examples must cover those too; three is not a cap when more rules are included.

This recording does not specify exactly how helper productions should be counted for the example requirement. At minimum, give separate coverage to the three named conditional forms and make the mapping easy for Ma'am to inspect.

### 2.3 Show derivations, not just source snippets

Every example must be derived using **our own defined grammar**. A snippet that looks familiar or passes the lexer is not enough: the productions must actually generate it. [Recording 01](2026-11-05-01-introduction.md) also requires leftmost and rightmost derivations and a parse-tree/derivation presentation. Those earlier requirements still apply; recording 02 reinforces the per-rule example requirement.

## 3. Illustrative examples for the three forms

These are editorial illustrations using LAlgo's existing token vocabulary. They are **not approved grammar rules, completed derivations, or executable-program claims**. Their exact syntax must be aligned with the formal grammar we develop.

### 3.1 Single IF

```text
IF (score >= 75) {
    OUTPUT "Passed"
}
```

Coverage: one conditional body, no alternative branch.

### 3.2 IF–ELSE

```text
IF (score >= 75) {
    OUTPUT "Passed"
}
ELSE {
    OUTPUT "Failed"
}
```

Coverage: two alternatives, with no `ELSE IF` branch.

### 3.3 IF–ELSE IF–ELSE

```text
IF (score >= 90) {
    OUTPUT "Excellent"
}
ELSE IF (score >= 75) {
    OUTPUT "Passed"
}
ELSE {
    OUTPUT "Failed"
}
```

Coverage: an additional condition before the final alternative. If our grammar allows repeated `ELSE IF` branches, document and demonstrate that rule as needed.

## 4. Do we already have these in LAlgo?

**Repository inspected:** `402c6d7`, the current commit before these notes. This comparison inspects the selected lexer, specification, examples, and existing test scope; no new parser validation is claimed.

| Requirement | Current status | Evidence and remaining work |
| --- | --- | --- |
| Recognize `IF` and `ELSE` | **Present at the lexical level** | [Selected lexer](../../lexical_analyzer/lalgo_unified.c) defines `KW_IF` and `KW_ELSE`. `ELSE IF` is a sequence of those two tokens, not a separate keyword. |
| Single-IF example | **Partial** | [demo.lal](../../lexical_analyzer/demo.lal) contains an IF without ELSE. It is an illustrative lexer input, with no corresponding formal syntax rule or grammar derivation. |
| Separate IF–ELSE example and rule | **Missing as a dedicated grammar deliverable** | The current conditional illustration in the [specification](../../lalgo_language_specification.tex) is a longer IF–ELSE IF–ELSE chain. Its branches do not replace a separate two-way example and rule. Section 3.2 above offers a candidate example, not a completed syntax proposal. |
| IF–ELSE IF–ELSE example | **Partial** | The specification and [sample.lal](../../lexical_analyzer/sample.lal) show this chain in binary search. The formal grammar and derivations are missing. |
| Three explicitly covered conditional forms/rules | **Missing** | The specification describes conditional forms but explicitly leaves complete statement grammar to future work. Map each named form to the syntax productions and its own example. |
| One example per rule, including additional rules | **Missing as a documented mapping** | Existing samples are not organized into a rule-to-example coverage table. Merely having several IF occurrences is insufficient. |
| Derive each example from our grammar | **Missing** | Neither the worked lexical token stream nor the DFA trace is a grammar derivation. Add the required leftmost/rightmost derivations once the productions are defined. |
| Parse and validate conditional syntax | **Missing** | The implementation tokenizes source text; it does not yet validate conditional structure or generate parse trees. Existing lexer tests do not prove parser behavior. |

## 5. Things we need to make sure

- [ ] Cover **single IF**, **IF–ELSE**, and **IF–ELSE IF–ELSE** explicitly in the syntax proposal.
- [ ] Map each required conditional form to its grammar rules and a distinct matching example.
- [ ] Cover any additional conditional rules with examples as required; do not reuse three examples of the same form as coverage for different rules.
- [ ] Derive each example using the productions actually written in our proposal.
- [ ] Include the leftmost/rightmost derivations and derivation/tree presentation required in recording 01.
- [ ] Keep the conditional requirements separate from the **one loop form** requirement; both must be addressed.
- [ ] When implementing the parser, verify all supported conditional forms and reject malformed structures. Decide how `ELSE` associates with `IF` in our grammar.

The final parser item is an implementation recommendation derived from the requirement to validate syntax, not an additional quotation from this clip. This update adds notes and candidate examples; it does not modify the language implementation or specification.

## 6. Source and verification notes

- Source: local `mp3 recording/2.mp3`; `ffprobe` duration: **104.141333 seconds**.
- Recording SHA-256: `0629814497a3c274ec52d09e172e5ae6ee839414956d492aef3b3b8e9d50f726`.
- Transcription: local Whisper large-v3-turbo, one automatically detected Tagalog pass and one Tagalog-language recheck with technical vocabulary. Both recover the three forms, the per-rule example requirement, and the instruction to derive the examples.
- The exact spoken wording has grammatical restarts; punctuation and technical capitalization were normalized without changing the stated requirements. The meaning of “nested IF” is explained in Section 2.1 rather than silently replacing the quoted phrase.
- Raw recognition output and processing scripts remain under ignored `build/transcription/2026-11-05/`. The recording is not included in the documentation commit.

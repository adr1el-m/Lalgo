/*
 * lalgo_parser.c -- LAlgo syntax analyzer (top-down, recursive descent)
 *
 * PUP COSC 303 - Group 4
 *
 * HOW IT WORKS (maps to the Syntax Analyzer 1 checklist)
 *   1. INPUT FROM THE SYMBOL TABLE, ONE TOKEN AT A TIME
 *        The lexer writes "Symbol Table.txt". This program opens that file and
 *        reads it line by line. next_tok() hands over ONE terminal per call.
 *        collect_statement() keeps asking for tokens until the statement's
 *        TERMINATOR is reached:
 *            simple statement  -> a ';'   (at brace depth 0)
 *            block statement   -> its matching '}' (IF looks ONE token ahead
 *                                 to see whether ELSE continues the chain)
 *        Only then is that statement parsed.
 *   2. PARSER ALGORITHM: plain recursive descent, one function per EBNF rule
 *        of Chapter 3 (R01 - R45). Comments are skipped by the token reader.
 *   5-10. Input, output (3 forms), assignment (3 forms), IF / IF-ELSE /
 *        IF-ELSE IF-ELSE, WHILE (plain + nested), declaration (3 forms) and
 *        SWITCH are all recognized. Every accepted statement prints the name
 *        of the rule that matched.
 *   11. ERROR RECOVERY: panic mode (see sync()). The error is reported, the
 *        damaged part of the statement is discarded up to a synchronizing
 *        point, and parsing CONTINUES with the next statement.
 *   12. ERROR MESSAGES:  "Syntax error on line L, column C: <reason>"
 *
 * BUILD :  gcc -std=c99 -Wall -Wextra -o lalgo_parser lalgo_parser.c
 * RUN   :  ./lalgo sample.lal              (lexer writes "Symbol Table.txt")
 *          ./lalgo_parser                  (reads "Symbol Table.txt")
 *          ./lalgo_parser other.txt        (reads another symbol table file)
 *          ./lalgo_parser "Symbol Table.txt" v   (also list collected terminals)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <setjmp.h>

/* ========================================================================== */
/* 1. TERMINALS = the token names written by the lexer in the symbol table    */
/* ========================================================================== */

#define TOKS(X)                                                               \
    X(IDENT) X(INTEGER) X(DECIMAL) X(STRING) X(CHAR) X(TRUE) X(FALSE)         \
    X(KW_INTEGER) X(KW_DECIMAL) X(KW_CHARACTER) X(KW_BOOLEAN) X(KW_ARRAY)     \
    X(KW_STRING) X(KW_IF) X(KW_ELSE) X(KW_WHILE) X(KW_FOR) X(KW_SWITCH)       \
    X(KW_CASE) X(KW_DEFAULT) X(KW_BREAK) X(KW_INPUT) X(KW_OUTPUT)             \
    X(DIV) X(AND) X(OR) X(NOT) X(PLUS) X(MINUS) X(STAR) X(SLASH) X(PERCENT)   \
    X(CARET) X(ASSIGN) X(EQ) X(NEQ) X(LT) X(GT) X(LE) X(GE)                   \
    X(LPAREN) X(RPAREN) X(LBRACKET) X(RBRACKET) X(LBRACE) X(RBRACE)           \
    X(COMMA) X(SEMICOLON) X(COLON) X(DOT) X(NOISE_SET) X(NOISE_THE)           \
    X(RES_ALGORITHM) X(RES_RETURN) X(RES_TRACE) X(RES_INVARIANT)              \
    X(INVALID) X(LINE_COMMENT) X(BLOCK_COMMENT) X(EOF)

#define X_ENUM(n) K_##n,
#define X_NAME(n) #n,
enum { TOKS(X_ENUM) NUM_TT };
static const char *TT_NAME[] = { TOKS(X_NAME) };

typedef struct { int type, line, col; char *lex; } Tok;

/* ========================================================================== */
/* 2. TOKEN READER: one record of the symbol table per call                   */
/* ========================================================================== */

/* Symbol Table.txt row layout written by the lexer:
 *   "%-5d %-16s %-30s %-9s %s"  -> No. | Token | Classification | Line:Col | Lexeme
 *   columns start at offsets 0, 6, 23, 54, 64                                   */
#define COL_TOKEN 6
#define COL_POS   54
#define COL_LEXEME 64

static FILE *fin;
static int   in_table, last_line = 1, last_col = 1;
static Tok   peeked;
static int   has_peek;

static int type_of(const char *name)
{
    int i;
    for (i = 0; i < NUM_TT; i++)
        if (strcmp(TT_NAME[i], name) == 0) return i;
    return K_INVALID;
}

static char *dup_str(const char *s)
{
    char *d = malloc(strlen(s) + 1);
    if (!d) { fprintf(stderr, "out of memory\n"); exit(2); }
    strcpy(d, s);
    return d;
}

/* returns 0 at the end of the table */
static int read_raw(Tok *t)
{
    char line[4096], name[17];
    size_t n;
    int ln, cl, k;

    while (fgets(line, sizeof line, fin)) {
        n = strlen(line);
        while (n && (line[n - 1] == '\n' || line[n - 1] == '\r')) line[--n] = '\0';
        if (!in_table) {                       /* skip the title lines            */
            if (strncmp(line, "---", 3) == 0) in_table = 1;
            continue;
        }
        if (n == 0) continue;
        if (strncmp(line, "Total entries", 13) == 0) return 0;
        if (n <= COL_LEXEME) continue;         /* not a data row                  */

        memcpy(name, line + COL_TOKEN, 16);
        name[16] = '\0';
        for (k = 15; k >= 0 && name[k] == ' '; k--) name[k] = '\0';
        if (sscanf(line + COL_POS, "%d:%d", &ln, &cl) != 2) continue;

        t->type = type_of(name);
        t->line = ln; t->col = cl;
        t->lex  = dup_str(line + COL_LEXEME);
        return 1;
    }
    return 0;
}

/* comments are not terminals: the reader silently skips them */
static Tok read_filtered(void)
{
    Tok t;
    while (read_raw(&t)) {
        if (t.type == K_LINE_COMMENT || t.type == K_BLOCK_COMMENT) continue;
        last_line = t.line;
        last_col  = t.col + (int)strlen(t.lex);
        return t;
    }
    t.type = K_EOF; t.line = last_line; t.col = last_col; t.lex = dup_str("");
    return t;
}

static Tok next_tok(void)
{
    if (has_peek) { has_peek = 0; return peeked; }
    return read_filtered();
}

static Tok peek_tok(void)
{
    if (!has_peek) { peeked = read_filtered(); has_peek = 1; }
    return peeked;
}

/* ========================================================================== */
/* 3. STATEMENT COLLECTOR: receive tokens until the statement terminator      */
/* ========================================================================== */

static Tok *B;                 /* the collected terminals of ONE statement     */
static int  blen, bcap, p;     /* p = parser cursor into B                     */

static void push(Tok t)
{
    if (blen >= bcap) {
        bcap = bcap ? bcap * 2 : 64;
        B = realloc(B, (size_t)bcap * sizeof *B);
        if (!B) { fprintf(stderr, "out of memory\n"); exit(2); }
    }
    B[blen++] = t;
}

static void collect_statement(void)
{
    int depth = 0;
    Tok t;
    blen = 0;
    for (;;) {
        t = next_tok();
        push(t);
        if (t.type == K_EOF) return;
        if (t.type == K_LBRACE) depth++;
        else if (t.type == K_RBRACE) {
            depth--;
            if (depth <= 0) {
                /* one token of lookahead: ELSE extends an IF chain */
                if (depth == 0 && peek_tok().type == K_KW_ELSE) continue;
                break;
            }
        } else if (t.type == K_SEMICOLON && depth == 0) break;
    }
    t.type = K_EOF; t.line = last_line; t.col = last_col; t.lex = dup_str("");
    push(t);                                   /* sentinel so the parser stops */
}

/* ========================================================================== */
/* 4. ERRORS AND RULE LOG                                                     */
/* ========================================================================== */

static jmp_buf *cur_jb;
static int  total_errors, nerr_stmt, depth_lvl;
static char errs[64][320];

typedef struct { int depth, line; const char *rule; } LogE;
static LogE LOG[512];
static int  nlog;

static int log_begin(void)
{
    if (nlog >= 512) return -1;
    LOG[nlog].depth = depth_lvl; LOG[nlog].line = B[p].line; LOG[nlog].rule = NULL;
    return nlog++;
}
static void log_set(int i, const char *rule) { if (i >= 0) LOG[i].rule = rule; }

static void found_str(const Tok *t, char *buf, size_t n)
{
    if (t->type == K_EOF)          snprintf(buf, n, "end of file");
    else if (t->type == K_INVALID) snprintf(buf, n, "'%.40s' (lexical error)", t->lex);
    else                           snprintf(buf, n, "'%.40s'", t->lex);
}

static void vreport(const Tok *at, int with_found, const char *fmt, va_list ap)
{
    char msg[160], fnd[80], full[320];
    vsnprintf(msg, sizeof msg, fmt, ap);
    if (with_found) {
        found_str(&B[p], fnd, sizeof fnd);
        snprintf(full, sizeof full, "Syntax error on line %d, column %d: %s, found %s",
                 at->line, at->col, msg, fnd);
    } else
        snprintf(full, sizeof full, "Syntax error on line %d, column %d: %s",
                 at->line, at->col, msg);
    if (nerr_stmt < 64) strcpy(errs[nerr_stmt], full);
    nerr_stmt++; total_errors++;
}

/* non-fatal: record the error and keep parsing */
static void report(const Tok *at, int with_found, const char *fmt, ...)
{
    va_list ap; va_start(ap, fmt); vreport(at, with_found, fmt, ap); va_end(ap);
}

/* fatal for this statement: record the error, jump to the recovery point */
static void fail_found(const char *fmt, ...)
{
    va_list ap; va_start(ap, fmt); vreport(&B[p], 1, fmt, ap); va_end(ap);
    longjmp(*cur_jb, 1);
}
static void fail_plain(const char *fmt, ...)
{
    va_list ap; va_start(ap, fmt); vreport(&B[p], 0, fmt, ap); va_end(ap);
    longjmp(*cur_jb, 1);
}

/* ========================================================================== */
/* 5. PARSER HELPERS                                                          */
/* ========================================================================== */

static const char *ctx = "statement";   /* what is being parsed (for messages) */
static int while_count;                 /* WHILE keywords consumed so far      */

static int cur(void) { return B[p].type; }
static int accept(int tt) { if (cur() == tt) { p++; return 1; } return 0; }

static void expect(int tt, const char *what)
{
    if (cur() == tt) { p++; return; }
    fail_found("expected %s", what);
}
static void expect_semi(void)
{
    if (cur() == K_SEMICOLON) { p++; return; }
    fail_found("expected ';' after %s", ctx);
}

static int is_type(int t)
{ return t >= K_KW_INTEGER && t <= K_KW_STRING; }

static int is_literal(int t)
{ return t == K_INTEGER || t == K_DECIMAL || t == K_STRING || t == K_CHAR ||
         t == K_TRUE || t == K_FALSE; }

static void parse_statement(void);
static void parse_expression(void);
static void parse_block(void);

/* ---- panic-mode recovery -------------------------------------------------- *
 * Called after an error. Starting at the token where the error was found, the
 * parser DISCARDS tokens, tracking ( [ { nesting, until it reaches a point
 * where a new statement can safely begin:
 *   - a ';' at nesting 0         -> consumed (it ends the damaged statement)
 *   - a '}' at nesting 0         -> NOT consumed (it closes the enclosing block)
 *   - a '}' that closes a '{' opened while skipping -> consumed
 *   - an unambiguous statement starter at nesting 0 (a type keyword, INPUT,
 *     OUTPUT, IF, WHILE, SWITCH, SET) or CASE / DEFAULT / BREAK of a switch
 *     arm -> NOT consumed (typical after a missing ';')
 * The token at the start of the failed statement is always skipped, so every
 * recovery makes progress.                                                     */
static void sync(int start)
{
    int d = 0;
    while (cur() != K_EOF) {
        int t = cur();
        if (d == 0) {
            if (t == K_SEMICOLON) { p++; return; }
            if (t == K_RBRACE) return;
            if (p > start && (is_type(t) || t == K_KW_INPUT || t == K_KW_OUTPUT ||
                              t == K_KW_IF || t == K_KW_WHILE || t == K_KW_SWITCH ||
                              t == K_NOISE_SET || t == K_KW_CASE ||
                              t == K_KW_DEFAULT || t == K_KW_BREAK))
                return;
        }
        if (t == K_LPAREN || t == K_LBRACKET || t == K_LBRACE) d++;
        else if (t == K_RPAREN || t == K_RBRACKET) { if (d > 0) d--; }
        else if (t == K_RBRACE) { d--; p++; if (d == 0) return; continue; }
        p++;
    }
}

/* parse ONE statement; on an error recover and return normally */
static void statement_with_recovery(void)
{
    int start = p, saved_depth = depth_lvl;
    jmp_buf jb;
    jmp_buf *saved = cur_jb;
    cur_jb = &jb;
    if (setjmp(jb) == 0) parse_statement();
    else { depth_lvl = saved_depth; sync(start); }
    cur_jb = saved;
}

/* ========================================================================== */
/* 6. EXPRESSIONS  (R29 - R41)                                                */
/* ========================================================================== */

static void parse_reference(void)                          /* R39 */
{
    expect(K_IDENT, "variable name");
    if (accept(K_LBRACKET)) {
        parse_expression();
        expect(K_RBRACKET, "']' to close the index");
    }
}

static void parse_primary(void)                            /* R38, R40, R41 */
{
    if (is_literal(cur())) { p++; return; }
    if (cur() == K_IDENT)  { parse_reference(); return; }
    if (accept(K_LPAREN)) {
        parse_expression();
        expect(K_RPAREN, "')' to close the expression");
        return;
    }
    if (accept(K_LBRACKET)) {                              /* ArrayLiteral */
        if (cur() != K_RBRACKET) {
            parse_expression();
            while (accept(K_COMMA)) parse_expression();
        }
        expect(K_RBRACKET, "']' to close the array");
        return;
    }
    fail_found("expected an expression");
}

static void parse_unary(void);

static void parse_power(void)                              /* R37 */
{
    parse_primary();
    if (accept(K_CARET)) parse_unary();
}

static void parse_unary(void)                              /* R36 */
{
    if (accept(K_MINUS)) parse_unary(); else parse_power();
}

static void parse_product(void)                            /* R35 */
{
    parse_unary();
    while (cur() == K_STAR || cur() == K_SLASH || cur() == K_PERCENT || cur() == K_DIV)
    { p++; parse_unary(); }
}

static void parse_sum(void)                                /* R34 */
{
    parse_product();
    while (cur() == K_PLUS || cur() == K_MINUS) { p++; parse_product(); }
}

static void parse_comparison(void)                         /* R33 */
{
    parse_sum();
    while (cur() == K_LT || cur() == K_LE || cur() == K_GT || cur() == K_GE)
    { p++; parse_sum(); }
}

static void parse_equality(void)                           /* R32 */
{
    parse_comparison();
    while (cur() == K_EQ || cur() == K_NEQ) { p++; parse_comparison(); }
}

static void parse_negation(void)                           /* R31 */
{
    if (accept(K_NOT)) parse_negation(); else parse_equality();
}

static void parse_conjunction(void)                        /* R30 */
{
    parse_negation();
    while (accept(K_AND)) parse_negation();
}

static void parse_expression(void)                         /* R29 */
{
    parse_conjunction();
    while (accept(K_OR)) parse_conjunction();
}

/* what is B[s..e) ?  1 = a single literal, 2 = a bare reference, 0 = expression */
static int shape_of(int s, int e)
{
    int i, d = 0;
    if (e - s == 1 && is_literal(B[s].type)) return 1;
    if (B[s].type != K_IDENT) return 0;
    if (e - s == 1) return 2;
    if (B[s + 1].type == K_LBRACKET && B[e - 1].type == K_RBRACKET) {
        for (i = s + 1; i < e; i++) {
            if (B[i].type == K_LBRACKET) d++;
            else if (B[i].type == K_RBRACKET) { d--; if (d == 0 && i != e - 1) return 0; }
        }
        return 2;
    }
    return 0;
}

/* ========================================================================== */
/* 7. SIMPLE STATEMENTS                                                       */
/* ========================================================================== */

static void parse_declaration(void)                        /* R13 - R17 */
{
    int e = log_begin();
    ctx = "declaration";
    p++;                                                   /* Type */
    expect(K_IDENT, "variable name after the data type");
    if (accept(K_SEMICOLON)) { log_set(e, "R14 DeclareOne"); return; }
    if (cur() == K_COMMA) {
        while (accept(K_COMMA)) expect(K_IDENT, "variable name after ','");
        expect_semi();
        log_set(e, "R15 DeclareList");
        return;
    }
    if (accept(K_ASSIGN)) {
        parse_expression();
        expect_semi();
        log_set(e, "R16 DeclareInit");
        return;
    }
    fail_found("expected ';', ',' or '=' after the variable name");
}

static void parse_input(void)                              /* R04 */
{
    int e = log_begin();
    ctx = "input statement";
    p++;
    expect(K_IDENT, "variable name after INPUT");
    expect_semi();
    log_set(e, "R04 InputStmt");
}

static void parse_output(void)                             /* R05 - R08 */
{
    int e = log_begin();
    ctx = "output statement";
    p++;
    accept(K_NOISE_THE);
    if (cur() == K_STRING) {
        p++;
        if (accept(K_SEMICOLON)) { log_set(e, "R06 OutputText"); return; }
        if (accept(K_COMMA)) {
            parse_reference();
            expect_semi();
            log_set(e, "R08 OutputMixed");
            return;
        }
        fail_found("expected ',' or ';' after the string in OUTPUT");
    }
    if (cur() == K_IDENT) {
        parse_reference();
        expect_semi();
        log_set(e, "R07 OutputValue");
        return;
    }
    fail_found("expected a string or a variable after OUTPUT");
}

static void parse_assignment(void)                         /* R09 - R12 */
{
    int e = log_begin(), s, en;
    ctx = "assignment";
    accept(K_NOISE_SET);
    parse_reference();
    expect(K_ASSIGN, "'=' after the variable");
    s = p;
    parse_expression();
    en = p;
    expect_semi();
    switch (shape_of(s, en)) {
        case 2:  log_set(e, "R10 AssignVariable");   break;
        case 1:  log_set(e, "R11 AssignConstant");   break;
        default: log_set(e, "R12 AssignExpression"); break;
    }
}

/* ========================================================================== */
/* 8. BLOCKS AND CONTROL STATEMENTS                                           */
/* ========================================================================== */

static void parse_block(void)                              /* R03 */
{
    expect(K_LBRACE, "'{' to start the block");
    depth_lvl++;
    while (cur() != K_RBRACE && cur() != K_EOF) {
        int before = p;
        statement_with_recovery();
        if (p == before && cur() != K_RBRACE) p++;         /* safety: progress */
    }
    depth_lvl--;
    expect(K_RBRACE, "'}' to close the block");
}

static void parse_if_head(void)                            /* R19 */
{
    expect(K_KW_IF, "IF");
    expect(K_LPAREN, "'(' after IF");
    parse_expression();
    expect(K_RPAREN, "')' after the IF condition");
    parse_block();
}

static void parse_if(void)                                 /* R18 - R23 */
{
    int e = log_begin(), chain = 0, has_else = 0;
    ctx = "IF statement";
    parse_if_head();
    while (cur() == K_KW_ELSE) {
        p++;
        if (cur() == K_KW_IF) { parse_if_head(); chain++; }
        else { parse_block(); has_else = 1; break; }
    }
    if (chain > 0 && !has_else) {
        report(&B[p - 1], 0, "expected a final ELSE block after ELSE IF");
        return;
    }
    if (chain == 0 && !has_else) log_set(e, "R20 IfOnly");
    else if (chain == 0)         log_set(e, "R21 IfElse");
    else                         log_set(e, "R22 IfChain");
}

static void parse_while(void)                              /* R24 - R28 */
{
    int e = log_begin(), w0;
    ctx = "WHILE statement";
    p++; while_count++;
    expect(K_LPAREN, "'(' after WHILE");
    parse_expression();
    expect(K_RPAREN, "')' after the WHILE condition");
    w0 = while_count;
    parse_block();
    log_set(e, while_count > w0 ? "R27 NestedWhile" : "R26 PlainWhile");
}

static void parse_arm_body(const char *what)
{
    while (cur() != K_KW_BREAK && cur() != K_KW_CASE && cur() != K_KW_DEFAULT &&
           cur() != K_RBRACE && cur() != K_EOF) {
        int before = p;
        statement_with_recovery();
        if (p == before && cur() != K_RBRACE) p++;
    }
    if (cur() == K_KW_BREAK) {
        p++;
        expect(K_SEMICOLON, "';' after BREAK");
    } else if (cur() == K_EOF) {
        fail_found("expected BREAK; to end the %s arm", what);
    } else {
        report(&B[p], 1, "expected BREAK; to end the %s arm", what);
    }
}

static void parse_switch(void)                             /* R42 - R45 */
{
    int e = log_begin(), ncase = 0;
    ctx = "SWITCH statement";
    p++;
    expect(K_LPAREN, "'(' after SWITCH");
    parse_reference();
    expect(K_RPAREN, "')' after the SWITCH selector");
    expect(K_LBRACE, "'{' to start the SWITCH body");
    depth_lvl++;
    while (cur() == K_KW_CASE) {
        p++;
        if (accept(K_MINUS)) expect(K_INTEGER, "an integer after '-' in the case label");
        else if (cur() == K_INTEGER || cur() == K_CHAR || cur() == K_STRING ||
                 cur() == K_TRUE || cur() == K_FALSE) p++;
        else fail_found("expected a case label (integer, character, string, TRUE or FALSE)");
        expect(K_COLON, "':' after the case label");
        parse_arm_body("CASE");
        ncase++;
    }
    if (ncase == 0) fail_found("expected at least one CASE arm");
    if (cur() != K_KW_DEFAULT) fail_found("expected a DEFAULT arm as the last arm");
    p++;
    expect(K_COLON, "':' after DEFAULT");
    parse_arm_body("DEFAULT");
    depth_lvl--;
    expect(K_RBRACE, "'}' to close the SWITCH");
    log_set(e, "R42 SwitchStmt");
}

/* ========================================================================== */
/* 9. STATEMENT DISPATCH: the first terminal selects the statement family     */
/* ========================================================================== */

static void parse_statement(void)                          /* R02 */
{
    int t = cur();
    if (is_type(t))                         parse_declaration();
    else if (t == K_KW_INPUT)               parse_input();
    else if (t == K_KW_OUTPUT)              parse_output();
    else if (t == K_NOISE_SET || t == K_IDENT) parse_assignment();
    else if (t == K_KW_IF)                  parse_if();
    else if (t == K_KW_WHILE)               parse_while();
    else if (t == K_KW_SWITCH)              parse_switch();
    else if (t == K_KW_ELSE)   fail_plain("ELSE without a matching IF");
    else if (t == K_KW_BREAK)  fail_plain("BREAK is only allowed at the end of a CASE or DEFAULT arm");
    else if (t == K_KW_CASE || t == K_KW_DEFAULT)
                               fail_plain("CASE/DEFAULT is only allowed inside a SWITCH");
    else if (t == K_KW_FOR)    fail_plain("FOR is not part of the grammar; use WHILE");
    else if (t == K_NOISE_THE) fail_plain("THE is only allowed right after OUTPUT");
    else if (t >= K_RES_ALGORITHM && t <= K_RES_INVARIANT)
                               fail_plain("'%s' is a reserved word and cannot start a statement", B[p].lex);
    else                       fail_found("expected a statement");
}

/* ========================================================================== */
/* 10. MAIN                                                                   */
/* ========================================================================== */

static void print_statement(int number, int verbose)
{
    int i;
    printf("Statement %d (lines %d-%d, %d terminal(s) collected)\n", number,
           B[0].line, B[blen >= 2 ? blen - 2 : 0].line, blen - 1);
    if (verbose) {
        printf("  terminals:");
        for (i = 0; i < blen - 1; i++) printf(" %s", TT_NAME[B[i].type]);
        printf("\n");
    }
}

int main(int argc, char **argv)
{
    const char *path = argc >= 2 ? argv[1] : "Symbol Table.txt";
    int verbose = argc >= 3 && argv[2][0] == 'v';
    int number = 0, i;

    fin = fopen(path, "r");
    if (!fin) { fprintf(stderr, "cannot open symbol table '%s' (run the lexer first)\n", path); return 2; }

    printf("=== LAlgo SYNTAX ANALYZER (recursive descent) ===\n");
    printf("Reading terminals one at a time from: %s\n\n", path);

    for (;;) {
        int first_error_free;
        collect_statement();                       /* tokens until the terminator */
        if (blen == 1 && B[0].type == K_EOF) break;

        print_statement(++number, verbose);
        p = 0; nlog = 0; nerr_stmt = 0;
        first_error_free = total_errors;

        while (cur() != K_EOF) {                   /* normally exactly one statement */
            if (cur() == K_RBRACE) {               /* '}' with no open block */
                report(&B[p], 0, "unexpected '}' without a matching '{'");
                p++;
                continue;
            }
            statement_with_recovery();
        }
        for (i = 0; i < nlog; i++)
            if (LOG[i].rule) {
                printf("  %*sline %-3d %s\n", LOG[i].depth * 2, "", LOG[i].line, LOG[i].rule);
            }
        for (i = 0; i < nerr_stmt && i < 64; i++) printf("  %s\n", errs[i]);
        if (total_errors == first_error_free) printf("  -> OK\n");
        printf("\n");
    }

    printf("=== RESULT: %d statement(s) checked, %d syntax error(s) ===\n", number, total_errors);
    fclose(fin);
    return total_errors > 0 ? 1 : 0;
}

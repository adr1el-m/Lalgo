/*
 * lalgo_unified.c -- LAlgo lexical analyzer (ONE unified finite automaton)
 *
 * PUP COSC 303 - Group 4
 *
 * RULES FOLLOWED
 *   1. The lexer is ONE deterministic finite automaton with ONE start state
 *      (q0). Keywords, noise words, identifiers, numbers, strings, operators,
 *      delimiters, comments and whitespace ALL branch out of q0.
 *   2. The automaton is simulated ONLY by transition-table lookup:
 *          next_state = DELTA[current_state][character_class]
 *      There is no string comparison anywhere (no strcmp/strncmp/memcmp, and
 *      <string.h> is not included). Keywords are not looked up in a list:
 *      their letters are states of the same automaton.
 *   3. Input: plain-text file with the LAlgo extension (.lal). Any other file
 *      type is rejected.
 *   4. Output: a plain-text file named "Symbol Table.txt" listing every
 *      recognized token with its lexeme and its classification.
 *   5. Error checking: illegal characters, malformed numbers, out-of-range
 *      integers, invalid/unterminated string, character and block-comment
 *      lexemes are all reported (line and column).
 *
 * BUILD :  gcc -std=c99 -Wall -Wextra -o lalgo lalgo_unified.c
 * RUN   :  ./lalgo sample.lal        token table + lexical errors on screen,
 *                                    "Symbol Table.txt" written
 *          ./lalgo sample.lal t      same, plus the step-by-step DFA trace
 *                                    (tip: add  > trace.txt  to save it)
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

/* ========================================================================== */
/* 1. TOKEN TYPES AND THEIR CLASSIFICATIONS (Section 2.3.12 of the spec)      */
/* ========================================================================== */

#define TOKEN_LIST(X)                                                          \
    X(IDENT, "Identifier")                                                     \
    X(INTEGER, "Constant (Integer)") X(DECIMAL, "Constant (Decimal)")          \
    X(STRING, "Constant (String)") X(CHAR, "Constant (Character)")             \
    X(TRUE, "Constant (Boolean)") X(FALSE, "Constant (Boolean)")               \
    X(KW_INTEGER, "Keyword (Data Type)") X(KW_DECIMAL, "Keyword (Data Type)")  \
    X(KW_CHARACTER, "Keyword (Data Type)")                                     \
    X(KW_BOOLEAN, "Keyword (Data Type)") X(KW_ARRAY, "Keyword (Data Type)")    \
    X(KW_STRING, "Keyword (Data Type)")                                        \
    X(KW_IF, "Keyword (Control Flow)") X(KW_ELSE, "Keyword (Control Flow)")    \
    X(KW_WHILE, "Keyword (Control Flow)") X(KW_FOR, "Keyword (Control Flow)")  \
    X(KW_INPUT, "Keyword (I/O)") X(KW_OUTPUT, "Keyword (I/O)")                 \
    X(DIV, "Arithmetic Operator") X(AND, "Boolean Operator (Logical)")         \
    X(OR, "Boolean Operator (Logical)") X(NOT, "Boolean Operator (Logical)")   \
    X(PLUS, "Arithmetic Operator") X(MINUS, "Arithmetic Operator")             \
    X(STAR, "Arithmetic Operator") X(SLASH, "Arithmetic Operator")             \
    X(PERCENT, "Arithmetic Operator") X(CARET, "Arithmetic Operator")          \
    X(ASSIGN, "Assignment Operator")                                           \
    X(EQ, "Boolean Operator (Relational)")                                     \
    X(NEQ, "Boolean Operator (Relational)")                                    \
    X(LT, "Boolean Operator (Relational)")                                     \
    X(GT, "Boolean Operator (Relational)")                                     \
    X(LE, "Boolean Operator (Relational)")                                     \
    X(GE, "Boolean Operator (Relational)")                                     \
    X(LPAREN, "Bracket") X(RPAREN, "Bracket")                                  \
    X(LBRACKET, "Bracket") X(RBRACKET, "Bracket")                              \
    X(LBRACE, "Bracket") X(RBRACE, "Bracket")                                  \
    X(COMMA, "Delimiter") X(SEMICOLON, "Delimiter")                            \
    X(COLON, "Delimiter") X(DOT, "Delimiter")                                  \
    X(NOISE_SET, "Noise Word") X(NOISE_THE, "Noise Word")                      \
    X(RES_ALGORITHM, "Reserved Word (Future)")                                 \
    X(RES_RETURN, "Reserved Word (Future)")                                    \
    X(RES_TRACE, "Reserved Word (Future)")                                     \
    X(RES_INVARIANT, "Reserved Word (Future)")                                 \
    X(INVALID, "Invalid (Not Recognized)")                                     \
    X(LINE_COMMENT, "Comment (Single-Line)")                                   \
    X(BLOCK_COMMENT, "Comment (Multi-Line)")

#define X_ENUM(n, c)  T_##n,
#define X_NAME(n, c)  #n,
#define X_CLASS(n, c) c,
enum { TOKEN_LIST(X_ENUM) T_EOF, NUM_TOKENS };
static const char *TOKEN_NAMES[] = { TOKEN_LIST(X_NAME) "EOF" };
static const char *TOKEN_CLASS[] = { TOKEN_LIST(X_CLASS) "End of File" };

typedef struct {
    int   type;
    char *lexeme;
    int   line, col;
    int   nl_before;   /* precededByNewline   */
    int   suspected;   /* suspectedKeyword    */
} Token;

static Token *toks;
static int    ntoks, captoks;

/* ========================================================================== */
/* 2. CHARACTER CLASSES = the columns of the transition table                 */
/*    Every letter is its own class so keyword letters can be DFA transitions */
/* ========================================================================== */

enum {
    K_UPPER0 = 0,        /* 0..25  : A..Z */
    K_LOWER0 = 26,       /* 26..51 : a..z */
    K_DIGIT = 52, K_UNDERSCORE, K_DOT, K_EQ, K_BANG, K_LT, K_GT, K_SLASH,
    K_STAR, K_DQUOTE, K_SQUOTE, K_BACKSLASH, K_NEWLINE, K_SPACE, K_TAB,
    K_PLUS, K_MINUS, K_PERCENT, K_CARET,
    K_LPAREN, K_RPAREN, K_LBRACKET, K_RBRACKET, K_LBRACE, K_RBRACE,
    K_COMMA, K_SEMICOLON, K_COLON,
    K_OTHER,    /* other printable ASCII: legal only inside literals/comments */
    K_ILLEGAL,  /* control chars, lone CR, non-ASCII ...                      */
    K_EOF,      /* virtual class: end of input                                */
    NUM_CLASSES
};

static const char *CLASS_NAMES[] = {      /* names of classes K_DIGIT.. */
    "DIGIT", "UNDERSCORE", "DOT", "EQ", "BANG", "LT", "GT", "SLASH", "STAR",
    "DQUOTE", "SQUOTE", "BACKSLASH", "NEWLINE", "SPACE", "TAB",
    "PLUS", "MINUS", "PERCENT", "CARET",
    "LPAREN", "RPAREN", "LBRACKET", "RBRACKET", "LBRACE", "RBRACE",
    "COMMA", "SEMICOLON", "COLON", "OTHER", "ILLEGAL", "EOF"
};

static const char *class_name(int k)
{
    if (k < K_LOWER0) return "UPPER";
    if (k < K_DIGIT)  return "lower";
    return CLASS_NAMES[k - K_DIGIT];
}

static unsigned char CLS[256];            /* byte -> character class */

static void init_tables(void)
{
    int c;
    for (c = 0; c < 256; c++) CLS[c] = K_ILLEGAL;
    for (c = 33; c <= 126; c++) CLS[c] = K_OTHER;
    for (c = 'A'; c <= 'Z'; c++) CLS[c] = (unsigned char)(K_UPPER0 + c - 'A');
    for (c = 'a'; c <= 'z'; c++) CLS[c] = (unsigned char)(K_LOWER0 + c - 'a');
    for (c = '0'; c <= '9'; c++) CLS[c] = K_DIGIT;
    CLS['_'] = K_UNDERSCORE;  CLS['.'] = K_DOT;      CLS['='] = K_EQ;
    CLS['!'] = K_BANG;        CLS['<'] = K_LT;       CLS['>'] = K_GT;
    CLS['/'] = K_SLASH;       CLS['*'] = K_STAR;     CLS['"'] = K_DQUOTE;
    CLS['\''] = K_SQUOTE;     CLS['\\'] = K_BACKSLASH;
    CLS['\n'] = K_NEWLINE;    CLS[' '] = K_SPACE;    CLS['\t'] = K_TAB;
    CLS['+'] = K_PLUS;        CLS['-'] = K_MINUS;    CLS['%'] = K_PERCENT;
    CLS['^'] = K_CARET;
    CLS['('] = K_LPAREN;      CLS[')'] = K_RPAREN;
    CLS['['] = K_LBRACKET;    CLS[']'] = K_RBRACKET;
    CLS['{'] = K_LBRACE;      CLS['}'] = K_RBRACE;
    CLS[','] = K_COMMA;       CLS[';'] = K_SEMICOLON; CLS[':'] = K_COLON;
}

/* ========================================================================== */
/* 3. THE UNIFIED DFA:  M = (Q, Sigma, DELTA, q0, F)   -- ONE start state     */
/* ========================================================================== */

#define MAX_STATES 320
#define ACC_NO     (-1)     /* state is not accepting                         */
#define ACC_SKIP   (-2)     /* accepting, but the lexeme is discarded (blank) */

static short         DELTA[MAX_STATES][NUM_CLASSES];  /* -1 = implicit error state */
static short         ACC_TOK[MAX_STATES];             /* accepted token type       */
static unsigned char ACC_SUSP[MAX_STATES];            /* suspectedKeyword flag     */
static unsigned char COMMIT[MAX_STATES];              /* stopping here is an error:
                                                         1 string, 2 char, 3 block */
static char          STATE_NAME[MAX_STATES][14];
static int           nstates;

static int new_state(const char *name, int acc)
{
    int s, c, i = 0;
    if (nstates >= MAX_STATES) { fprintf(stderr, "too many DFA states\n"); exit(1); }
    s = nstates++;
    for (c = 0; c < NUM_CLASSES; c++) DELTA[s][c] = -1;
    ACC_TOK[s] = (short)acc;
    ACC_SUSP[s] = 0;
    COMMIT[s] = 0;
    for (; name[i] && i < 13; i++) STATE_NAME[s][i] = name[i];
    STATE_NAME[s][i] = '\0';
    return s;
}

static void on(int from, int cls, int to) { DELTA[from][cls] = (short)to; }

static void on_all(int from, int to)          /* every real character, not EOF */
{ int c; for (c = 0; c < K_EOF; c++) DELTA[from][c] = (short)to; }

static void on_letters(int from, int to)
{ int c; for (c = K_UPPER0; c < K_DIGIT; c++) DELTA[from][c] = (short)to; }

/* ---- keyword list (used ONLY to build states; never compared at run time) -- */

typedef struct { const char *word; int type; } KwDef;
static const KwDef KEYWORDS[] = {
    {"INTEGER", T_KW_INTEGER}, {"DECIMAL", T_KW_DECIMAL},
    {"CHARACTER", T_KW_CHARACTER}, {"BOOLEAN", T_KW_BOOLEAN},
    {"ARRAY", T_KW_ARRAY}, {"STRING", T_KW_STRING},
    {"IF", T_KW_IF}, {"ELSE", T_KW_ELSE}, {"WHILE", T_KW_WHILE},
    {"FOR", T_KW_FOR}, {"INPUT", T_KW_INPUT}, {"OUTPUT", T_KW_OUTPUT},
    {"TRUE", T_TRUE}, {"FALSE", T_FALSE},
    {"DIV", T_DIV}, {"AND", T_AND}, {"OR", T_OR}, {"NOT", T_NOT},
    {"ALGORITHM", T_RES_ALGORITHM}, {"RETURN", T_RES_RETURN},
    {"TRACE", T_RES_TRACE}, {"INVARIANT", T_RES_INVARIANT},
    {"SET", T_NOISE_SET}, {"THE", T_NOISE_THE},
    {0, 0}
};

/* one-character tokens: each gets its own accepting state hanging off q0 */
typedef struct { int cls; const char *name; int tok; } Punct;
static const Punct PUNCT[] = {
    {K_DOT, "DOT", T_DOT},             {K_PLUS, "PLUS", T_PLUS},
    {K_MINUS, "MINUS", T_MINUS},       {K_STAR, "STAR", T_STAR},
    {K_PERCENT, "PERCENT", T_PERCENT}, {K_CARET, "CARET", T_CARET},
    {K_LPAREN, "LPAREN", T_LPAREN},    {K_RPAREN, "RPAREN", T_RPAREN},
    {K_LBRACKET, "LBRACKET", T_LBRACKET}, {K_RBRACKET, "RBRACKET", T_RBRACKET},
    {K_LBRACE, "LBRACE", T_LBRACE},    {K_RBRACE, "RBRACE", T_RBRACE},
    {K_COMMA, "COMMA", T_COMMA},       {K_SEMICOLON, "SEMICOLON", T_SEMICOLON},
    {K_COLON, "COLON", T_COLON},
    {-1, 0, 0}
};

#define MAX_TRIE 160
static short TR[MAX_TRIE][26];        /* temporary keyword trie (build time)  */
static int   TR_TYPE[MAX_TRIE];       /* keyword token at that node, or -1    */
static char  TR_NAME[MAX_TRIE][12];   /* letters on the path to that node     */
static int   trie_n = 1;
static short E_STATE[MAX_TRIE];       /* "exact" copy of each trie node       */
static short F_STATE[MAX_TRIE];       /* "other letter case" copy of the node */

static void build_trie(void)
{
    int w, i, s, idx, len;
    for (s = 0; s < MAX_TRIE; s++) TR_TYPE[s] = -1;
    for (w = 0; KEYWORDS[w].word; w++) {
        s = 0;
        for (i = 0; KEYWORDS[w].word[i]; i++) {
            idx = KEYWORDS[w].word[i] - 'A';
            if (TR[s][idx] == 0) {
                TR[s][idx] = (short)trie_n;
                len = 0;
                while (TR_NAME[s][len]) { TR_NAME[trie_n][len] = TR_NAME[s][len]; len++; }
                TR_NAME[trie_n][len] = KEYWORDS[w].word[i];
                TR_NAME[trie_n][len + 1] = '\0';
                trie_n++;
            }
            s = TR[s][idx];
        }
        TR_TYPE[s] = KEYWORDS[w].type;
    }
}

static void build_dfa(void)
{
    int S0, ID, n, l, c, i;
    int INT, DOTS, DEC, BADN;
    int ASG, EQS, BNG, NEQS, LTS, LES, GTS, GES;
    int DIVS, LINE, BLK, BST, BEND;
    int STO, STD, CO, CB, CD, WS;
    char nm[14];

    S0 = new_state("START", ACC_NO);                 /* q0: the ONLY start state */

    /* ---- identifiers: q0 --letter--> ID, ID loops on letter/digit/_ -------- */
    ID = new_state("ID", T_IDENT);
    on_letters(ID, ID);
    on(ID, K_DIGIT, ID);
    on(ID, K_UNDERSCORE, ID);

    /* ---- keywords / noise words / reserved words, branching from q0 ---------
     * Each trie node n becomes TWO states:
     *   E:<letters>  the letters so far are UPPERCASE (exact spelling)
     *   F:<letters>  same letters but some were lowercase -> only a
     *                "suspectedKeyword" identifier, never a keyword
     * Both fall into ID as soon as the word stops matching a keyword prefix.  */
    build_trie();
    for (n = 1; n < trie_n; n++) {
        snprintf(nm, sizeof nm, "E:%s", TR_NAME[n]);
        E_STATE[n] = (short)new_state(nm, TR_TYPE[n] >= 0 ? TR_TYPE[n] : T_IDENT);
        snprintf(nm, sizeof nm, "F:%s", TR_NAME[n]);
        F_STATE[n] = (short)new_state(nm, T_IDENT);
        ACC_SUSP[F_STATE[n]] = (unsigned char)(TR_TYPE[n] >= 0);
    }
    for (n = 0; n < trie_n; n++) {
        int se = (n == 0) ? S0 : E_STATE[n];
        for (l = 0; l < 26; l++) {
            c = TR[n][l];
            on(se, K_UPPER0 + l, c ? E_STATE[c] : ID);
            on(se, K_LOWER0 + l, c ? F_STATE[c] : ID);
            if (n) {
                on(F_STATE[n], K_UPPER0 + l, c ? F_STATE[c] : ID);
                on(F_STATE[n], K_LOWER0 + l, c ? F_STATE[c] : ID);
            }
        }
        if (n) {
            on(se, K_DIGIT, ID);          on(se, K_UNDERSCORE, ID);
            on(F_STATE[n], K_DIGIT, ID);  on(F_STATE[n], K_UNDERSCORE, ID);
        }
    }

    /* ---- numbers: INT, DEC, and BAD_NUM for things like 2low / 1.5abc ------ */
    INT  = new_state("INT", T_INTEGER);
    DOTS = new_state("INT.", ACC_NO);
    DEC  = new_state("DEC", T_DECIMAL);
    BADN = new_state("BAD_NUM", T_INVALID);
    on(S0, K_DIGIT, INT);
    on(INT, K_DIGIT, INT);   on(INT, K_DOT, DOTS);
    on(DOTS, K_DIGIT, DEC);  on(DEC, K_DIGIT, DEC);
    on_letters(INT, BADN);   on(INT, K_UNDERSCORE, BADN);
    on_letters(DEC, BADN);   on(DEC, K_UNDERSCORE, BADN);
    on_letters(BADN, BADN);  on(BADN, K_DIGIT, BADN);  on(BADN, K_UNDERSCORE, BADN);

    /* ---- relational / assignment ------------------------------------------ */
    ASG  = new_state("ASSIGN", T_ASSIGN);
    EQS  = new_state("EQ", T_EQ);
    BNG  = new_state("BANG", ACC_NO);                /* lone '!' is not a token */
    NEQS = new_state("NEQ", T_NEQ);
    LTS  = new_state("LT", T_LT);   LES = new_state("LE", T_LE);
    GTS  = new_state("GT", T_GT);   GES = new_state("GE", T_GE);
    on(S0, K_EQ, ASG);   on(ASG, K_EQ, EQS);
    on(S0, K_BANG, BNG); on(BNG, K_EQ, NEQS);
    on(S0, K_LT, LTS);   on(LTS, K_EQ, LES);
    on(S0, K_GT, GTS);   on(GTS, K_EQ, GES);

    /* ---- division / line comment / block comment -------------------------- */
    DIVS = new_state("SLASH", T_SLASH);
    LINE = new_state("LINE_CMT", T_LINE_COMMENT);
    BLK  = new_state("BLK_CMT", ACC_NO);
    BST  = new_state("BLK_STAR", ACC_NO);
    BEND = new_state("BLK_END", T_BLOCK_COMMENT);
    on(S0, K_SLASH, DIVS);
    on(DIVS, K_SLASH, LINE);  on(DIVS, K_STAR, BLK);
    on_all(LINE, LINE);       on(LINE, K_NEWLINE, -1);
    on_all(BLK, BLK);         on(BLK, K_STAR, BST);
    on_all(BST, BLK);         on(BST, K_STAR, BST);  on(BST, K_SLASH, BEND);
    COMMIT[BLK] = COMMIT[BST] = 3;                   /* never closed = error */

    /* ---- string literal ---------------------------------------------------- */
    STO = new_state("STR_OPEN", ACC_NO);
    STD = new_state("STR_DONE", T_STRING);
    on(S0, K_DQUOTE, STO);
    on_all(STO, STO);
    on(STO, K_DQUOTE, STD);
    on(STO, K_BACKSLASH, -1); on(STO, K_NEWLINE, -1); on(STO, K_ILLEGAL, -1);
    COMMIT[STO] = 1;

    /* ---- character literal ------------------------------------------------- */
    CO = new_state("CHR_OPEN", ACC_NO);
    CB = new_state("CHR_BODY", ACC_NO);
    CD = new_state("CHR_DONE", T_CHAR);
    on(S0, K_SQUOTE, CO);
    on_all(CO, CB);
    on(CO, K_SQUOTE, -1); on(CO, K_BACKSLASH, -1); on(CO, K_NEWLINE, -1); on(CO, K_ILLEGAL, -1);
    on(CB, K_SQUOTE, CD);
    COMMIT[CO] = COMMIT[CB] = 2;

    /* ---- whitespace -------------------------------------------------------- */
    WS = new_state("WS", ACC_SKIP);
    on(S0, K_SPACE, WS);  on(S0, K_TAB, WS);  on(S0, K_NEWLINE, WS);
    on(WS, K_SPACE, WS);  on(WS, K_TAB, WS);  on(WS, K_NEWLINE, WS);

    /* ---- one-character operators and delimiters ---------------------------- */
    for (i = 0; PUNCT[i].cls >= 0; i++)
        on(S0, PUNCT[i].cls, new_state(PUNCT[i].name, PUNCT[i].tok));

    /* Anything else (e.g. '@', '_', '\') has no transition out of q0, so the
     * automaton goes to the implicit error state: an illegal character.      */
}

/* ========================================================================== */
/* 4. LEXER STATE, ERRORS, TOKEN EMISSION                                     */
/* ========================================================================== */

static int   trace_no;                  /* lexeme counter for the trace        */
static int   trace_on;                  /* 1 = print every DFA step            */
static FILE *g_out;                     /* where trace text is written         */

typedef struct {
    char *src;
    int   len, pos, line, col;
    int   pending_nl;      /* a line feed was passed since the last real token  */
    int   aux_nl;          /* a line feed was passed since the last token of any
                              kind (comments / invalid lexemes included)       */
} Lexer;

typedef struct { int line, col; char msg[160]; } LexError;
static LexError *errs;
static int nerrs, caperrs;

static void *xrealloc(void *p, size_t n)
{
    void *q = realloc(p, n);
    if (!q) { fprintf(stderr, "out of memory\n"); exit(1); }
    return q;
}

static void lex_error(int line, int col, const char *fmt, ...)
{
    va_list ap;
    if (nerrs == caperrs) {
        caperrs = caperrs ? caperrs * 2 : 16;
        errs = xrealloc(errs, (size_t)caperrs * sizeof(LexError));
    }
    errs[nerrs].line = line;
    errs[nerrs].col = col;
    va_start(ap, fmt);
    vsnprintf(errs[nerrs].msg, sizeof errs[nerrs].msg, fmt, ap);
    va_end(ap);
    if (trace_on)
        fprintf(g_out, "\n  !!! LEXICAL ERROR at %d:%d: %s\n", line, col, errs[nerrs].msg);
    nerrs++;
}

static int class_at(const Lexer *lx, int p)
{
    if (p < 0 || p >= lx->len) return K_EOF;
    return CLS[(unsigned char)lx->src[p]];
}

static void advance(Lexer *lx, int n)
{
    int i;
    for (i = 0; i < n && lx->pos < lx->len; i++) {
        if (lx->src[lx->pos] == '\n') { lx->line++; lx->col = 1; lx->pending_nl = lx->aux_nl = 1; }
        else lx->col++;
        lx->pos++;
    }
}

/* print a lexeme for the trace: escaped, cut at 40 characters */
static void trace_lexeme(const char *s)
{
    int i, n = 0;
    putc('"', g_out);
    for (i = 0; s[i]; i++) {
        unsigned char c = (unsigned char)s[i];
        if (n >= 40) { fprintf(g_out, "..."); break; }
        if (c == '\n')                { fprintf(g_out, "\\n"); n += 2; }
        else if (c == '\t')           { fprintf(g_out, "\\t"); n += 2; }
        else if (c < 32 || c > 126)   { putc('?', g_out); n++; }
        else                          { putc(c, g_out); n++; }
    }
    putc('"', g_out);
}

static void emit(Lexer *lx, int type, int start, int len, int line, int col, int suspected)
{
    Token *t;
    int i;
    if (ntoks == captoks) {
        captoks = captoks ? captoks * 2 : 64;
        toks = xrealloc(toks, (size_t)captoks * sizeof(Token));
    }
    t = &toks[ntoks++];
    t->lexeme = xrealloc(NULL, (size_t)len + 1);
    for (i = 0; i < len; i++) t->lexeme[i] = lx->src[start + i];
    t->lexeme[len] = '\0';
    t->type = type;
    t->line = line;
    t->col = col;
    t->nl_before = lx->pending_nl;
    t->suspected = suspected;
    lx->pending_nl = lx->aux_nl = 0;
    if (trace_on) {
        fprintf(g_out, "\n  >>> TOKEN EMITTED: %s  ", TOKEN_NAMES[type]);
        trace_lexeme(t->lexeme);
        fprintf(g_out, "  at %d:%d\n\n", line, col);
    }
}

/* Comments and invalid lexemes are reported as tokens too, but they must not
 * change the precededByNewline flag of the next real token. */
static void emit_aux(Lexer *lx, int type, int start, int len, int line, int col)
{
    int keep = lx->pending_nl, vis = lx->aux_nl;
    emit(lx, type, start, len, line, col, 0);
    toks[ntoks - 1].nl_before = vis;   /* newline since the previous visible token */
    lx->pending_nl = keep;             /* real tokens keep their original flag     */
    lx->aux_nl = 0;
}

/* ========================================================================== */
/* 5. DFA SIMULATION (transition-table lookup) + TRACE                        */
/* ========================================================================== */

typedef struct {
    int len;        /* length of the longest accepted prefix (0 = none)        */
    int acc_state;  /* accepting state of that prefix, -1 if none              */
    int stop_state; /* state the DFA was in when it stopped                    */
} Run;

static void trace_banner(const Lexer *lx)
{
    char buf[80];
    int ls = lx->pos, le = lx->pos, k, lim;

    trace_no++;
    snprintf(buf, sizeof buf, "LEXEME #%d   (line %d, col %d)", trace_no, lx->line, lx->col);
    fprintf(g_out, "+------------------------------------------------------------+\n");
    fprintf(g_out, "| %-58s |\n", buf);
    fprintf(g_out, "+------------------------------------------------------------+\n");

    while (ls > 0 && lx->src[ls - 1] != '\n') ls--;       /* start of this line */
    while (le < lx->len && lx->src[le] != '\n') le++;     /* end of this line   */
    lim = (le - ls > 70) ? ls + 70 : le;
    fprintf(g_out, "  source: ");
    for (k = ls; k < lim; k++) {
        int c = (unsigned char)lx->src[k];
        putc(c == '\t' ? ' ' : (c >= 32 && c <= 126 ? c : '?'), g_out);
    }
    fprintf(g_out, "\n");
    if (lx->pos - ls < 70) {
        fprintf(g_out, "          ");
        for (k = 0; k < lx->pos - ls; k++) putc(' ', g_out);
        fprintf(g_out, "^ the automaton starts here, in q0\n");
    }
    fprintf(g_out, "\n  step  from                reads   class       goes to             note\n");
    fprintf(g_out, "  ----  ------------------  ------  ----------  ------------------  ----------------\n");
}

static void char_repr(char *buf, int ch, int is_eof)
{
    if (is_eof)                     snprintf(buf, 12, "EOF");
    else if (ch == '\n')            snprintf(buf, 12, "\\n");
    else if (ch == '\t')            snprintf(buf, 12, "\\t");
    else if (ch == ' ')             snprintf(buf, 12, "' '");
    else if (ch >= 33 && ch <= 126) snprintf(buf, 12, "'%c'", ch);
    else                            snprintf(buf, 12, "#%d", ch);
}

/* Maximal munch: keep reading while DELTA has a transition, and remember the
 * last accepting position so the scanner can fall back to it. */
static Run dfa_run(const Lexer *lx)
{
    Run r;
    int s = 0, i = 0, nx, cls, ch, step = 0, first;
    int show;

    first = class_at(lx, lx->pos);
    show = trace_on && first != K_SPACE && first != K_TAB && first != K_NEWLINE;
    r.len = 0; r.acc_state = -1;
    if (show) trace_banner(lx);
    for (;;) {
        cls = class_at(lx, lx->pos + i);
        nx = DELTA[s][cls];
        if (show) {
            char cb[12], fb[24], tb[24], note[48];
            ch = (cls == K_EOF) ? 0 : (unsigned char)lx->src[lx->pos + i];
            char_repr(cb, ch, cls == K_EOF);
            snprintf(fb, sizeof fb, "q%d %s", s, STATE_NAME[s]);
            if (nx < 0) {
                snprintf(tb, sizeof tb, "(dead)");
                snprintf(note, sizeof note, "STOP");
            } else {
                snprintf(tb, sizeof tb, "q%d %s", nx, STATE_NAME[nx]);
                if (ACC_TOK[nx] >= 0)
                    snprintf(note, sizeof note, "ACCEPT: %s%s", TOKEN_NAMES[ACC_TOK[nx]],
                             ACC_SUSP[nx] ? " (suspected)" : "");
                else if (ACC_TOK[nx] == ACC_SKIP)
                    snprintf(note, sizeof note, "ACCEPT: (skip)");
                else
                    note[0] = '\0';
            }
            fprintf(g_out, "  %4d  %-18s  %-6s  %-10s  %-18s  %s\n",
                    ++step, fb, cb, class_name(cls), tb, note);
        }
        if (nx < 0) break;                       /* implicit error state */
        s = nx;
        i++;
        if (ACC_TOK[s] != ACC_NO) { r.len = i; r.acc_state = s; }
    }
    r.stop_state = s;
    if (show) {
        if (r.acc_state >= 0) {
            fprintf(g_out, "\n  result: longest match = %d char(s), last ACCEPT in q%d (%s)\n",
                    r.len, r.acc_state, STATE_NAME[r.acc_state]);
            if (i > r.len)
                fprintf(g_out, "  backtrack: went %d char(s) deep, but only the first %d count\n",
                        i, r.len);
        } else {
            fprintf(g_out, "\n  result: no ACCEPT state was ever reached\n");
        }
    }
    return r;
}

/* error recovery for quoted literals: consume through the closing quote, or
 * stop before a line feed / EOF. Returns total length consumed; *closed says
 * whether a closing quote was found. */
static int skip_quoted(const Lexer *lx, int quote_class, int *closed)
{
    int j = 1, c;
    for (;;) {
        c = class_at(lx, lx->pos + j);
        if (c == K_EOF || c == K_NEWLINE) { *closed = 0; return j; }
        if (c == quote_class) { *closed = 1; return j + 1; }
        j++;
    }
}

/* ========================================================================== */
/* 6. SCANNER: run the unified DFA from q0, then act on the accepting state   */
/* ========================================================================== */

static void scan(Lexer *lx)
{
    for (;;) {
        int line = lx->line, col = lx->col, start = lx->pos;
        Run r;
        int tok;

        if (class_at(lx, start) == K_EOF) {
            emit(lx, T_EOF, start, 0, line, col, 0);
            return;
        }
        r = dfa_run(lx);

        /* stopped inside an unterminated string / char / block comment */
        if (COMMIT[r.stop_state]) {
            int kind = COMMIT[r.stop_state], closed, n;
            if (kind == 3) {
                lex_error(line, col, "Unterminated block comment");
                n = lx->len - lx->pos;                 /* consume to EOF */
            } else if (kind == 1) {
                n = skip_quoted(lx, K_DQUOTE, &closed);
                if (closed) lex_error(line, col, "Invalid character inside string literal");
                else        lex_error(line, col, "Unterminated string literal");
            } else {
                n = skip_quoted(lx, K_SQUOTE, &closed);
                if (closed) lex_error(line, col, "Invalid character literal (need exactly one permitted character)");
                else        lex_error(line, col, "Unterminated character literal");
            }
            emit_aux(lx, T_INVALID, start, n, line, col);
            advance(lx, n);
            continue;
        }

        /* no accepting state: illegal standalone character or lone '!' */
        if (r.acc_state < 0) {
            int ch = (unsigned char)lx->src[start];
            if (ch == '!')
                lex_error(line, col, "Illegal character '!' (lone '!' is not an operator)");
            else if (ch >= 32 && ch <= 126)
                lex_error(line, col, "Illegal character '%c'", ch);
            else
                lex_error(line, col, "Illegal character (ASCII %d)", ch);
            emit_aux(lx, T_INVALID, start, 1, line, col);
            advance(lx, 1);
            continue;
        }

        tok = ACC_TOK[r.acc_state];                    /* token = f(accepting state) */
        switch (tok) {

        case ACC_SKIP:                                 /* whitespace */
            if (trace_on) fprintf(g_out, "  (whitespace x%d skipped, no token)\n\n", r.len);
            advance(lx, r.len);
            break;

        case T_INVALID:                                /* BAD_NUM: 2low, 1.5abc */
            lex_error(line, col, "Malformed number '%.*s'", r.len, lx->src + start);
            emit_aux(lx, T_INVALID, start, r.len, line, col);
            advance(lx, r.len);
            break;

        case T_INTEGER: {                              /* 32-bit range check */
            long v = 0;
            int k, bad = 0;
            for (k = 0; k < r.len; k++) {
                v = v * 10 + (lx->src[start + k] - '0');
                if (v > 2147483647L) { bad = 1; break; }
            }
            if (bad) {
                lex_error(line, col, "Integer literal '%.*s' out of range (max 2147483647)",
                          r.len, lx->src + start);
                emit_aux(lx, T_INVALID, start, r.len, line, col);
            } else emit(lx, T_INTEGER, start, r.len, line, col, 0);
            advance(lx, r.len);
            break;
        }

        case T_LINE_COMMENT:
        case T_BLOCK_COMMENT: {
            int k, ln = line, cl = col;
            emit_aux(lx, tok, start, r.len, line, col);
            for (k = 0; k < r.len; k++) {              /* check body characters */
                int c = class_at(lx, start + k);
                if (c == K_ILLEGAL)
                    lex_error(ln, cl, "Illegal character in comment (ASCII %d)",
                              (unsigned char)lx->src[start + k]);
                if (c == K_NEWLINE) { ln++; cl = 1; } else cl++;
            }
            advance(lx, r.len);
            break;
        }

        default:                                       /* every other token */
            emit(lx, tok, start, r.len, line, col, ACC_SUSP[r.acc_state]);
            advance(lx, r.len);
            break;
        }
    }
}

/* ========================================================================== */
/* 7. MAIN: input check, token table, lexical errors, Symbol Table file       */
/* ========================================================================== */

#define SYMTAB_FILE "Symbol Table.txt"

/* Only files carrying the LAlgo extension (.lal) may be read. The test is done
 * character by character (no string comparison). Case is ignored. */
static int has_lal_ext(const char *path)
{
    int len = 0;
    while (path[len]) len++;
    return len > 4 && path[len - 4] == '.' &&
           (path[len - 3] | 32) == 'l' &&
           (path[len - 2] | 32) == 'a' &&
           (path[len - 1] | 32) == 'l';
}

static int is_aux_token(int t)
{
    return t == T_INVALID || t == T_LINE_COMMENT || t == T_BLOCK_COMMENT;
}

/* Lexeme as it should be printed. Comments and invalid lexemes can span lines
 * or hold odd bytes, so they are escaped and cut to at most max characters
 * (buf must hold max+1). Every other token is printed unchanged. */
static const char *shown(const Token *t, char *buf, int max)
{
    int o = 0, i, k, el, cut = 0;
    char e[2];
    if (!is_aux_token(t->type)) return t->lexeme;
    for (i = 0; t->lexeme[i]; i++) {
        unsigned char c = (unsigned char)t->lexeme[i];
        e[0] = (char)c; e[1] = '\0'; el = 1;
        if (c == '\n')      { e[0] = '\\'; e[1] = 'n'; el = 2; }
        else if (c == '\t') { e[0] = '\\'; e[1] = 't'; el = 2; }
        else if (c < 32 || c > 126) e[0] = '?';
        if (o + el > max) { cut = 1; break; }
        for (k = 0; k < el; k++) buf[o++] = e[k];
    }
    if (cut) { o = max - 3; buf[o++] = '.'; buf[o++] = '.'; buf[o++] = '.'; }
    buf[o] = '\0';
    return buf;
}

int main(int argc, char **argv)
{
    FILE *f, *sym;
    Lexer lx;
    long n, r, w;
    int i, count, invalid;
    char buf[64], big[1100];

    if (argc < 2) {
        fprintf(stderr, "usage: %s <source.lal> [t]\n", argv[0]);
        return 2;
    }
    if (!has_lal_ext(argv[1])) {
        fprintf(stderr, "error: '%s' is not a LAlgo source file (expected a .lal file)\n", argv[1]);
        return 2;
    }
    trace_on = (argc >= 3 && argv[2][0] == 't');   /* 't' = show DFA trace */
    g_out = stdout;

    f = fopen(argv[1], "rb");
    if (!f) { fprintf(stderr, "cannot open %s\n", argv[1]); return 2; }
    fseek(f, 0, SEEK_END); n = ftell(f); fseek(f, 0, SEEK_SET);
    lx.src = xrealloc(NULL, (size_t)n + 1);
    n = (long)fread(lx.src, 1, (size_t)n, f);
    fclose(f);

    /* a plain-text source file never contains NUL bytes */
    for (r = 0; r < n; r++)
        if (lx.src[r] == '\0') {
            fprintf(stderr, "error: '%s' is not a plain text file\n", argv[1]);
            return 2;
        }

    /* CRLF -> LF (an isolated CR stays and is later reported as illegal) */
    for (r = w = 0; r < n; r++) {
        if (lx.src[r] == '\r' && r + 1 < n && lx.src[r + 1] == '\n') continue;
        lx.src[w++] = lx.src[r];
    }
    lx.len = (int)w; lx.src[w] = '\0';
    lx.pos = 0; lx.line = 1; lx.col = 1; lx.pending_nl = lx.aux_nl = 0;

    init_tables();
    build_dfa();

    if (trace_on) {
        printf("=== UNIFIED DFA TRACE ===\n");
        printf("ONE automaton with ONE start state (q0 START): %d states, %d character classes.\n",
               nstates, NUM_CLASSES);
        printf("Keywords, noise words, identifiers, numbers, operators, delimiters, strings,\n");
        printf("comments and whitespace ALL branch out of q0.\n\n");
        printf("How to read it:\n");
        printf("  * One row = one character read:  state --char--> next state.\n");
        printf("  * State names:  E:WHI = letters so far are an UPPERCASE keyword prefix,\n");
        printf("                  F:WHI = same prefix but with some lowercase (suspectedKeyword),\n");
        printf("                  ID = general identifier.\n");
        printf("  * ACCEPT: X = the text read so far is a valid token of type X; the scanner\n");
        printf("    remembers that spot.\n");
        printf("  * STOP = no transition. The scanner falls back to the LAST ACCEPT spot\n");
        printf("    (maximal munch / backtracking).\n");
        printf("  * '>>> TOKEN EMITTED' = the token this DFA run produced.\n\n");
    }
    scan(&lx);

    /* ---- token table ---- */
    printf("=== TOKENS ===\n");
    printf("%-8s %-14s %-24s %-8s %s\n", "line:col", "type", "lexeme", "newline", "suspectedKeyword");
    for (i = 0; i < ntoks; i++)
        printf("%3d:%-4d %-14s %-24s %-8s %s\n",
               toks[i].line, toks[i].col, TOKEN_NAMES[toks[i].type],
               shown(&toks[i], buf, 24), toks[i].nl_before ? "TRUE" : "FALSE",
               toks[i].suspected ? "TRUE" : "");

    /* ---- lexical errors ---- */
    printf("\n=== LEXICAL ERRORS: %d ===\n", nerrs);
    for (i = 0; i < nerrs; i++)
        printf("line %d, col %d: %s\n", errs[i].line, errs[i].col, errs[i].msg);

    /* ---- Symbol Table (plain-text output file) ---- */
    sym = fopen(SYMTAB_FILE, "w");
    if (!sym) { fprintf(stderr, "cannot write %s\n", SYMTAB_FILE); return 2; }
    fprintf(sym, "SYMBOL TABLE\nSource file: %s\n\n", argv[1]);
    fprintf(sym, "%-5s %-16s %-30s %-9s %s\n", "No.", "Token", "Classification", "Line:Col", "Lexeme");
    fprintf(sym, "%-5s %-16s %-30s %-9s %s\n", "---", "-----", "--------------", "--------", "------");
    count = invalid = 0;
    for (i = 0; i < ntoks; i++) {
        char pos[24];
        if (toks[i].type == T_EOF) continue;
        if (toks[i].type == T_INVALID) invalid++;
        snprintf(pos, sizeof pos, "%d:%d", toks[i].line, toks[i].col);
        /* Lexeme is the LAST column so even a long comment is written in full */
        fprintf(sym, "%-5d %-16s %-30s %-9s %s\n", ++count,
                TOKEN_NAMES[toks[i].type], TOKEN_CLASS[toks[i].type], pos,
                shown(&toks[i], big, 1000));
    }
    fprintf(sym, "\nTotal entries: %d (invalid: %d)\n", count, invalid);
    fclose(sym);
    printf("\nSymbol table written to %s (%d entries)\n", SYMTAB_FILE, count);

    return nerrs > 0 ? 1 : 0;
}

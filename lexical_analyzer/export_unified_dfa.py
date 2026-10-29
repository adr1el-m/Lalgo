"""Generate the paper's complete DFA appendix from the selected C implementation."""
from pathlib import Path
import json
import subprocess

project = Path(__file__).resolve().parents[1]
out = project / 'build/unified-paper'
out.mkdir(parents=True, exist_ok=True)
source = project / 'lexical_analyzer/lalgo_unified.c'
harness = out / 'dump_dfa.c'
harness.write_text('#define main lexer_main\n#include ' + json.dumps(str(source)) + r'''
#undef main
int main(void) {
    int s,c;
    init_tables(); build_dfa();
    printf("%d %d\n", nstates, NUM_CLASSES);
    for(s=0;s<nstates;s++) {
        printf("%d|%s|%s|%d|%d|",s,STATE_NAME[s],ACC_TOK[s]==ACC_NO?"NONE":ACC_TOK[s]==ACC_SKIP?"SKIP":TOKEN_NAMES[ACC_TOK[s]],ACC_SUSP[s],COMMIT[s]);
        for(c=0;c<NUM_CLASSES;c++) printf("%d,",DELTA[s][c]);
        printf("\n");
    }
}
''')
subprocess.run(['cc', '-std=c99', str(harness), '-o', str(out/'dump_dfa')], check=True)
data = subprocess.check_output([str(out/'dump_dfa')], text=True)
(out/'dfa-states.txt').write_text(data)
lines = data.splitlines()
count, classes = map(int, lines[0].split())
assert count == 254 and classes == 83
names = list('ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz') + ['DIGIT','UNDERSCORE','DOT','EQ','BANG','LT','GT','SLASH','STAR','DQUOTE','SQUOTE','BACKSLASH','NEWLINE','SPACE','TAB','PLUS','MINUS','PERCENT','CARET','LPAREN','RPAREN','LBRACKET','RBRACKET','LBRACE','RBRACE','COMMA','SEMICOLON','COLON','OTHER','ILLEGAL','EOF']
def esc(s):
    return ''.join({'_':r'\_', '&':r'\&', '%':r'\%', '#':r'\#', '{':r'\{', '}':r'\}'}.get(c,c) for c in s)
def compact(indices):
    result=[]
    for a,b,label in [(0,25,'A--Z'),(26,51,'a--z')]:
        selected=[i for i in indices if a<=i<=b]
        if len(selected)==26: result.append(label)
        elif len(selected)>15:
            result.append(label+' except '+','.join(names[i] for i in range(a,b+1) if i not in selected))
        elif selected:result.extend(names[i] for i in selected)
    result.extend(esc(names[i]) for i in indices if i>=52)
    return ', '.join(result)
rows=[]; startrows=[]
for line in lines[1:]:
    sid,name,acc,susp,commit,trans=line.split('|')
    targets=list(map(int,trans.rstrip(',').split(',')))
    groups={}
    for i,dest in enumerate(targets):
        if dest>=0: groups.setdefault(dest,[]).append(i)
    desc=[]
    for dest,indices in groups.items():
        if len(indices)>65:
            label='all real classes except '+', '.join(esc(names[i]) for i in range(82) if i not in indices)
            if len(indices)==82:label='all real classes'
        else: label=compact(indices)
        desc.append(label+r' $\to q_{'+str(dest)+'}$')
    if sid=='0':
        startrows=[r'$q_{'+str(dest)+r'}$ & '+compact(indices)+r' \\' for dest,indices in groups.items()]
        outgoing=r'See the start-state table above.'
    else:outgoing=r';\newline '.join(desc) if desc else r'None (all inputs stop the match).'
    action=esc(acc)
    if susp=='1': action+=r'; suspected'
    if commit!='0':action+=r'; committed '+{'1':'string','2':'character','3':'comment'}[commit]
    rows.append(r'$q_{'+sid+r'}$ '+r'\texttt{'+esc(name)+r'} & '+action+' & '+outgoing+r' \\')
text=r'''\clearpage
\section{Complete Unified DFA Transition Specification}\label{app:unified}
This appendix exhaustively specifies the transition table built by \codeinline{lalgo\_unified.c}: 254 explicit states and 83 input classes, including virtual EOF. It is generated from the implementation during the paper build. The overview in Section~\ref{sec:unified} and the smaller category diagrams are readable projections; the tables here supply every concrete state and defined transition.

Each uppercase and lowercase letter is a distinct class (52 classes). DIGIT denotes 0--9. The next 27 classes identify underscore, dot, assignment/equality, exclamation mark, comparisons, slashes, quotes, backslash, whitespace, arithmetic symbols, brackets and punctuation, in their named order. OTHER denotes remaining printable ASCII; ILLEGAL denotes unsupported bytes; EOF is virtual. The alphabet mapping follows the character-set section. All omitted entries are $\delta(q,c)=q_{\bot}$; the implicit non-accepting sink has $\delta(q_{\bot},c)=q_{\bot}$ for every class. Thus the mathematical transition function is total, although the implementation stores missing transitions as -1 and stops lookahead there.

NONE means non-accepting; SKIP is an accepting whitespace action; every token name labels an accepting state, including INVALID for malformed-number candidates. ``Suspected'' sets the capitalization diagnostic. ``Committed'' requires an error when scanning stops before a closing quote or block-comment terminator, even if a shorter prefix was accepted. EOF is emitted by the scanner when the source is exhausted, not by an accepting DFA state. Token emission and restarting at $q_0$ are scanner actions, not epsilon transitions.

\subsection{All Transitions from the Single Start State}
\begingroup\small
\begin{longtable}{L{23mm} L{115mm}}
\toprule Destination & Input classes from $q_0$ \\\midrule\endfirsthead
\toprule Destination & Input classes from $q_0$ \\\midrule\endhead
\bottomrule\endfoot
'''+ '\n'.join(startrows)+r'''
\end{longtable}\endgroup
\subsection{All States, Accepting Actions, and Remaining Transitions}
\begingroup\footnotesize
\setlength{\tabcolsep}{3pt}
\begin{longtable}{L{31mm} L{35mm} L{78mm}}
\toprule State / implementation name & Accepting action / metadata & Defined outgoing transitions \\\midrule\endfirsthead
\toprule State / implementation name & Accepting action / metadata & Defined outgoing transitions \\\midrule\endhead
\midrule\multicolumn{3}{r}{Continued on next page}\\\endfoot
\bottomrule\endlastfoot
'''+ '\n'.join(rows)+r'''
\end{longtable}\endgroup
'''
(out/'unified_dfa_tables.tex').write_text(text)
print(f'Exported {count} states and {classes} classes to the complete DFA appendix.')

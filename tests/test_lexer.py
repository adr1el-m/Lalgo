"""Black-box contract checks for the selected unified lexer; standard library only."""
from pathlib import Path
import argparse
import hashlib
import json
import re
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]
ROW = re.compile(r'^\s*(\d+):(\d+)\s+([A-Z_]+)\s+(.*?)\s+(TRUE|FALSE)\s*(TRUE)?$')


def run(binary, data, name='case.lal', trace=False):
    with tempfile.TemporaryDirectory(prefix='lalgo-test-') as directory:
        cwd = Path(directory)
        (cwd / name).write_bytes(data.encode('ascii') if isinstance(data, str) else data)
        proc = subprocess.run([str(binary), name] + (['t'] if trace else []),
                              cwd=cwd, text=True, capture_output=True)
        table = cwd / 'Symbol Table.txt'
        rows = []
        for line in proc.stdout.splitlines():
            match = ROW.match(line)
            if match:
                ln, col, kind, lexeme, nl, suspected = match.groups()
                rows.append((kind, lexeme, int(ln), int(col), nl == 'TRUE', bool(suspected)))
        return proc, rows, table.read_text() if table.exists() else None


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--binary', default='build/lalgo')
    args = parser.parse_args()
    binary = Path(args.binary).resolve()
    results = []

    def check(label, data, expected=None, code=0, errors=0, inspect=None, **kwargs):
        try:
            proc, rows, table = run(binary, data, **kwargs)
            assert proc.returncode == code, (proc.returncode, proc.stdout, proc.stderr)
            if code != 2:
                assert f'=== LEXICAL ERRORS: {errors} ===' in proc.stdout, proc.stdout
                assert rows and rows[-1][0] == 'EOF', rows
                assert table and 'End of File' not in table, table
            if expected is not None:
                assert [r[0] for r in rows] == expected + ['EOF'], rows
            if inspect:
                inspect(proc, rows, table)
            results.append({'case': label, 'passed': True})
            print(f'PASS {label}')
        except AssertionError as exc:
            results.append({'case': label, 'passed': False, 'detail': str(exc)})
            print(f'FAIL {label}: {exc}')

    def require(condition, detail):
        assert condition, detail

    words = 'INTEGER DECIMAL CHARACTER BOOLEAN ARRAY STRING IF ELSE WHILE FOR INPUT OUTPUT TRUE FALSE DIV AND OR NOT ALGORITHM RETURN TRACE INVARIANT SET THE'
    kinds = 'KW_INTEGER KW_DECIMAL KW_CHARACTER KW_BOOLEAN KW_ARRAY KW_STRING KW_IF KW_ELSE KW_WHILE KW_FOR KW_INPUT KW_OUTPUT TRUE FALSE DIV AND OR NOT RES_ALGORITHM RES_RETURN RES_TRACE RES_INVARIANT NOISE_SET NOISE_THE'.split()
    check('all 24 reserved spellings', words, kinds)
    check('case diagnostics and identifier suffixes', 'WHILE while While while1 WHILE_ WHIL',
          ['KW_WHILE'] + ['IDENT'] * 5,
          inspect=lambda p, r, t: require([x[5] for x in r[:-1]] == [False, True, True, False, False, False], r))
    check('identifier alphabet', 'a A a1 a_b Z9_', ['IDENT'] * 5)
    check('invalid identifier start', '_name', ['INVALID', 'IDENT'], code=1, errors=1)
    check('integer range and leading zeros', '0 0007 2147483647 2147483648',
          ['INTEGER'] * 3 + ['INVALID'], code=1, errors=1)
    check('decimal and dot backtracking', '3.14 3. .5 1..2',
          ['DECIMAL', 'INTEGER', 'DOT', 'DOT', 'INTEGER', 'INTEGER', 'DOT', 'DOT', 'INTEGER'])
    check('number identifier adjacency', '2low 1.5abc 7_x', ['INVALID'] * 3, code=1, errors=3)
    check('separate unary sign and exponent', '-2 ^ 3', ['MINUS', 'INTEGER', 'CARET', 'INTEGER'])
    check('symbolic operators and punctuation', '= == != < <= > >= + - * / % ^ ( ) [ ] { } , ; : .',
          'ASSIGN EQ NEQ LT LE GT GE PLUS MINUS STAR SLASH PERCENT CARET LPAREN RPAREN LBRACKET RBRACKET LBRACE RBRACE COMMA SEMICOLON COLON DOT'.split())
    check('lone exclamation recovery', '! x', ['INVALID', 'IDENT'], code=1, errors=1)
    check('strings empty and tab', '"" "a\tb"', ['STRING', 'STRING'])
    check('one character and tab', "'A' ' ' '\t'", ['CHAR'] * 3)
    check('bad character lengths', "'' 'ab' x", ['INVALID', 'INVALID', 'IDENT'], code=1, errors=2)
    check('backslash rejected in string', '"a\\b" x', ['INVALID', 'IDENT'], code=1, errors=1)
    check('unterminated string line recovery', '"abc\nx', ['INVALID', 'IDENT'], code=1, errors=1,
          inspect=lambda p, r, t: require(r[1][2:5] == (2, 1, True), r))
    check('comments retained', '// hi\n/* block */ x', ['LINE_COMMENT', 'BLOCK_COMMENT', 'IDENT'])
    check('unterminated block commits past slash', '/* open', ['INVALID'], code=1, errors=1)
    check('block comments do not nest', '/* outer /* inner */ x', ['BLOCK_COMMENT', 'IDENT'])
    check('newline preserved across auxiliary comment', 'x\n/* c */ y', ['IDENT', 'BLOCK_COMMENT', 'IDENT'],
          inspect=lambda p, r, t: require(r[2][4] is True, r))
    check('CRLF normalization and positions', b'x\r\n\ty', ['IDENT', 'IDENT'],
          inspect=lambda p, r, t: require(r[1][2:5] == (2, 2, True), r))
    check('isolated carriage return invalid', b'x\ry', ['IDENT', 'INVALID', 'IDENT'], code=1, errors=1)
    check('non ASCII comment diagnostic', b'//\xff\nx', ['LINE_COMMENT', 'IDENT'], code=1, errors=1)
    check('empty source EOF', '', [])
    check('wrong extension rejected', 'x', code=2, name='case.txt',
          inspect=lambda p, r, t: require(t is None and 'expected a .lal file' in p.stderr, p.stderr))
    check('uppercase extension accepted', 'x', ['IDENT'], name='case.LAL')
    check('NUL input rejected', b'x\x00y', code=2,
          inspect=lambda p, r, t: require(t is None and 'not a plain text file' in p.stderr, p.stderr))
    check('DFA trace dimensions', 'WHILE x', ['KW_WHILE', 'IDENT'], trace=True,
          inspect=lambda p, r, t: require('254 states, 83 character classes' in p.stdout and 'TOKEN EMITTED' in p.stdout, p.stdout))
    check('paper worked example exact stream', 'INTEGER low = 3.\n// boundary\nwhile low <= 5\n',
          'KW_INTEGER IDENT ASSIGN INTEGER DOT LINE_COMMENT IDENT IDENT LE INTEGER'.split(),
          inspect=lambda p, r, t: require(r == [
              ('KW_INTEGER', 'INTEGER', 1, 1, False, False),
              ('IDENT', 'low', 1, 9, False, False),
              ('ASSIGN', '=', 1, 13, False, False),
              ('INTEGER', '3', 1, 15, False, False),
              ('DOT', '.', 1, 16, False, False),
              ('LINE_COMMENT', '// boundary', 2, 1, True, False),
              ('IDENT', 'while', 3, 1, True, True),
              ('IDENT', 'low', 3, 7, False, False),
              ('LE', '<=', 3, 11, False, False),
              ('INTEGER', '5', 3, 14, False, False),
              ('EOF', '', 4, 1, True, False)], r))
    check('binary search sample', (ROOT / 'lexical_analyzer/sample.lal').read_bytes(),
          inspect=lambda p, r, t: require(len(r) == 100 and 'Total entries: 99 (invalid: 0)' in t, (len(r), t)))
    check('supplementary demo', (ROOT / 'lexical_analyzer/demo.lal').read_bytes())
    check('negative example seven errors', (ROOT / 'lexical_analyzer/bad.lal').read_bytes(), code=1, errors=7,
          inspect=lambda p, r, t: require('Total entries: 28 (invalid: 7)' in t, t))

    report = {'source': 'lexical_analyzer/lalgo_unified.c',
              'sha256': hashlib.sha256((ROOT / 'lexical_analyzer/lalgo_unified.c').read_bytes()).hexdigest(),
              'cases': len(results), 'passed': sum(r['passed'] for r in results), 'results': results}
    out = ROOT / 'build/tests'
    out.mkdir(parents=True, exist_ok=True)
    (out / 'results.json').write_text(json.dumps(report, indent=2) + '\n')
    (out / 'summary.tex').write_text(
        '\\newcommand{\\ValidationCases}{' + str(report['cases']) + '}\n' +
        '\\newcommand{\\ValidationPassed}{' + str(report['passed']) + '}\n')
    print(f"{report['passed']}/{report['cases']} cases passed")
    return 0 if report['passed'] == report['cases'] else 1


if __name__ == '__main__':
    raise SystemExit(main())

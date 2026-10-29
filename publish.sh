#!/usr/bin/env bash
set -euo pipefail

cd "$(dirname "$0")"

BUILD_ONLY=false
NO_PUSH=false

for arg in "$@"; do
  case "$arg" in
    --build-only) BUILD_ONLY=true ;;
    --no-push) NO_PUSH=true ;;
    *)
      echo "Usage: ./publish.sh [--build-only] [--no-push]" >&2
      exit 2
      ;;
  esac
done

mkdir -p build
make test
python3 lexical_analyzer/export_unified_dfa.py
latexmk -pdf -interaction=nonstopmode -halt-on-error -outdir=build lalgo_language_specification.tex
cp build/lalgo_language_specification.pdf LAlgo_Language_Design_Specification.pdf
latexmk -pdf -interaction=nonstopmode -halt-on-error -outdir=build lalgo_lexer_paper.tex
cp build/lalgo_lexer_paper.pdf LAlgo_Lexer_Paper.pdf

if $BUILD_ONLY; then
  exit 0
fi

node ../papers-site/generate-changelog.js CHANGELOG.md lalgo-language-design-specification

ARGS=(
  node ../papers-site/publish.js
  --kind paper
  --slug lalgo-language-design-specification
  --title "LAlgo: A Domain-Specific Programming Language Specification"
  --subtitle "Formal Syntax and Operational Lexicon"
  --author "Buenaventura, Harvey L.; De Mesa, Joshua C.; Dela Rosa, Mico Von Adriel A.; Magalona, Adriel M.; Morales, Timothy Gabriel; Nevado, Sean Andrew M.; Salvacion, Dean James G."
  --subject "Programming Languages"
  --tags "LAlgo,Programming Languages,Finite Automata,Lexical Analysis,Algorithms"
  --abstract "A lexical specification for LAlgo with one unified deterministic finite automaton, a complete 254-state transition appendix, token and error reporting, and Symbol Table output; parser and runtime extensions remain future work."
  --source LAlgo_Language_Design_Specification.pdf
  --yes
)

if ! $NO_PUSH; then
  ARGS+=(--push)
fi

"${ARGS[@]}"

LEXER_ARGS=(
  node ../papers-site/publish.js
  --kind paper
  --slug lalgo-lexer
  --title "LAlgo: Design and Validation of a Unified DFA Lexer"
  --subtitle "Table-driven lexical analysis in C99"
  --author "Buenaventura, Harvey L.; De Mesa, Joshua C.; Dela Rosa, Mico Von Adriel A.; Magalona, Adriel M.; Morales, Timothy Gabriel; Nevado, Sean Andrew M.; Salvacion, Dean James G."
  --subject "Programming Languages"
  --tags "LAlgo,Lexical Analysis,Finite Automata,C99,Compiler Design"
  --abstract "An implementation-focused account of LAlgo's 254-state unified DFA lexer, integrated keyword recognition, maximal munch, diagnostic recovery, token listing, and reproducible local validation."
  --source LAlgo_Lexer_Paper.pdf
  --yes
)
if ! $NO_PUSH; then
  LEXER_ARGS+=(--push)
fi
"${LEXER_ARGS[@]}"

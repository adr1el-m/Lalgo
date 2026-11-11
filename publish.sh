#!/usr/bin/env bash
set -euo pipefail

cd "$(dirname "$0")"

BUILD_ONLY=false
NO_PUSH=false
MEETING_GUIDE_ONLY=false

for arg in "$@"; do
  case "$arg" in
    --build-only) BUILD_ONLY=true ;;
    --no-push) NO_PUSH=true ;;
    --meeting-guide-only) MEETING_GUIDE_ONLY=true ;;
    *)
      echo "Usage: ./publish.sh [--build-only] [--no-push] [--meeting-guide-only]" >&2
      exit 2
      ;;
  esac
done

mkdir -p build
if $MEETING_GUIDE_ONLY; then
  make test
  mkdir -p build/meeting-guide
  latexmk -pdf -interaction=nonstopmode -halt-on-error -outdir=build/meeting-guide lalgo_team_guide.tex
  cp build/meeting-guide/lalgo_team_guide.pdf LAlgo_Team_Requirements_Guide.pdf
  if $BUILD_ONLY; then
    exit 0
  fi
  GUIDE_ARGS=(
    node "${PAPERS_SITE_DIR:-../papers-site}/publish.js"
    --kind paper
    --slug lalgo-team-requirements-guide
    --title "LAlgo: What We Need to Do"
    --subtitle "A quick guide for the team"
    --author "PUP COSC 303 - Group 4"
    --subject "Programming Languages"
    --tags "LAlgo,Programming Languages,Team Guide,Lexical Analysis,Syntax Analysis"
    --abstract "A concise guide to the 5 November 2026 meeting requirements, with ready and focus status labels, lexical/syntax coverage, the syntax and semantic checking tables, deadlines, and a team action plan."
    --source LAlgo_Team_Requirements_Guide.pdf
    --yes
  )
  if ! $NO_PUSH; then
    GUIDE_ARGS+=(--push)
  fi
  "${GUIDE_ARGS[@]}"
  exit 0
fi
make test
python3 lexical_analyzer/export_unified_dfa.py
python3 syntax_proposal/build_examples.py
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
  --subtitle "Lexical Specification and EBNF Syntax Proposal"
  --author "Buenaventura, Harvey L.; De Mesa, Joshua C.; Dela Rosa, Mico Von Adriel A.; Magalona, Adriel M.; Morales, Timothy Gabriel; Nevado, Sean Andrew M.; Salvacion, Dean James G."
  --subject "Programming Languages"
  --tags "LAlgo,Programming Languages,Finite Automata,Lexical Analysis,Algorithms"
  --abstract "LAlgo lexical specification and EBNF syntax proposal with a unified lexer, numbered examples, both derivations, one parse tree per example, and SWITCH selection."
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

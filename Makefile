CC = cc
CFLAGS = -std=c99 -Wall -Wextra -Wpedantic -Werror

SOURCE ?= syntax_proposal/switch_sample.lal

.PHONY: all lexer syntax test papers guide
all: lexer test

lexer: build/lalgo

build/lalgo: lexical_analyzer/lalgo_unified.c
	mkdir -p build
	$(CC) $(CFLAGS) $< -o $@

syntax: lexer
	python3 syntax_proposal/check_syntax.py "$(SOURCE)"

test: lexer
	python3 tests/test_lexer.py --binary build/lalgo
	python3 tests/test_syntax.py

papers:
	./publish.sh --build-only

guide:
	./publish.sh --meeting-guide-only --build-only

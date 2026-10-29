CC = cc
CFLAGS = -std=c99 -Wall -Wextra -Wpedantic -Werror

.PHONY: all lexer test papers
all: lexer test

lexer: build/lalgo

build/lalgo: lexical_analyzer/lalgo_unified.c
	mkdir -p build
	$(CC) $(CFLAGS) $< -o $@

test: lexer
	python3 tests/test_lexer.py --binary build/lalgo

papers:
	./publish.sh --build-only

# Makefile compil


# $@ : the current target
# $^ : the current prerequisites
# $< : the first current prerequisite

all: bin/compil clean_bin

symbolTable.o: lib/symbolTable/symbolTable.c lib/symbolTable/symbolTable.h
	gcc -c lib/symbolTable/symbolTable.c -o src/symbolTable.o -std=c99

abstract-tree.o: lib/tree/abstract-tree.c lib/tree/abstract-tree.h lib/symbolTable/symbolTable.h
	gcc -c lib/tree/abstract-tree.c -o src/abstract-tree.o -std=c99

error_check.o: lib/error_check/error_check.c lib/error_check/error_check.h
	gcc -c lib/error_check/error_check.c -o src/error_check.o -std=c99

translate.o: lib/translate/translate.c lib/translate/translate.h lib/error_check/error_check.h
	gcc -c lib/translate/translate.c -o src/translate.o -std=c99

parser.tab.c: src/parser.y
	bison --defines --debug src/parser.y -o src/parser.tab.c

lex.yy.c: src/lexer.lex
	flex -o src/lex.yy.c src/lexer.lex

bin/compil: lex.yy.c parser.tab.c symbolTable.o error_check.o abstract-tree.o translate.o
	gcc -o bin/compil src/lex.yy.c src/parser.tab.c src/parser.tab.h src/symbolTable.o src/error_check.o src/abstract-tree.o src/translate.o -ansi -pedantic -std=c99

clean_bin:
	rm src/*.[coh]

nasm:
	nasm -f elf64 -o test.o test.asm
	gcc -g -o nasm_final test.o -nostartfiles -no-pie
	rm test.o


clean:
	rm bin/compil

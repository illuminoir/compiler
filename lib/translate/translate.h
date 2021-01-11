#include "../error_check/error_check.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/* Generates NASM code to get the effective address of a variable.
 * Searches for the variable inside the symbol table and generates the NASM code
 * with its address if found, -1 otherwise. (meaning the identifier is a function name)
 * Parameters :
 *		SymbolTable *symtab : pointer to the current symbol table
 *		char* ident 		: identifier to search for
 * 		FILE* file 			: file to generate the code in */
void getAddrVar(SymbolTable *symtab, char* ident, FILE* file);

/* Returns the address of the given identifier.
 * Parameters :
 *		SymbolTable *symtab : pointer to the current symbol table
 *		char* ident 		: identifier to search for */
int getAddrParam(SymbolTable *symtab, char* ident);

/* updates the Neg global variable that allows the implementation
 * of the negation operator '!'.
 * Cycles the var_neg global variable between 1 and 0 with every call. */
void update_neg();

/* Writes the given text inside the given file.
 * Parameters :
 *		FILE* file : pointer to the file.
 *		char* text : text to write inside the file. */
void generate(FILE* file, char* text);

/* Translates the given code into NASM code by analyzing
 * the syntax tree.
 * Parameters :
 *		Node* node 			: the syntax tree's current node
 *		FILE* file 			: file in which to write the NASM code
 *		SymbolTable *symtab : pointer to the current symbolTable */
void translate(Node *node, FILE *file, SymbolTable *symtab);

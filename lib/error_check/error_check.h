/* Checks for the possible cases of error that are not checked
 * during parsing and the creation of the symbol table
 * During parsing : checks whether main function exists, lexical and 
 * 					grammatical errors
 * During creaiton of symbol table : checks for any undeclared identifier
 * Remaining cases : function is called with wrong types, function returns wrong type
 * 					or returns when void, pointer is used in operation
 					print on pointer = semantic error

 					warn only when wrong type is used for reade/readc */

#include "../tree/abstract-tree.h"
#include <stdio.h>
#include <string.h>

/* Returns whether the given identifier is a pointer or not.
 * Parameters :
 *		SymbolTable* symtab : pointer to the current symbol table
 *		char* ident 		: identifier to check
 * Returns : 1 if ident is a pointer, 0 otherwise. */
int identifierIsPointer(SymbolTable* symtab, char* ident);

/* Checks whether a pointer is used in the given arithmetic or comparison operation.
 * Parameters :
 *		Node* node 			: node of the operation
 *		SymbolTable* symtab : pointer to the current symbol table 
 * Returns : 1 if there is a pointer as operand of the operation 0 otherwise.*/
int checkForPointerInOp(Node* node, SymbolTable* symtab);

/* Checks whether a pointer is used in a call of the print() function.
 * Parameters :
 *		Node* node 			: node of the operation
 *		SymbolTable* symtab : pointer to the current symbol table 
 * Returns : 1 if there is a pointer as operand of the operation 0 otherwise.*/
int checkForPointerInPrint(Node* node, SymbolTable* symtab);

/* Returns the type of a given variable.
 * Parameters : 
 *		SymbolTable *symtab : pointer to the current symbol table
 *		char* ident 		: identifier to search for
 * Returns : the type of the variable, -1 if it doesn't exist (shouldn't happen)*/
int getTypeVar(SymbolTable *symtab, char *ident);

/* Returns the line number of a given variable.
 * Parameters : 
 *		SymbolTable *symtab : pointer to the current symbol table
 *		char* ident 		: identifier to search for
 * Returns : the type of the variable, -1 if it doesn't exist (shouldn't happen)*/
int getLinenoVar(SymbolTable *symtab, char *ident);

/* Checks that the program to be compiled has no error that would
 * prevent compilation to happen. Displays warnings and errors when they happen.
 * Parameters :
 *		Node* node 			: the current node to check (starts from the root)
 *		SymbolTable *symtab : pointer to the current symbol table */
void error_check(Node *node, SymbolTable *symtab);

/* Searches for the parameters in the SymbolTable which name is id, 
 * and fills types with the types of all the parameters. 
 * Parameters :
 *		char* id     : name of the Symboltable to look into
 *		int* types 	 : tab of integers, to be filled with the parameters' types 
 * Returns : the function's number of parameters */
int lookup_types(char* id, int* types, int* isPointer);

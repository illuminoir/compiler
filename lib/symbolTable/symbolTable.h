/* symbolTable.h */

#define INT 255
#define CHAR 256
#define VOID_TYPE 257
#define MAXNAME 128

typedef struct _listEntry {
	struct _listEntry *next; /* linked list to all our variables */
	char identifier[MAXNAME]; /* the variable's identifier */
	int type; /* the variable's type */
	int lineno;
	int isPointer;
	int isParam;
	int address;
	int isFunction; /* to verify the types in functions */
}ListEntry;

/* the symbol tables are linked from the bottom, they are linked with
 * parent link */
typedef struct _symbolTable {
	char name[MAXNAME];
	ListEntry *list; /* the list of variables in the current table */
	struct _symbolTable *parent; /* the table's parent */
	int counter; /*a counter of total bytes of all the variables in the symbolTable*/
	int isGlobal; /* differentiate global and local variables with a boolean */
}SymbolTable;


/* Create a list entry for our variables and initialize it with
 * the first given variable.
 * Parameters :
 		char* ident    : the first variable's identifier
 		int type 	   : the first variable's type 
		int isPointer  : boolean to assess whether the variable is a pointer
		int isParam	   : boolean to assess whether the variable is a function parameter
		int lineno	   : line at which the variable occurs
		int isFunction : boolean to assess whether the identifier is a function name
 * Returns : the created ListEntry object */
ListEntry *makeListEntry(SymbolTable* st, char* ident, int type, int isPointer, int isParam, int lineno, int isFunction);

/* Create a symbol table to store information on variables.
 * Parameters :
 		char* name : the symbol table's name
 * Returns : the created symbol table */
SymbolTable *makeSymbolTable(char* name);

/* Adds a variable into the given symbol table.
 * Parameters :
 		SymbolTable* st : the symbol table in which to add the variable.
 		char* ident 	: the first variable's identifier
 		int type 		: the first variable's type 
		int isPointer 	: boolean to assess whether the variable is a pointer
		int isParam		: boolean to assess whether the variable is a function parameter
		int lineno		: line at which the variable occurs
		int isFunction  : boolean to assess whether the identifier is a function name */
void addVar(SymbolTable* st, char* ident, int type, int isPointer, int isParam, int lineno, int isFunction);

/* Links the two symbol tables together, making st's parent parent.
 * Parameters :
 		SymbolTable* st 	: the current symbol table
 		SymbolTable* parent : st's new parent */
void addParentTable(SymbolTable* st, SymbolTable* parent);

/* Displays the entire symbol table starting from its root.
 * Parameters :
 		SymbolTable* st : the symbol table to display. */
void printSymbolTable(SymbolTable* st);

/* Searches for the declaration of a variable named ident inside the symbol table.
 * Parameters :
 		SymbolTable* st : the symbol table to search into
 		char* ident 	: the name of the variable to search for
 		int lineno		: line at which the variable occurs 
 * Returns : 1 if the variable was found, 0 otherwise. */
int lookup(SymbolTable* st, char* ident, int lineno);





#include <stdio.h>
#include "symbolTable.h"
#include <string.h>
#include <stdlib.h>

extern int err;


ListEntry *makeListEntry(SymbolTable* st, char* ident, int type, int isPointer, int isParam, int lineno, int isFunction){
	ListEntry* tmp = malloc(sizeof(ListEntry));

	if(!tmp){
		fprintf(stderr, "Run out of memory\n");
		exit(1);
	}

	/* initialize the list of variable entries in our symbol table */
	strcpy(tmp->identifier, ident);

	tmp->type = type;
	tmp->isPointer = isPointer;
	tmp->isParam = isParam;
	tmp->lineno = lineno;
	tmp->isFunction = isFunction;
	/**/
	tmp->address = (isFunction ? -1 : st->counter);
	tmp->next = NULL;

	return tmp;
}

SymbolTable *makeSymbolTable(char* name){
	SymbolTable* tmp = malloc(sizeof(SymbolTable));

	if (!tmp) {
    	fprintf(stderr, "Run out of memory\n");
    	exit(1);
	}

	/*initialize the name of the corresponding scope*/
	strcpy(tmp->name, name); 

	if(!strcmp(name, "Global"))
		tmp->isGlobal = 1;
	else
		tmp->isGlobal = 0;

	/* initialize the symbol table with one variable */
	tmp->list = NULL;
	tmp->parent = NULL;
	tmp->counter = 0;

	return tmp;
}


void addVar(SymbolTable* st, char* ident, int type, int isPointer, int isParam, int lineno, int isFunction){


	/* if we add the first variable to our current table */
	if(!(st->list)){
		st->list = makeListEntry(st, ident, type, isPointer, isParam, lineno, isFunction);
		if (!isFunction)
			st->counter += 8;/*(isPointer ? 8 : (type == INT ? 4 : 1));*/
		return;
	}

	ListEntry* tmp = st->list;

	/* if the variable is already at the root of the table */
	if(!strcmp(tmp->identifier, ident)){
		fprintf(stderr, "semantic error, redefinition of variable %s near line %d\n", ident, lineno);
		return;
	}

	/* otherwise, search for it */
	while(tmp->next){
		tmp = tmp->next;

		/* variable is inside the table */
		if(!strcmp(tmp->identifier, ident)){
			fprintf(stderr, "semantic error, redefinition of variable %s near line %d\n", ident, lineno);
			return;
		}
	}

	/* the variable was not found so we add it to our symbol table */
	ListEntry* var = makeListEntry(st, ident, type, isPointer, isParam, lineno, isFunction);

	if (!isFunction)
		st->counter += 8; /*(isPointer ? 8 : (type == INT ? 4 : 1));*/

	tmp->next = var;
}

void addParentTable(SymbolTable* st, SymbolTable* parent){
	st->parent = parent;
}

void printSymbolTable(SymbolTable* st){
    SymbolTable* tmp = st;
	
    ListEntry* tmp_le = st->list;

    /* print all variables from the current table */
    while(tmp_le){
        printf("Scope : %10s | ID : %10s | Type : %4s | IsPointer : %3s | IsParam : %3s | IsFunction : %3s | lineno : %3d | address : %d\n" ,st->name, tmp_le->identifier, tmp_le->type == INT ? "Int" : (tmp_le->type == CHAR ? "Char" : "Void"), tmp_le->isPointer ? "Yes" : "No", tmp_le->isParam ? "Yes" : "No", tmp_le->isFunction ? "Yes" : "No", tmp_le->lineno, tmp_le->address);
        tmp_le = tmp_le->next;
    }
}

int lookup(SymbolTable* st, char* ident, int lineno){
    SymbolTable tmp = *st;
    SymbolTable* ptr = &tmp;
    SymbolTable* glob = tmp.parent;
    
    /* search in the current symbol table */
    /* search its list */
    while(ptr->list){
        if(!strcmp(ident, ptr->list->identifier)){
            return 1;
        }
        ptr->list = ptr->list->next;
    }
    /* if it was not found, look up inside the parent table */
    if((strcmp("Global", ptr->name))){
        if(lookup(glob, ident, lineno))
            return 1;
        else
            return 0;
    }
    fprintf(stderr, "line %d : error : %s undeclared\n", lineno, ident); 
    err = 1;
    return 0;
}

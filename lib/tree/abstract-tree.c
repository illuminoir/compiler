/* abstract-tree.c */
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "abstract-tree.h"


extern int lineno;  /* from lexer */


static const char *StringFromKind[] = {
  "Program",      //0
  "VarDeclList",  //1
  "VarDec",       //2
  "FuncDecList",  //3
  "FuncDec",//4
  "Pointer",//5
  "Deref",
  "ParamList",//6
  "Param",//7
  "Body",//8
  "StmtList",//9
  "Assign",//10
  "Return",//11
  "IntLiteral",//12
  "CharLiteral",//13
  "Identifier",//14
  "Int",//15
  "Char",//16
  "Reade",//17
  "Readc",//18
  "Print",//19
  "IfThen",//20
  "IfThenElse",//21
  "While",//22
  "FuncCall",//23
  "Arguments",//24
  "Add",//25
  "Sub",//26
  "Or",//27
  "And",//28
  "Eq",//29
  "Diff", //30
  "Mult",//31
  "Div",
  "Modulo",
  "Neg",
  "Inf",
  "Sup",
  "InfEq",
  "SupEq",
  "ValueOf",
  "AddressOf"
  /* and all other node labels */
  /* The list must coincide with the enum in abstract-tree.h */
  /* To avoid listing them twice, see https://stackoverflow.com/a/10966395 */
};

Node *makeNode(Kind kind) {
  Node *node = malloc(sizeof(Node));
  if (!node) {
    printf("Run out of memory\n");
    exit(1);
  }
  node->kind = kind;
  node->firstChild = node->nextSibling = NULL;
  node->lineno = lineno;
  node->st = NULL;
  return node;
}

void addSibling(Node *node, Node *sibling) {
  Node *curr = node;
  while (curr->nextSibling != NULL) {
    curr = curr->nextSibling;
  }
  curr->nextSibling = sibling;
}

void addChild(Node *parent, Node *child) {
  if (parent->firstChild == NULL) {
    parent->firstChild = child;
  }
  else {
    addSibling(parent->firstChild, child);
  }
}

void deleteTree(Node *node) {
  if (node->firstChild) {
    deleteTree(node->firstChild);
  }
  if (node->nextSibling) {
    deleteTree(node->nextSibling);
  }
  free(node);
}

void printTree(Node *node) {
  static bool rightmost[128]; /* current node is rightmost sibling */
  static int depth = 0;       /* depth of current node */
  for (int i = 1; i < depth; i++) { /* 2502 = vertical line */
    printf(rightmost[i] ? "    " : "\u2502   ");
  }
  if (depth > 0) { /* 2514 = up and right; 2500 = horiz; 251c = vertical and right */ 
    printf(rightmost[depth] ? "\u2514\u2500\u2500 " : "\u251c\u2500\u2500 ");
  }
  printf("%s", StringFromKind[node->kind]);
  switch (node->kind) {
  case IntLiteral: printf(": %d", node->u.integer); break;
  case CharLiteral: printf(": '%c'", node->u.character); break;
  case Identifier: printf(": %s", node->u.identifier); break;
  default: break;
  }
  printf("\n");
  depth++;
  for (Node *child = node->firstChild; child != NULL; child = child->nextSibling) {
    rightmost[depth] = (child->nextSibling) ? false : true;
    printTree(child);
  }
  depth--;
}

void fillTable(SymbolTable *st, Node* node){

    if (!node)
        return;
  
    //printf("st : %s, node : %d\n", st->name, node->kind);
    static SymbolTable* parent = NULL;
    static SymbolTable* scope = NULL;

    Node* ntype;
    Node* vardec;
    Node* id;

    int itype;
    int isPointer;
    int funcReturnType;

    switch (node->kind) {
        /*creates a symbolTable for the global variables*/
        case Program: 
            parent = st;
            scope = st;
            node->st = st;
            break;
      
        /*creates a symbolTable for the new function declarations*/  
        case FuncDec:
            /* if the function's return type is VOID, the identifier is on the first child */
            /* otherwise it is on the second, the first being the return type instead. */
        	if (FIRSTCHILD(node)->kind == Identifier){
            funcReturnType = VOID_TYPE;
        		id = FIRSTCHILD(node);
          }
        	else	{
            funcReturnType = FIRSTCHILD(node)->kind == Int ? INT : CHAR;
            id = SECONDCHILD(node);
          }
            scope = makeSymbolTable(id->u.identifier);
            addParentTable(scope, parent);
            node->st = scope;
            if (!strcmp(st->name, "Global")) 
            	addVar(st, id->u.identifier, funcReturnType, 0, 0, id->lineno, 1);
            else
            	addVar(st->parent, id->u.identifier, funcReturnType, 0, 0, id->lineno, 1);  

    	
            break;
          
        /*adds variables according to their declaration*/
        case VarDeclList: 
            if (node->firstChild){
                ntype = node->firstChild;
                while (ntype){
                    itype = (ntype->kind == Int ? INT : CHAR);		  
                    vardec = ntype->firstChild;
                    /* while we have variables to search for */
                    while(vardec){
                        if (FIRSTCHILD(vardec)->kind == Pointer){
                            id = SECONDCHILD(vardec);
                            isPointer = 1;
                        }
                        else {
                            id = FIRSTCHILD(vardec);
                            isPointer = 0;
                        }
                        addVar(st, id->u.identifier, itype, isPointer, 0, vardec->lineno, 0);
                        vardec = vardec->nextSibling;
                    }
                    ntype = ntype->nextSibling;
                }
            }
            break;
        
        case Param:
            if (node->firstChild){
                if (FIRSTCHILD(node)->kind == Pointer){
                    isPointer = 1;
                    ntype = FIRSTCHILD(node)->firstChild;
              	}
              	else{
                    isPointer = 0;
                    ntype = FIRSTCHILD(node);
              	}
              	itype = (ntype->kind == Int ? INT : CHAR);
              	id = SECONDCHILD(node);
              	addVar(st, id->u.identifier, itype, isPointer, 1, node->lineno, 0);
            }
            break;
            
        case Identifier:
            /* if we meet an identifier that is not in a declaration node,
               it is one that must be defined */
            lookup(st, node->u.identifier, node->lineno);
            break;
          
        default:
            break;
    }
    if (node -> firstChild)
        fillTable(scope, node->firstChild);
    if (node -> nextSibling)	
      	fillTable(scope, node->nextSibling); 
}


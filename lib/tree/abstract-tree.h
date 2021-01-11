/* abstract-tree.h */

#include "../symbolTable/symbolTable.h"

typedef enum {
  Program,
  VarDeclList,
  VarDec,
  FuncDecList,
  FuncDec,
  Pointer,
  Deref,
  ParamList,
  Param,
  Body,
  StmtList,
  Assign,
  Return,
  IntLiteral,
  CharLiteral,
  Identifier,
  Int,
  Char,
  Reade,
  Readc,
  Print,
  IfThen,
  IfThenElse,
  While,
  FuncCall,
  Arguments,
  Add,
  Sub,
  Or,
  And,
  Eq,
  Diff,
  Mult,
  Div,
  Modulo,
  Neg,
  Inf,
  Sup,
  InfEq,
  SupEq,
  ValueOf,
  AddressOf
  /* and allother node labels */
  /* The list must coincide with the strings in abstract-tree.c */
  /* To avoid listing them twice, see https://stackoverflow.com/a/10966395 */
} Kind;

typedef struct Node {
  Kind kind;
  union {
    int integer;
    char character;
    char identifier[64];
  } u;
  struct Node *firstChild, *nextSibling;
  int lineno;
  SymbolTable* st;
} Node;

Node *makeNode(Kind kind);
void addSibling(Node *node, Node *sibling);
void addChild(Node *parent, Node *child);
void deleteTree(Node*node);
void printTree(Node *node);

#define FIRSTCHILD(node) node->firstChild
#define SECONDCHILD(node) node->firstChild->nextSibling
#define THIRDCHILD(node) node->firstChild->nextSibling->nextSibling
#define FOURTHCHILD(node) node->firstChild->nextSibling->nextSibling->nextSibling


/*  Creates then fills the symbolTable, according to the given Node
 *  While created, the symbol table is also printed on the standard output.
 *  Also looks for undefined variable that were called in the program.
 *  Parameters :
      SymbolTable* st : the symbol table
      Node* node      : the tree to be browsed */
void fillTable(SymbolTable *st, Node* node);

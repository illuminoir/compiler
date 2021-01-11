%{
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "../lib/translate/translate.h"

void yyerror(char *);
int yylex();


struct Node* root; //root of the tree

extern int lineno;
extern int charno;
extern int is_lexical_error;
extern FILE* yyin;
int err = 0; /* error code for undeclared variables */
int has_main = 0; /* boolean to check for the existence of main function */

%}


%union {
    int num; 
    char character;
    char comp[3];
    char addsub;
    char type[16];
    char ident[64];
    struct Node* node;
}

%token<character> CHARACTER
%token<type> TYPE
%token<comp> EQ DIFF INF SUP INFEQ SUPEQ 
%token<addsub> ADDSUB
%token<ident> IDENT
%token<num> NUM
%token READE READC PRINT IF ELSE WHILE RETURN VOID OR AND

%expect 1

%type<node> Prog DeclVars Declarateurs DeclFoncts DeclFonct
%type<node> EnTeteFonct Parametres ListTypVar Corps
%type<node> SuiteInstr Instr Exp TB FB M E T F LValue Arguments ListExp


%%
Prog:  DeclVars DeclFoncts               {
                                            $$ = makeNode (Program); 
                                            addChild($$, $1);
                                            addChild($$, $2);
                                            root = $$; //store the tree in our global variable
                                         }
    ;
DeclVars:
       DeclVars TYPE Declarateurs ';'    {
                                            Node* tmp;

                                            $$ = $1;
                                            if(!strcmp($2, "Int")) //if the type is int
                                                tmp = makeNode(Int);
                                            if(!strcmp($2, "Char")) //otherwise the type is char
                                                tmp = makeNode(Char);
                                            addChild($$, tmp); //and link all this to the tree
                                            addChild(tmp, $3); //add the other variables as children
                                         }
    |                                    { $$ = makeNode(VarDeclList); }  
    ;
Declarateurs:
       Declarateurs ',' IDENT           {
                                            $$ = $1;

                                            Node* var = makeNode(VarDec);
                                            Node* tmp = makeNode(Identifier);
                                            strcpy(tmp->u.identifier, $3); //get the identifier from the lexer
                                            addChild(var, tmp);
                                            addSibling($$, var);
                                        }
    |  Declarateurs ',' '*' IDENT       {
                                            $$ = makeNode(VarDec);

                                            addSibling($$, $1);
                                            addChild($$, makeNode(Pointer));
                                            Node* tmp = makeNode(Identifier);
                                            strcpy(tmp->u.identifier, $4); //get the identifier from the lexer
                                            addChild($$, tmp);
                                        }
    |  IDENT                            {
                                            $$ = makeNode(VarDec);

                                            Node* tmp = makeNode(Identifier); 
                                            strcpy(tmp->u.identifier, $1); //get the identifier from the lexer
                                            addChild($$, tmp);
                                        }
    |  '*' IDENT                        {
                                            $$ = makeNode(VarDec);

                                            //for a Pointer, make it a child and the identifier is the pointer's child
                                            addChild($$, makeNode(Pointer));
                                            Node* tmp = makeNode(Identifier);
                                            strcpy(tmp->u.identifier, $2); //get the identifier from the lexer
                                            addChild($$, tmp);
                                        }
    ;
DeclFoncts:
       DeclFoncts DeclFonct             {
                                            $$ = $1;
                                            addChild($$, $2);
                                        }
    |  DeclFonct                        {   $$ = makeNode(FuncDecList);

                                            addChild($$, $1);
                                        }
    ;
DeclFonct:
       EnTeteFonct Corps                {
                                            $$ = makeNode(FuncDec);
                                            addChild($$, $1);
                                            Node* tmp;
                                            tmp = makeNode(Body);
                                            addChild(tmp, $2);
                                            addChild($$, tmp);
                                        }
    ;
EnTeteFonct:
       TYPE IDENT '(' Parametres ')'    {
                                            if(!strcmp($1, "Int")) //if the type is int
                                                $$ = makeNode(Int);
                                            if(!strcmp($1, "Char")) //otherwise the type is char
                                                $$ = makeNode(Char);

                                            Node* tmp = makeNode(Identifier);
                                            strcpy(tmp->u.identifier, $2); //get the identifier from the lexer

                                            //if the function main exists within the code
                                            if(!strcmp($2, "main"))
                                                has_main = 1;

                                            addSibling($$, tmp);
                                            addSibling($$, $4);
                                        }
    |  TYPE '*' IDENT '(' Parametres ')'{
                                            $$ = makeNode(Pointer);

                                            if(!strcmp($1, "Int")) //if the type is int
                                                addChild($$, makeNode(Int));
                                            if(!strcmp($1, "Char")) //otherwise the type is char
                                                addChild($$, makeNode(Char));

                                            Node* tmp = makeNode(Identifier);
                                            strcpy(tmp->u.identifier, $3); //get the identifier from the lexer
                                            addSibling($$, tmp);
                                            addSibling($$, $5);
                                        }
    |  VOID IDENT '(' Parametres ')'    {
                                            Node* tmp = makeNode(Identifier);
                                            strcpy(tmp->u.identifier, $2); //get the identifier from the lexer
                                            $$ = tmp;
                                            addSibling($$, $4);

                                        }
    ;
Parametres:
       VOID                             { $$ = makeNode(ParamList); }
    |  ListTypVar                       {
                                            $$ = makeNode(ParamList);
                                            addChild($$, $1);
                                        }
    ;
ListTypVar:
       ListTypVar ',' TYPE IDENT        {
                                            $$ = $1;
                       									    Node* tmp;
                       									    if (!strcmp($3, "Int"))
                       									        tmp = makeNode(Int);
                       									    if (!strcmp($3, "Char"))
                       									        tmp = makeNode(Char);
                       									    Node* id = makeNode(Identifier);
                       									    strcpy(id->u.identifier, $4);
                       									    Node* param = makeNode(Param);
                       									    addChild(param, tmp);
                       									    addChild(param, id);
                       									    addSibling($$, param);
                       									}
       
    |  ListTypVar ',' TYPE '*' IDENT    {
                                            $$ = $1;
                       									    Node* tmp;
                       									    if (!strcmp($3, "Int"))
                       									        tmp = makeNode(Int);
                       									    if (!strcmp($3, "Char"))
                       									        tmp = makeNode(Char);
                       									    Node* id = makeNode(Identifier);
                       									    strcpy(id->u.identifier, $5);
                       									    Node* ptr = makeNode(Pointer);
                       									    Node* param = makeNode(Param);
                       									    addChild(ptr, tmp);
                       									    addChild(param, ptr);
                       									    addChild(param, id);
                       									    addSibling($$, param);
                       									}
       									    									 
    
    |  TYPE IDENT                       {
                                            $$ = makeNode(Param);
                       									    Node* tmp;
                       									    if (!strcmp($1, "Int"))
                       									        tmp = makeNode(Int);
                       									    if (!strcmp($1, "Char"))
                       									        tmp = makeNode(Char);
                       									    addChild($$, tmp);
                       									    Node* id = makeNode(Identifier);
                       									    strcpy(id->u.identifier, $2);
                       									    addChild($$, id);    
                    									  }
    
    |  TYPE '*' IDENT                   {   
                                            $$ = makeNode(Param);
                      									    Node* ptr = makeNode(Pointer);
                         									    Node* tmp;
                         									    if (!strcmp($1, "Int"))
                         									        tmp = makeNode(Int);
                         									    if (!strcmp($1, "Char"))
                         									        tmp = makeNode(Char);
                         									    addChild($$, ptr);
                         									    addChild(ptr, tmp);
                         									    Node* id = makeNode(Identifier);
                         									    strcpy(id->u.identifier, $3);
                         									    addChild($$, id);  
                         								}
    ;
Corps: '{' DeclVars SuiteInstr '}'      {
                                            $$ = $2;
                                            addSibling($$, $3);
                                        }
    ;
SuiteInstr:
       SuiteInstr Instr                 {
                                            $$ = $1;
                                            addChild($$, $2);
                                        }
    |                                   { $$ = makeNode(StmtList); }  
    ;
Instr:
       LValue '=' Exp ';'               { 
                                            $$ = makeNode(Assign); 
                                            addChild($$, $1);
                                            addChild($$, $3);
                                        }
    |  READE '(' IDENT ')' ';'          {
                                            $$ = makeNode(Reade);
                                            Node* value = makeNode(AddressOf);
                                            addChild($$, value);
                                            Node* tmp = makeNode(Identifier);
                                            strcpy(tmp->u.identifier, $3); //get the identifier from the lexer
                                            addChild(value, tmp);
                                        }
    |  READC '(' IDENT ')' ';'          {
                                            $$ = makeNode(Readc);
                                            Node* value = makeNode(AddressOf);
                                            addChild($$, value);
                                            Node* tmp = makeNode(Identifier);
                                            strcpy(tmp->u.identifier, $3); //get the identifier from the lexer
                                            addChild(value, tmp);  
                                        }
    |  PRINT '(' Exp ')' ';'            {
                                            $$ = makeNode(Print);
                                            addChild($$, $3);
                                        }
    |  IF '(' Exp ')' Instr             {
                                            $$ = makeNode(IfThen);
                                            addChild($$, $3);
                                            addChild($$, $5);
                                        }
    |  IF '(' Exp ')' Instr ELSE Instr  {
                                            $$ = makeNode(IfThenElse);
                                            addChild($$, $3);
                                            addChild($$, $5);
                                            addChild($$, $7);
                                        }
    |  WHILE '(' Exp ')' Instr          {
                                            $$ = makeNode(While);
                                            addChild($$, $3);
                                            addChild($$, $5);
                                        }
    |  IDENT '(' Arguments  ')' ';'     {
                                            $$ = makeNode(FuncCall);

                                            Node* tmp = makeNode(Identifier);
                                            strcpy(tmp->u.identifier, $1); //get the identifier from the lexer
                                            addChild($$, tmp);

                                            addChild($$, $3);
                                        }
    |  RETURN Exp ';'                   {
                                            $$ = makeNode(Return);
                                            addChild($$, $2);
                                        }
    |  RETURN ';'                       { $$ = makeNode(Return); }
    |  '{' SuiteInstr '}'               { $$ = $2; }
    |  ';'                              {}
    ;
Exp :  Exp OR TB                        { 
                                            $$ = makeNode(Or);
                                            addChild($$, $1);
                                            addChild($$, $3);
                                        }
    |  TB                               { $$ = $1;}
    ;
TB  :  TB AND FB                        {
                                            $$ = makeNode(And);
                                            addChild($$, $1);
                                            addChild($$, $3);
                                        }
    |  FB                               { $$ = $1; }
    ;
FB  :  FB EQ M                          {
											                      $$ = makeNode(Eq);
                                            addChild($$, $1);
                                            addChild($$, $3);
                                        }
                                        
    |  FB DIFF M                        {
                                            $$ = makeNode(Diff);
                                            addChild($$, $1);
                                            addChild($$, $3);
                                        }                                    
                                        
    |  M                                { $$ = $1; }
    ;
M   :  M INF E                          {		
                                            $$ = makeNode(Inf);
                                            addChild($$, $1);
                                            addChild($$, $3);
                                        }
    |  M SUP E                          {		
                                            $$ = makeNode(Sup);		
                                            addChild($$, $1);
                                            addChild($$, $3);
                                        }                                    
                                        
    |  M INFEQ E                        {		
                                            $$ = makeNode(InfEq);		
                                            addChild($$, $1);
                                            addChild($$, $3);
                                        }
                                        
    |  M SUPEQ E                        {		
                                            $$ = makeNode(SupEq);		
                                            addChild($$, $1);
                                            addChild($$, $3);
                                        }                                        
                                        
    |  E                                { $$ = $1; }
    ;
E   :  E ADDSUB T                       { 
                                            if($2 == '+')
                                                $$ = makeNode(Add);
                                            else
                                                $$ = makeNode(Sub);
                                            addChild($$, $1);
                                            addChild($$, $3);
                                        }
    |  T                                { $$ = $1; }
    ;    
T   :  T '*' F                          {
                                            $$ = makeNode(Mult);
                                            addChild($$, $1);
                                            addChild($$, $3);
                                        }
    |  T '/' F                          {
                                            $$ = makeNode(Div);
                                            addChild($$, $1);
                                            addChild($$, $3);
                                        }        
    |  T '%' F                          {   
                                            $$ = makeNode(Modulo);
                                            addChild($$, $1);
                                            addChild($$, $3);
                                        }            
    |  F                                { $$ = $1; }
    ;
F   :  ADDSUB F                         {
                                            if($1 == '+') 
                                                $$ = $2;
                                            else { //if the given token is negative, mention so using a Neg node in the tree
                                                $$ = makeNode(Neg);
                                                addChild($$, $2);
                                            }
                                        }
    |  '!' F                            {
                                            $$ = makeNode(Neg);
                                            addChild($$, $2);
                                        }
                                        
                                        
    |  '&' IDENT                        { 
                                            $$ = makeNode(Deref);
											                      Node* value = makeNode(ValueOf);
                                            Node* tmp = makeNode(Identifier);
                                            strcpy(tmp->u.identifier, $2); //get the identifier from the lexer

                                            addChild($$, value);
                                            addChild(value, tmp);
    									}
    |  '(' Exp ')'                      { $$ = $2; }
    |  NUM                              { 
                                            $$ = makeNode(IntLiteral);
                                            $$->u.integer = $1;
                                        }
    |  CHARACTER                        { 
                                            $$ = makeNode(CharLiteral);
                                            $$->u.character = $1;
                                        }
                                        
                                        
	|  IDENT                              {
                                            $$ = makeNode(ValueOf);
                                            Node* tmp = makeNode(Identifier);
                                            strcpy(tmp->u.identifier, $1); //get the identifier from the lexer
                                            addChild($$, tmp);
                                        }
                                        
    |  '*' IDENT                        {
                                            $$ = makeNode(Pointer);
											                      Node* value = makeNode(ValueOf);
                                            Node* tmp = makeNode(Identifier);
                                            strcpy(tmp->u.identifier, $2); //get the identifier from the lexer

                                            addChild($$, value);
                                            addChild(value, tmp);
                                        }
                                        
                                        
                                        
    |  IDENT '(' Arguments  ')'         {
                                            $$ = makeNode(FuncCall);

                                            Node* tmp = makeNode(Identifier);
                                            strcpy(tmp->u.identifier, $1); //get the identifier from the lexer
                                            addChild($$, tmp);

                                            addChild($$, $3);
                                        }
    |  '*' IDENT '(' Arguments  ')'     {
                                            //pointeur sur fonction ici, à rajouter??
                                            $$ = makeNode(FuncCall);
                                            
                                            Node* tmp = makeNode(Identifier);
                                            strcpy(tmp->u.identifier, $2); //get the identifier from the lexer
                                            addChild($$, tmp);

                                            addChild($$, $4);
                                        }
    ;
LValue:
       IDENT                            {
                                            $$ = makeNode(Identifier);
                                            strcpy($$->u.identifier, $1); //get the identifier from the lexer
                                        }
    |  '*' IDENT                        {
                                            $$ = makeNode(Pointer);

                                            Node* tmp = makeNode(Identifier);
                                            strcpy(tmp->u.identifier, $2); //get the identifier from the lexer

                                            addChild($$, tmp);
                                        }
    ;
Arguments:
       ListExp                          { 
                                            $$ = makeNode(Arguments);
                                            addChild($$, $1);
                                        }
    |                                   { $$ = NULL; }
    ;
ListExp:
       ListExp ',' Exp                  {
                                            $$ = $1; 
                                            addSibling($$, $3);
                                        }
    |  Exp                              { $$ = $1; }
    ;
%%


void print_error(){
    fprintf(stderr,"\033[0;31m");
    fprintf(stderr, "fatal error: ");
    fprintf(stderr, "\033[0m");
    fprintf(stderr, "compilation terminated.\n");
    return;
}


int main(int argc, char** argv) {
    int is_invalid;

    //check for file as argument
    if(argc != 3 || strcmp(argv[1], "-o")){
        fprintf(stderr, "usage: ./bin/compil -o [FILE].asm < [FILE].tpc\n");
        return -1;
    }

    //parse the file
    is_invalid = yyparse();

    if(is_invalid) //if parsing brought an error
        return is_invalid;

    //if main function wasn't found while parsing
    if(!has_main){
        fprintf(stderr, "semantic error: undefined reference to 'main'\n");
        print_error();
        return -1;
    }

    //print the tree from the parsing
    //printTree(root);
	
    //create and fill the symbol table from the tree
	SymbolTable* global = makeSymbolTable("Global");
	fillTable(global, root);	

    error_check(root, global);

    //if we found an error while creating the symbol table
    //such as an undeclared variable
    if(err){
        print_error();
        return -1;
    }

    FILE* nasm = fopen(argv[2], "w+");

    //if no error, the code can be generated
    translate(root, nasm, global);


    if(!nasm){
        fprintf(stderr, "error while creating NASM file\n");
        return -1; 
    }

    //if everything went well, compilation was successful
    printf("compiled successfully.\n");
    
    return is_invalid;
}

void yyerror(char *s){
    int lines_seen, chars_seen;
    int count = 0;
    char read, write;

    if(fseek(yyin, 0L, SEEK_SET) != 0){
        fprintf(stderr, "could not rewind yyin, error when trying to display error\n");
        return;
    }

    if(charno == 1 && lineno == 1){
        fprintf(stderr, "semantic error: undefined reference to 'main'\n");
        print_error();
        return;    
    }

    if(!is_lexical_error)
        fprintf(stderr, "%s near line %d\n--->", s, lineno);
    else
        fprintf(stderr, "--->");

    //skip the lines until we are at the line that gave the error
    for(lines_seen = 1 ; lines_seen < lineno - 1 ; lines_seen++){
        read = '0';
        while(read != '\n'){
            read = fgetc(yyin);
            if(read == EOF)
                return;
        }
    }

    //we are at the line, write it character by caracter
    while(count < charno + 1){
        write = fgetc(yyin);
        if(write == '\n')
            continue;
        fprintf(stderr, "%c", write);
        count++;
    }
    fprintf(stderr, "\n");

    //charno + 4 since we print an arrow of length 4
    for(chars_seen = 1 ; chars_seen < charno + 4 ; chars_seen++)
        fprintf(stderr, " ");
    fprintf(stderr, "^");

    fprintf(stderr, "\n");

    
}

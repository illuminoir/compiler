%{
/* as.lex */
int lineno = 1;
int charno = 1;
int is_lexical_error = 0;
int fileno(FILE *stream);

#include <string.h>
#include "parser.tab.h"
#include "../lib/tree/abstract-tree.h"

void lexical_error();

%}


%option nounput
%option noinput
%option noyywrap

%x COM

%%


"/*"									{BEGIN COM;}
<COM>"*/" 								{BEGIN INITIAL;}
<COM>\n 								{lineno++;}
<COM>.									{;}


\n 										{
											lineno++;
											charno = 1;
										}

[ \r\t]+ 								{
											charno += yyleng;
										}

print  									{
											charno += yyleng;
											return PRINT;
										}

"&&"									{
											charno += yyleng;
											return AND;
										}

"||"  									{
											charno += yyleng;
											return OR;
										}

[+-]									{
											charno += yyleng;
											yylval.addsub = yytext[0];
											return ADDSUB;
										}

[*%/]									{
											charno += yyleng;
											return yytext[0];
										}

int     	  				      	    {
											charno += yyleng;
											strcpy(yylval.type, "Int");
											return TYPE;
										}

char	    			        	   	{
											charno += yyleng;
											strcpy(yylval.type, "Char");
											return TYPE;
										}

void 					 				{
											charno += yyleng;
											return VOID;
										}

return									{
											charno += yyleng;
											return RETURN;
										}
			
if										{
											charno += yyleng;
											return IF;
										}

while									{
											charno += yyleng;
											return WHILE;
										}

else									{
											charno += yyleng;
											return ELSE;
										}

reade 									{
											charno += yyleng;
											return READE;
										}

readc  									{
											charno += yyleng;
											return READC;
										}

"<="									{
											charno += yyleng;
											return INFEQ;
										}

">="									{
											charno += yyleng;
											return SUPEQ;
										}

"<"										{
											charno += yyleng;
											return INF;
										}

">"										{
											charno += yyleng;
											return SUP;
										}

"!="								    {
											charno += yyleng;
											return DIFF;
										}

"=="								    {
											charno += yyleng;
											return EQ;
										}

[0-9]+									{
											charno += yyleng;
											yylval.num = atoi(yytext);
											return NUM;
										}

'\\?.'									{
											charno += yyleng;
											yylval.character = yytext[1];
											return CHARACTER;
										}

[a-zA-Z][a-zA-Z0-9_]*			  		{
											charno += yyleng;
											strcpy(yylval.ident, yytext);
											return IDENT;
										}

";"|","|"("|")"|"!"|"&"|"{"|"}"|"="		{
											charno += yyleng;
											return yytext[0];
										}	

.										{
											lexical_error();
											is_lexical_error = 1;
											return yytext[0];
										}

%%

void lexical_error(){
	/* indiquer au parser que erreur, un programme en erreur lexicale est considéré bon sans erreur syntaxique */
	fprintf(stderr, "lexical error on line %d at character %d\n", lineno, charno);
}

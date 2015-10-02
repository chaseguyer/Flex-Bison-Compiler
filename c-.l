%{
#include "tree.c"
#include <stdlib.h>
#include <string.h>
#include "c-.tab.h"

char makeEscapeCharC(char* input) {
	if(strlen(input) == 3) 
		return input[1];
		
	else if(input[1] == '\\') {
		if(input[2] == 'n')
			return '\n';
		else if(input[2] == '0')
			return '\0';
		else
			return input[2];
	} 

	else if(strlen(input) > 3 && input[0] != '\\') 
		return 'E';
	else if(input[0] == '\'' && input[1] == '\'')
		return 'E';

	return '\n';
}

char* makeEscapeCharS(char* input) {
	int i, j = 0;
	char* tmpString = (char*) malloc(sizeof(char) * strlen(input));
	for(i = 1; i < (strlen(input)-1); i++) {
		if(input[i] == '\\') {
	        if(input[i+1] == 'n') {
				tmpString[j] = '\n';
				i++;
			} 
	        else if(input[i+1] == '0') {
				tmpString[j] = '\0';
				i++;
			}
			else{
				tmpString[j] = input[i+1];
				i++;
			}
		}
		else {
			tmpString[j] = input[i];
		}
		j++;
	}
	yylval.token.stringLen = j;
	return tmpString;
}
//"+"					{ yylval.token.cconst = yytext[0]; return PLUS; } 
%}

%option yylineno
%option noyywrap
%x COMMENT
%x COMMENT_BLOCK

%%

[ \t\n]				;
"//"				{ BEGIN(COMMENT); }
<COMMENT>.			;
<COMMENT>"\n"		{ BEGIN(INITIAL); }
"/*"				{ BEGIN(COMMENT_BLOCK); }
<COMMENT_BLOCK>.	;
<COMMENT_BLOCK>"*/"	{ BEGIN(INITIAL); }
[#^_@'"~]			{ yylval.token.input = strdup(yytext); return ERROR; }
"true"				{ yylval.token.input = strdup(yytext); yylval.token.bconst = 1; return BOOLCONST; }
"false"				{ yylval.token.input = strdup(yytext); yylval.token.bconst = 0; return BOOLCONST; }
"!="				{ yylval.token.input = strdup(yytext); return NOTEQ; }
"*="				{ yylval.token.input = strdup(yytext); return MULASS; }
"++"				{ yylval.token.input = strdup(yytext); return INC; }
"+="				{ yylval.token.input = strdup(yytext); return ADDASS; }
"--"				{ yylval.token.input = strdup(yytext); return DEC; }
"-="				{ yylval.token.input = strdup(yytext); return SUBASS; }
"/="				{ yylval.token.input = strdup(yytext); return DIVASS; }
"<="				{ yylval.token.input = strdup(yytext); return LESSEQ; }
"=="				{ yylval.token.input = strdup(yytext); return EQ; }
">="				{ yylval.token.input = strdup(yytext); return GRTEQ; }
"bool"				{ yylval.token.input = strdup(yytext); return BOOL; }
"break"				{ yylval.token.input = strdup(yytext); return BREAK; }
"char"				{ yylval.token.input = strdup(yytext); return CHAR; }
"else"				{ yylval.token.input = strdup(yytext); return ELSE; }
"foreach"			{ yylval.token.input = strdup(yytext); return FOREACH; }
"if"				{ yylval.token.input = strdup(yytext); return IF; }
"in"				{ yylval.token.input = strdup(yytext); return IN; }
"int"				{ yylval.token.input = strdup(yytext); return INT; }
"return"			{ yylval.token.input = strdup(yytext); return RETURN; }
"static"			{ yylval.token.input = strdup(yytext); return STATIC; }
"while"				{ yylval.token.input = strdup(yytext); return WHILE; }
";"					{ yylval.token.input = strdup(yytext); return SEMICOLON; }
","					{ yylval.token.input = strdup(yytext); return COMMA; }
":"					{ yylval.token.input = strdup(yytext); return COLON; }
"["					{ yylval.token.input = strdup(yytext); return LBRACK; }
"]"					{ yylval.token.input = strdup(yytext); return RBRACK; }
"("					{ yylval.token.input = strdup(yytext); yylval.token.nconst = yylineno; return LPEREN; }
")"					{ yylval.token.input = strdup(yytext); return RPEREN; }
"{"					{ yylval.token.input = strdup(yytext); yylval.token.nconst = yylineno; return LCURL; } 
"}"					{ yylval.token.input = strdup(yytext); return RCURL; }
"="					{ yylval.token.input = strdup(yytext); return ASSIGN; }
"|"					{ yylval.token.input = strdup(yytext); return OR; }
"&"					{ yylval.token.input = strdup(yytext); return AND; }
"!"					{ yylval.token.input = strdup(yytext); return BANG; }
"<"					{ yylval.token.input = strdup(yytext); return LESS; }
">"					{ yylval.token.input = strdup(yytext); return GRT; }
"+"					{ yylval.token.input = strdup(yytext); return PLUS; }
"-"					{ yylval.token.input = strdup(yytext); return MINUS; }
"*"					{ yylval.token.input = strdup(yytext); return STAR; }
"/"					{ yylval.token.input = strdup(yytext); return FWDSLASH; }
"%"					{ yylval.token.input = strdup(yytext); return MOD; }
"?"					{ yylval.token.input = strdup(yytext); return QUES; }
[0-9]+				{ yylval.token.input = strdup(yytext); yylval.token.nconst = atoi(yytext); return NUMCONST; }
\"(.)*\"			{ yylval.token.input = strdup(yytext); yylval.token.sconst = makeEscapeCharS(yytext); return STRINGCONST; }
'(.)*'				{ yylval.token.input = strdup(yytext); yylval.token.cconst = makeEscapeCharC(yytext); return CHARCONST; }
[a-zA-Z]+[0-9]*		{ yylval.token.input = strdup(yytext); yylval.token.id = strdup(yytext); return ID; }
.					{ yylval.token.input = strdup(yytext); return yytext[0]; }
%%

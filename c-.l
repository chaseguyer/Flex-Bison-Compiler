%{
#include"tree.cpp"
#include"semantic.cpp"
#include"symTab.cpp"
#include"emitcode.cpp"
#include"codegen.cpp"
#include"synErr.cpp"
#include"c-.tab.h"
#include<stdlib.h>
#include<stdio.h>
#include<string>

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
		return '`'; // the ` is only temporary, it needs to be flagged for a warning somehow
					// will change this later
	else if(input[0] == '\'' && input[1] == '\'')
		return '`';

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

%}

%option yylineno
%option noyywrap
%x COMMENT

%%

[ \t\n]				;
"//"				{ BEGIN(COMMENT); }
<COMMENT>.			;
<COMMENT>"\n"		{ BEGIN(INITIAL); }
"true"				{ yylval.token.input = strdup(yytext); yylval.token.bconst = 1; return BOOLCONST; }
"false"				{ yylval.token.input = strdup(yytext); yylval.token.bconst = 0; return BOOLCONST; }
"!="				{ yylval.token.input = strdup(yytext); yylval.token.lineNum = yylineno; return NOTEQ; }
"*="				{ yylval.token.input = strdup(yytext); yylval.token.lineNum = yylineno; return MULASS; }
"++"				{ yylval.token.input = strdup(yytext); yylval.token.lineNum = yylineno; return INC; }
"+="				{ yylval.token.input = strdup(yytext); yylval.token.lineNum = yylineno; return ADDASS; }
"--"				{ yylval.token.input = strdup(yytext); yylval.token.lineNum = yylineno; return DEC; }
"-="				{ yylval.token.input = strdup(yytext); yylval.token.lineNum = yylineno; return SUBASS; }
"/="				{ yylval.token.input = strdup(yytext); yylval.token.lineNum = yylineno; return DIVASS; }
"<="				{ yylval.token.input = strdup(yytext); yylval.token.lineNum = yylineno; return LESSEQ; }
"=="				{ yylval.token.input = strdup(yytext); yylval.token.lineNum = yylineno; return EQ; }
">="				{ yylval.token.input = strdup(yytext); yylval.token.lineNum = yylineno; return GRTEQ; }
"bool"				{ yylval.token.input = strdup(yytext); yylval.token.lineNum = yylineno; return BOOL; }
"break"				{ yylval.token.input = strdup(yytext); yylval.token.lineNum = yylineno; return BREAK; }
"char"				{ yylval.token.input = strdup(yytext); yylval.token.lineNum = yylineno; return CHAR; }
"else"				{ yylval.token.input = strdup(yytext); yylval.token.lineNum = yylineno; return ELSE; }
"foreach"			{ yylval.token.input = strdup(yytext); yylval.token.lineNum = yylineno; return FOREACH; }
"if"				{ yylval.token.input = strdup(yytext); yylval.token.lineNum = yylineno; return IF; }
"in"				{ yylval.token.input = strdup(yytext); yylval.token.lineNum = yylineno; return IN; }
"int"				{ yylval.token.input = strdup(yytext); yylval.token.lineNum = yylineno; return INT; }
"return"			{ yylval.token.input = strdup(yytext); yylval.token.lineNum = yylineno; return RETURN; }
"static"			{ yylval.token.input = strdup(yytext); yylval.token.lineNum = yylineno; return STATIC; }
"while"				{ yylval.token.input = strdup(yytext); yylval.token.lineNum = yylineno; return WHILE; }
";"					{ yylval.token.input = strdup(yytext); yylval.token.lineNum = yylineno; return ';'; }
","					{ yylval.token.input = strdup(yytext); yylval.token.lineNum = yylineno; return ','; }
":"					{ yylval.token.input = strdup(yytext); yylval.token.lineNum = yylineno; return ':'; }
"["					{ yylval.token.input = strdup(yytext); yylval.token.lineNum = yylineno; return '['; }
"]"					{ yylval.token.input = strdup(yytext); yylval.token.lineNum = yylineno; return ']'; }
"("					{ yylval.token.input = strdup(yytext); yylval.token.lineNum = yylineno; return '('; }
")"					{ yylval.token.input = strdup(yytext); yylval.token.lineNum = yylineno; return ')'; }
"{"					{ yylval.token.input = strdup(yytext); yylval.token.lineNum = yylineno; return '{'; } 
"}"					{ yylval.token.input = strdup(yytext); yylval.token.lineNum = yylineno; return '}'; }
"="					{ yylval.token.input = strdup(yytext); yylval.token.lineNum = yylineno; return '='; }
"|"					{ yylval.token.input = strdup(yytext); yylval.token.lineNum = yylineno; return '|'; }
"&"					{ yylval.token.input = strdup(yytext); yylval.token.lineNum = yylineno; return '&'; }
"!"					{ yylval.token.input = strdup(yytext); yylval.token.lineNum = yylineno; return '!'; }
"<"					{ yylval.token.input = strdup(yytext); yylval.token.lineNum = yylineno; return '<'; }
">"					{ yylval.token.input = strdup(yytext); yylval.token.lineNum = yylineno; return '>'; }
"+"					{ yylval.token.input = strdup(yytext); yylval.token.lineNum = yylineno; return '+'; }
"-"					{ yylval.token.input = strdup(yytext); yylval.token.lineNum = yylineno; return '-'; }
"*"					{ yylval.token.input = strdup(yytext); yylval.token.lineNum = yylineno; return '*'; }
"/"					{ yylval.token.input = strdup(yytext); yylval.token.lineNum = yylineno; return '/'; }
"%"					{ yylval.token.input = strdup(yytext); yylval.token.lineNum = yylineno; return '%'; }
"?"					{ yylval.token.input = strdup(yytext); yylval.token.lineNum = yylineno; return '?'; }
[0-9]+				{ yylval.token.input = strdup(yytext); yylval.token.nconst = atoi(yytext); return NUMCONST; }
\"((\\\")|[^\"\n])*\"	{ yylval.token.input = strdup(yytext); yylval.token.sconst = makeEscapeCharS(yytext); return STRINGCONST; }
'((\\\')|[^\'\n])*\'	{ 
						yylval.token.input = strdup(yytext); 
						yylval.token.cconst = makeEscapeCharC(yytext); 
						if(yylval.token.cconst == '`') {
							printf("WARNING(%d): character is %lu characters and not a single character: %s\n", yylineno, (strlen(strdup(yytext))-2), strdup(yytext)); // the -2 is for a ' at the beginning and end
							numWarnings++;
						}
						return CHARCONST; 
					}
[a-zA-Z]+[a-zA-Z0-9]* { yylval.token.input = strdup(yytext); yylval.token.id = strdup(yytext); yylval.token.lineNum = yylineno; return ID; }
.					{ 
						printf("WARNING(%d): Invalid input character: \'%s\'. Character ignored.\n", yylineno, strdup(yytext));
						numWarnings++;
					}
%%


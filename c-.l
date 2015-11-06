%{
#include "tree.c"
#include "semantic.c"
#include "symTab.cpp"
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

// // // // // // // // // // // // // // // // // // // // 
//
// Error message printing
//
// Must make messages look nice.  For example:
// msg = "syntax error, unexpected ',', expecting BOOL or CHAR or INT or ID."
// becomes (xx marks important data):
//  0 syntax
//  1 error,
//  2 unexpected
//  3 ',',    xx
//  4 expecting
//  5 BOOL    xx
//  6 or
//  7 CHAR    xx
//  8 or
//  9 INT     xx
// 10 or
// 11 ID.     xx

// assumes a string with breakchar separating each element.
// breakchars will be replaced by null chars: '\0'
// array of pointers to strings is then returned in
// the array strs which must be allocated by the user!
// the number of strings found is returned as a value of
// the function.  This number is always at least 1.
// The array is terminated by a NULL so there must be
// enough room for all the string pointers plus one for the
// sentinel marker.

int line;

int split(char *s, char *strs[], char breakchar)
{
    int num;
    
    strs[0] = s;
    num = 1;
    for (char *p = s; *p; p++) {
        if (*p==breakchar) {
            strs[num++] = p+1;
            *p = '\0';
        }
    }
    strs[num] = NULL;
    
    return num;
}


// trim off the last character
void trim(char *s)
{
    s[strlen(s)-1] = '\0';
}


// map from string to char * for storing nice translation of
// internal names for tokens.  Preserves (char *) used by
// bison.
static std::map<std::string , char *> niceTokenNameMap;    // use an ordered map (not as fast as unordered)

// WARNING: this routine must be called to initialize mapping of
// (strings returned as error message) --> (human readable strings)
//
void initTokenMaps() {
    niceTokenNameMap["NOTEQ"] = (char *)"'!='";
    niceTokenNameMap["MULASS"] = (char *)"'*='";
    niceTokenNameMap["INC"] = (char *)"'++'";
    niceTokenNameMap["ADDASS"] = (char *)"'+='";
    niceTokenNameMap["DEC"] = (char *)"'--'";
    niceTokenNameMap["SUBASS"] = (char *)"'-='";
    niceTokenNameMap["DIVASS"] = (char *)"'/='";
    niceTokenNameMap["LESSEQ"] = (char *)"'<='";
    niceTokenNameMap["EQ"] = (char *)"'=='";
    niceTokenNameMap["GRTEQ"] = (char *)"'>='";
    niceTokenNameMap["BOOL"] = (char *)"bool";
    niceTokenNameMap["BREAK"] = (char *)"break";
    niceTokenNameMap["CHAR"] = (char *)"char";
    niceTokenNameMap["ELSE"] = (char *)"else";
    niceTokenNameMap["FOREACH"] = (char *)"foreach";
    niceTokenNameMap["IF"] = (char *)"if";
    niceTokenNameMap["IN"] = (char *)"in";
    niceTokenNameMap["INT"] = (char *)"int";
    niceTokenNameMap["RETURN"] = (char *)"return";
    niceTokenNameMap["STATIC"] = (char *)"static";
    niceTokenNameMap["WHILE"] = (char *)"while";
    niceTokenNameMap["BOOLCONST"] = (char *)"Boolean constant";
    niceTokenNameMap["NUMCONST"] = (char *)"numeric constant";
    niceTokenNameMap["ID"] = (char *)"identifier";
    niceTokenNameMap["CHARCONST"] = (char *)"character constant";
    niceTokenNameMap["STRINGCONST"] = (char *)"string constant";
    niceTokenNameMap["$end"] = (char *)"end of input";
}


// looks of pretty printed words for tokens that are
// not already in single quotes.  It uses the niceTokenNameMap table.
char *niceTokenStr(char *tokenName ) {
    if (tokenName[0] == '\'') return tokenName;
    if (niceTokenNameMap.find(tokenName) == niceTokenNameMap.end()) {
        printf("ERROR(SYSTEM): niceTokenStr fails to find string '%s'\n", tokenName); 
        fflush(stdout);
        exit(1);
    }
    return niceTokenNameMap[tokenName];
}


// Is this a message that we need to elaborate with the current parsed token.
// This elaboration is some what of a crap shoot since the token could
// be already overwritten with a look ahead token.   But probably not.
bool elaborate(char *s)
{
    return (strstr(s, "constant") || strstr(s, "identifier"));
}


// This is the yyerror called by the bison parser for errors.
// It only does errors and not warnings.   

void yyerror(const char *msg)
{
    char *space;
    char *strs[100];
    int numstrs;

    // make a copy of msg string
    space = strdup(msg);

    // split out components
    numstrs = split(space, strs, ' ');
    if (numstrs>4) trim(strs[3]);

    // translate components
    for (int i=3; i<numstrs; i+=2) {
        strs[i] = niceTokenStr(strs[i]);
    }

    // print components
    printf("ERROR(%d): Syntax error, unexpected %s", line, strs[3]);
    if (elaborate(strs[3])) {
        if (yytext[0]=='\'' || yytext[0]=='"') printf(" %s", yytext); 
        else printf(" \'%s\'", yytext);
    }
    if (numstrs>4) printf(",");
    for (int i=4; i<numstrs; i++) {
        printf(" %s", strs[i]);
    }
    printf(".\n");
    fflush(stdout);   // force a dump of the error

    numErrors++;

    free(space);
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
[#^_@'"~]			{ yylval.token.input = strdup(yytext); return ERROR; }
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
";"					{ yylval.token.input = strdup(yytext); yylval.token.lineNum = yylineno; return SEMICOLON; }
","					{ yylval.token.input = strdup(yytext); yylval.token.lineNum = yylineno; return COMMA; }
":"					{ yylval.token.input = strdup(yytext); yylval.token.lineNum = yylineno; return COLON; }
"["					{ yylval.token.input = strdup(yytext); yylval.token.lineNum = yylineno; return LBRACK; }
"]"					{ yylval.token.input = strdup(yytext); yylval.token.lineNum = yylineno; return RBRACK; }
"("					{ yylval.token.input = strdup(yytext); yylval.token.lineNum = yylineno; return LPEREN; }
")"					{ yylval.token.input = strdup(yytext); yylval.token.lineNum = yylineno; return RPEREN; }
"{"					{ yylval.token.input = strdup(yytext); yylval.token.lineNum = yylineno; return LCURL; } 
"}"					{ yylval.token.input = strdup(yytext); yylval.token.lineNum = yylineno; return RCURL; }
"="					{ yylval.token.input = strdup(yytext); yylval.token.lineNum = yylineno; return ASSIGN; }
"|"					{ yylval.token.input = strdup(yytext); yylval.token.lineNum = yylineno; return OR; }
"&"					{ yylval.token.input = strdup(yytext); yylval.token.lineNum = yylineno; return AND; }
"!"					{ yylval.token.input = strdup(yytext); yylval.token.lineNum = yylineno; return BANG; }
"<"					{ yylval.token.input = strdup(yytext); yylval.token.lineNum = yylineno; return LESS; }
">"					{ yylval.token.input = strdup(yytext); yylval.token.lineNum = yylineno; return GRT; }
"+"					{ yylval.token.input = strdup(yytext); yylval.token.lineNum = yylineno; return PLUS; }
"-"					{ yylval.token.input = strdup(yytext); yylval.token.lineNum = yylineno; return MINUS; }
"*"					{ yylval.token.input = strdup(yytext); yylval.token.lineNum = yylineno; return STAR; }
"/"					{ yylval.token.input = strdup(yytext); yylval.token.lineNum = yylineno; return FWDSLASH; }
"%"					{ yylval.token.input = strdup(yytext); yylval.token.lineNum = yylineno; return MOD; }
"?"					{ yylval.token.input = strdup(yytext); yylval.token.lineNum = yylineno; return QUES; }
[0-9]+				{ yylval.token.input = strdup(yytext); yylval.token.nconst = atoi(yytext); return NUMCONST; }
\"(.)*\"			{ yylval.token.input = strdup(yytext); yylval.token.sconst = makeEscapeCharS(yytext); return STRINGCONST; }
'(.)*'				{ yylval.token.input = strdup(yytext); yylval.token.cconst = makeEscapeCharC(yytext); return CHARCONST; }
[a-zA-Z]+[a-zA-Z0-9]* { yylval.token.input = strdup(yytext); yylval.token.id = strdup(yytext); yylval.token.lineNum = yylineno; return ID; }
.					{ 
						// we will deal with warnings here
						yylval.token.input = strdup(yytext); 
						return yytext[0]; 
					}
%%

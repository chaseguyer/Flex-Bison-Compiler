%{
#include"globals.h"
#include"tree.h"
#include"struct.h"
#include"semantic.h"
#include"symTab.h"
#include<iostream>
#include<stdio.h>
#include<stdlib.h>
#include<cstring>
#include<getopt.h>
#define YYERROR_VERBOSE

using namespace std;

extern int yylex();
extern int yylineno;
extern FILE *yyin;

TreeNode *syntaxTree = NULL;

int numWarnings = 0, numErrors = 0;
void yyerror(const char *msg) {printf("ERROR(PARSER): %s on line %d\n", msg, yylineno); }
char cconstError(char, char*);

%}

%union {
	int number;
	ExpType type;
	Token token;
	TreeNode *tree;
}

%token <token.nconst> NUMCONST 
%token <token.cconst> CHARCONST 
%token <token.sconst> STRINGCONST 
%token <token.id> ID 
%token <token.bconst> BOOLCONST 
%token <token.input> NOTEQ MULASS INC ADDASS DEC SUBASS DIVASS LESSEQ EQ GRTEQ ERROR
%token <token.input> BOOL BREAK CHAR ELSE FOREACH IF IN INT RETURN STATIC WHILE
%token <token.input> SEMICOLON COMMA COLON RBRACK RPEREN RCURL 
%token <token.input> ASSIGN OR AND BANG LESS GRT PLUS MINUS STAR FWDSLASH MOD QUES

%token<token.nconst> LBRACK LPEREN LCURL 

%type<number> type_specifier
%type <token.input> relop unaryop mulop sumop
%type <tree> program
%type <tree> declaration_list
%type <tree> declaration
%type <tree> var_declaration;
%type <tree> var_decl_list;
%type <tree> var_decl_initialize;
%type <tree> var_decl_id;
%type <tree> scoped_var_declaration;
%type <tree> scoped_type_specifier;
%type <tree> fun_declaration;
%type <tree> local_declarations;
%type <tree> params;
%type <tree> param_id;
%type <tree> param_list;
%type <tree> param_type_list;
%type <tree> param_id_list;
%type <tree> statement;
%type <tree> statement_list;
%type <tree> matched;
%type <tree> unmatched;
%type <tree> expression;
%type <tree> expression_stmt;
%type <tree> compound_stmt;
%type <tree> matched_selection_stmt; 
%type <tree> matched_foreach_stmt;
%type <tree> matched_while_stmt;
%type <tree> unmatched_selection_stmt;
%type <tree> unmatched_foreach_stmt;
%type <tree> simple_expression;
%type <tree> mutable;
%type <tree> and_expression;
%type <tree> unary_rel_expression;
%type <tree> rel_expression;
%type <tree> sum_expression;
%type <tree> term;
%type <tree> unary_expression;
%type <tree> factor;
%type <tree> immutable;
%type <tree> call;
%type <tree> args;
%type <tree> arg_list;
%type <tree> constant;
%type <tree> unmatched_while_stmt;
%type <tree> return_stmt break_stmt;

%%

program					: declaration_list { syntaxTree = $1; } 
						;

declaration_list		: declaration_list declaration {
							TreeNode *t = $1;
							if(t != NULL) {
								while(t -> sibling != NULL) t = t -> sibling;
								t -> sibling = $2;
								$$ = $1;
							} 
							else {
								$$ = $2;
							}
						}
						| declaration { $$ = $1; }
						;

declaration				: var_declaration { 
							$$ = $1;
						}
						| fun_declaration { 
							$$ = $1;
						}
						;

var_declaration			: type_specifier var_decl_list SEMICOLON { 
							TreeNode *t = $2;
							if(t != NULL) {
								do {
									t -> type = (ExpType)$1;
									t = t -> sibling;
								} while(t != NULL);
					
								$$ = $2;
							} else
								$$ = NULL; 
						}
						;

scoped_var_declaration	: scoped_type_specifier var_decl_list SEMICOLON { 
							TreeNode *t = $2;
							if(t != NULL) {
								do {
									t -> type = (ExpType)$1 -> type;
									t -> isStatic = $1 -> isStatic;
									t -> arrayLen = $1 -> arrayLen;
									t = t -> sibling;
								} while(t != NULL); 

								$$ = $2;
							} else
								$$ = NULL; 
						}
						;

var_decl_list			: var_decl_list COMMA var_decl_initialize { 
							TreeNode *t = $1;
							if(t != NULL) {
								while(t -> sibling != NULL) t = t -> sibling;
								t -> sibling = $3;
								$$ = $1;
							} else
								$$ = $3;
						} 
						| var_decl_initialize { $$ = $1; }
						;

var_decl_initialize		: var_decl_id {
							$$ = $1;
						}
						| var_decl_id COLON simple_expression {
							$$ = $1;
							$$ -> child[0] = $3;
						}
						;

var_decl_id				: ID { 
							$$ = newDeclNode(VarK, yylineno);
							$$ -> attr.name = $1;
						}
						| ID LBRACK NUMCONST RBRACK { 
							$$ = newDeclNode(VarK, yylineno);
							$$ -> attr.name = $1;
							$$ -> isArray = 1;
							$$ -> arrayLen = $3;
						}	
						;

scoped_type_specifier	: STATIC type_specifier { 
							$$ = newDeclNode(VarK, yylineno);	
							$$ -> isStatic = 1;
							$$ -> type = (ExpType)$2;	
						}
						| type_specifier {  
							$$ = newDeclNode(VarK, yylineno);	
							$$ -> isStatic = 0;
							$$ -> type = (ExpType)$1;
						}
						;

type_specifier			: INT {
							$$ = Integer;
						}
						| BOOL { 
							$$ = Boolean;
						}
						| CHAR { 
							$$ = Character;
						}
						;

fun_declaration			: type_specifier ID LPEREN params RPEREN statement {
							$$ = newDeclNode(FunK, $3);
							$$ -> child[0] = $4;
							$$ -> child[1] = $6;
							$$ -> attr.name = $2;
							$$ -> type = (ExpType)$1;
						} 
						| ID LPEREN params RPEREN statement {
							$$ = newDeclNode(FunK, $2);
							$$ -> child[0] = $3;	
							$$ -> child[1] = $5;
							$$ -> attr.name = $1;
						}
						;

params					: param_list {
							$$ = $1;
						} 
						| { $$ = NULL; }
						;

param_list				: param_list SEMICOLON param_type_list {
							TreeNode *t = $1;
							if(t != NULL) {
								while(t -> sibling != NULL) t = t -> sibling;
								t -> sibling = $3;
								$$ = $1;
							} else
								$$ = $3;
						} 
						| param_type_list {
							$$ = $1;
						} 
						;

param_type_list			: type_specifier param_id_list { 
							TreeNode *t = $2;
							if(t != NULL) {
								do {
									t -> type = (ExpType)$1;
									t = t -> sibling;
								} while(t != NULL);
								
								$$ = $2;
							} else
								$$ = NULL;
						} 
						;

param_id_list			: param_id_list COMMA param_id { 
							TreeNode *t = $1;
							if(t != NULL) {
								while(t -> sibling != NULL) t = t -> sibling;
								t -> sibling = $3;
								$$ = $1;
							} else
								$$ = $3;
						} 
						| param_id {
							$$ = $1;
						} 
						;

param_id				: ID { 
							$$ = newDeclNode(ParamK, yylineno);
							$$ -> attr.name = $1;
						} 
						| ID LBRACK RBRACK { 
							$$ = newDeclNode(ParamK, yylineno);
							$$ -> attr.name = $1;
							$$ -> isArray = 1;
						}
						;

statement				: matched { $$ = $1; }
						| unmatched { $$ = $1; }
						;
						
matched					: expression_stmt  { $$ = $1; }
						| compound_stmt { $$ = $1; }
						| matched_selection_stmt  { $$ = $1; }
						| matched_foreach_stmt { $$ = $1; }
						| matched_while_stmt { $$ = $1; }
						| return_stmt { $$ = $1; }
						| break_stmt { $$ = $1; }
						;

unmatched				: unmatched_selection_stmt { $$ = $1; }
						| unmatched_foreach_stmt { $$ = $1; }
						| unmatched_while_stmt { $$ = $1; }
						;

compound_stmt			: LCURL local_declarations statement_list RCURL { 
							$$ = newStmtNode(CompK, $1);
							$$ -> child[0] = $2;
							$$ -> child[1] = $3;
						}
						;

local_declarations		: local_declarations scoped_var_declaration  {
							TreeNode *t = $1;
							if(t != NULL) {
								while(t -> sibling != NULL) t = t -> sibling;
								t -> sibling = $2;
								$$ = $1;
							} else 
								$$ = $2;
						}
						| { $$ = NULL; }
						;

statement_list			: statement_list statement {
							TreeNode *t = $1;
							if(t != NULL) {
								while(t -> sibling != NULL) t = t -> sibling;
								t -> sibling = $2;
								$$ = $1;
							} else 
								$$ = $2;
						}
						| { $$ = NULL; }
						;

expression_stmt			: expression SEMICOLON {
							$$ = $1;
						}
						| SEMICOLON {
							$$ = NULL;
						}
						;

matched_selection_stmt	: IF LPEREN simple_expression RPEREN matched ELSE matched {
							$$ = newStmtNode(IfK, $2);
							$$ -> attr.name = $1;
							$$ -> child[0] = $3;
							$$ -> child[1] = $5;
							$$ -> child[2] = $7;
						}
						;

unmatched_selection_stmt: IF LPEREN simple_expression RPEREN matched {
							$$ = newStmtNode(IfK, $2);
							$$ -> attr.name = $1;
							$$ -> child[0] = $3;
							$$ -> child[1] = $5;
						}
						| IF LPEREN simple_expression RPEREN unmatched {
							$$ = newStmtNode(IfK, $2);
							$$ -> attr.name = $1;
							$$ -> child[0] = $3;
							$$ -> child[1] = $5;
						}
						| IF LPEREN simple_expression RPEREN matched ELSE unmatched {
							$$ = newStmtNode(IfK, $2);
							$$ -> attr.name = $1;
							$$ -> child[0] = $3;
							$$ -> child[1] = $5;
							$$ -> child[2] = $7;
						}
						;

matched_foreach_stmt	: FOREACH LPEREN mutable IN simple_expression RPEREN matched {
							$$ = newStmtNode(ForeachK, $2);
							$$ -> attr.name = $1;
							$$ -> child[0] = $3;
							$$ -> child[1] = $5;
							$$ -> child[2] = $7;
						}
						;

unmatched_foreach_stmt	: FOREACH LPEREN mutable IN simple_expression RPEREN unmatched {
							$$ = newStmtNode(ForeachK, $2);
							$$ -> attr.name = $1;
							$$ -> child[0] = $3;
							$$ -> child[1] = $5;
							$$ -> child[2] = $7;
						}
						;

matched_while_stmt		: WHILE LPEREN simple_expression RPEREN matched {
							$$ = newStmtNode(WhileK, $2);
							$$ -> attr.name = $1;
							$$ -> child[0] = $3;
							$$ -> child[1] = $5;
						}

						;

unmatched_while_stmt	: WHILE LPEREN simple_expression RPEREN unmatched {
							$$ = newStmtNode(WhileK, $2);
							$$ -> attr.name = $1;
							$$ -> child[0] = $3;
							$$ -> child[1] = $5;
						}
						;

return_stmt				: RETURN SEMICOLON { 
							$$ = newStmtNode(ReturnK, yylineno);
							$$ -> attr.name = $1;	
						}
						| RETURN expression SEMICOLON {
							$$ = newStmtNode(ReturnK, yylineno);
							$$ -> attr.name = $1;	
							$$ -> child[0] = $2;	
						}
						;

break_stmt				: BREAK SEMICOLON {
							$$ = newStmtNode(BreakK, yylineno);
							$$ -> attr.name = $1;	
						}
						;

expression				: mutable ASSIGN expression {
							$$ = newExpNode(AssignK, yylineno);
							$$ -> child[0] = $1;
							$$ -> child[1] = $3;
							$$ -> attr.name = $2;
						}
						| mutable ADDASS expression {
							$$ = newExpNode(AssignK, yylineno);
							$$ -> child[0] = $1;
							$$ -> child[1] = $3;
							$$ -> attr.name = $2;
						}
						| mutable SUBASS expression {
							$$ = newExpNode(AssignK, yylineno);
							$$ -> child[0] = $1;
							$$ -> child[1] = $3;
							$$ -> attr.name = $2;
						}
						| mutable MULASS expression {
							$$ = newExpNode(AssignK, yylineno);
							$$ -> child[0] = $1;
							$$ -> child[1] = $3;
							$$ -> attr.name = $2;
						}
						| mutable DIVASS expression {
							$$ = newExpNode(AssignK, yylineno);
							$$ -> child[0] = $1;
							$$ -> child[1] = $3;
							$$ -> attr.name = $2;
						}
						| mutable INC{
							$$ = newExpNode(AssignK, yylineno);
							$$ -> child[0] = $1;
							$$ -> attr.name = $2;
						}
						| mutable DEC{
							$$ = newExpNode(AssignK, yylineno);
							$$ -> child[0] = $1;
							$$ -> attr.name = $2;
						}
						| simple_expression {
							$$ = $1; 
						}
						;

simple_expression		: simple_expression OR and_expression {
							$$ = newExpNode(OpK, yylineno);
							$$ -> child[0] = $1;
							$$ -> child[1] = $3;
							$$ -> attr.name = $2;
						}
						| and_expression {
							$$= $1;
						}
						;

and_expression			: and_expression AND unary_rel_expression {
							$$ = newExpNode(OpK, yylineno);
							$$ -> child[0] = $1;
							$$ -> child[1] = $3;
							$$ -> attr.name = $2;
						}
						| unary_rel_expression {
							$$= $1;
						}
						;

unary_rel_expression	: BANG unary_rel_expression {
							$$ = newExpNode(OpK, yylineno);
							$$ -> child[0] = $2;
							$$ -> attr.name = $1;
						}
						| rel_expression {
							$$= $1;
						}
						;

rel_expression			: sum_expression relop sum_expression { 
							$$ = newExpNode(OpK, yylineno);
							$$ -> child[0] = $1;
							$$ -> child[1] = $3;
							$$ -> attr.name = $2;
						}
						| sum_expression { 
							$$ = $1;
						}
						;

relop					: LESSEQ {
							$$ = $1;
						}
						| LESS {
							$$ = $1;
						}
						| GRT {
							$$ = $1;
						}
						| GRTEQ {
							$$ = $1;
						}
						| EQ {
							$$ = $1;
						}
						| NOTEQ {
							$$ = $1;
						}
						;

sum_expression			: sum_expression sumop term {
							$$ = newExpNode(OpK, yylineno);
							$$ -> child[0] = $1;
							$$ -> child[1] = $3;
							$$ -> attr.name = $2;
						}
						| term {
							$$ = $1;
						}
						;

sumop					: PLUS {
							$$ = $1;
						}
						| MINUS {
							$$ = $1;
						}
						;

term					: term mulop unary_expression {
							$$ = newExpNode(OpK, yylineno);
							$$ -> child[0] = $1;
							$$ -> child[1] = $3;
							$$ -> attr.name = $2;
						}
						| unary_expression {
							$$ = $1;
						}
						;

mulop					: STAR {
							$$ = $1;
						}
						| FWDSLASH {
							$$ = $1;
						}
						| MOD {
							$$ = $1;
						}
						;

unary_expression		: unaryop unary_expression {
							$$ = newExpNode(OpK, yylineno);
							$$ -> attr.name = $1;
							$$ -> child[0] = $2;	
						}
						| factor {
							$$ = $1;
						}
						;

unaryop					: MINUS {
							$$ = $1;
						}
						| STAR {
							$$ = $1;
						}
						| QUES {
							$$ = $1;
						}
						;

factor					: immutable { $$ = $1; }
						| mutable { $$ = $1; }
						;

mutable					: ID {
							$$ = newExpNode(IdK, yylineno);
							$$ -> attr.name = $1;
						}
						| ID LBRACK expression RBRACK {
							$$ = newExpNode(IdK, yylineno);
							$$ -> attr.name = $1;	
							$$ -> child[0] = $3;	
						}
						;

immutable				: LPEREN expression RPEREN {
							$$ = $2;
						}
						| call {
							$$ = $1;
						}	
						| constant {
							$$ = $1;
						}
						;

call					: ID LPEREN args RPEREN {
							$$ = newExpNode(CallK, $2); 
							$$ -> attr.name = $1;
							$$ -> child[0] = $3;	
						}
						;

args					: arg_list {
							$$ = $1;
						}
						| { $$ = NULL; }
						;

arg_list				: arg_list COMMA expression {
							TreeNode *t = $1;
							if(t != NULL) {
								while(t -> sibling != NULL) t = t -> sibling;
								t -> sibling = $3;
								$$ = $1;
							} else {
								$$ = $3;
							}
						}
						| expression {
							$$ = $1;
						}
						;

constant				: NUMCONST {
							$$ = newExpNode(ConstK, yylineno);
							$$ -> attr.value = $1;
							$$ -> type = Integer;
						}
						| CHARCONST {
							$$ = newExpNode(ConstK, yylineno);
							$$ -> attr.cvalue = $1;
							$$ -> type = Character;
						}
						| STRINGCONST {
							$$ = newExpNode(ConstK, yylineno);
							$$ -> attr.string = $1;
							$$ -> type = String;
						}
						| BOOLCONST {
							$$ = newExpNode(ConstK, yylineno);
							$$ -> attr.value = $1;
							$$ -> type = Boolean;
						}
						;
%%

int main(int args, char** argv) {
	FILE* fptr;

	int c, pFlag = 0, PFlag = 0;
	while((c = getopt(args, argv, "dpP")) != EOF) {
		switch(c) {
			case 'd':
				yydebug = 1;
				break;
			case 'p':
				pFlag = 1;
				break;
			case 'P':
				PFlag = 1;
				break;
			default:
				break;
		}
	}

	if(yydebug == 1 || pFlag == 1 || PFlag == 1) 
		fptr = fopen(argv[2], "r");
	else  
		fptr = fopen(argv[1], "r");

	yyin = fptr;

	do {	
		yyparse();
	} while (!feof(yyin));

	// Bool of last arg is NOTYPE vs TYPE
	if(pFlag) printTree(stdout, syntaxTree, 0, 0, 0); 
	if(numErrors == 0) scopeAndType(syntaxTree); // semantic analysis
	if(PFlag) printTree(stdout, syntaxTree, 0, 0, 1);  

	printf("Number of warnings: %d\nNumber of errors: %d\n", numWarnings, numErrors);
	return 0;
	
}

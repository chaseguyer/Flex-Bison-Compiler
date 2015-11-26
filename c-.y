%{
#include<string>
#include<stdio.h>
#include<stdlib.h>
#include<getopt.h>
#include"struct.h"
#include"globals.h"
#include"tree.h"
#include"semantic.h"
#include"symTab.h"
#include"emitcode.h"
#include"codegen.h"
#include"synErr.h"

#define YYERROR_VERBOSE

extern int yylex(), yylineno;
extern char* yytext;
extern FILE *yyin; 
int numWarnings = 0, numErrors = 0;
FILE *code;

TreeNode *syntaxTree = NULL;

%}

%union {
	int number;
	ExpType type;
	Token token;
	TreeNode *tree;
}

%token <token> NUMCONST 
%token <token> CHARCONST 
%token <token> STRINGCONST 
%token <token> BOOLCONST 
%token <token> ID 
%token <token> NOTEQ MULASS INC ADDASS DEC SUBASS DIVASS EQ LESSEQ GRTEQ 
%token <token> BOOL BREAK CHAR ELSE FOREACH IF IN INT RETURN STATIC WHILE

%token <token> '(' '[' '{' '=' '+' '-' '*' '/' '?' '%' '|' '&' '!' '>' '<'

%type <number> type_specifier
%type <token> relop unaryop mulop sumop assignop
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
								while(t->sibling != NULL) t = t->sibling;
								t->sibling = $2;
								$$ = $1;
							} 
							else {
								$$ = $2;
							}
						}
						| declaration { $$ = $1; }
						;

declaration				: var_declaration { $$ = $1; }
						| fun_declaration { $$ = $1; }
						| error { $$ = NULL; }
						;

var_declaration			: type_specifier var_decl_list ';' {
							yyerrok; 
							TreeNode *t = $2;
							if(t != NULL) {
								do {
									t->type = (ExpType)$1;
									t = t->sibling;
								} while(t != NULL);
								$$ = $2;
							} else
								$$ = NULL; 
						}
						| error ';' { yyerrok; $$ = NULL; }
						;

scoped_var_declaration	: scoped_type_specifier var_decl_list ';' { 
							yyerrok;
							TreeNode *t = $2;
							if(t != NULL) {
								do {
									t->type = (ExpType)$1->type;
									t->isStatic = $1->isStatic;
									t = t->sibling;
								} while(t != NULL); 
								$$ = $2;
							} else
								$$ = NULL; 
						}
						| scoped_type_specifier error { $$ = NULL; }
						| error ';' { yyerrok; $$ = NULL; }
						;

var_decl_list			: var_decl_list ',' var_decl_initialize { 
							yyerrok;
							TreeNode *t = $1;
							if(t != NULL) {
								while(t->sibling != NULL) t = t->sibling;
								t->sibling = $3;
								$$ = $1;
							} else
								$$ = $3;
						} 
						| var_decl_initialize { $$ = $1; }
						| error ',' var_decl_initialize { yyerrok; $$ = NULL; } 
						| var_decl_list ',' error { $$ = NULL; }
						;

var_decl_initialize		: var_decl_id {
							$$ = $1;
						}
						| var_decl_id ':' simple_expression {
							$$ = $1;
							$$->child[0] = $3;
						}
						| error ':' simple_expression { yyerrok; $$ = NULL; }
						| var_decl_id ':' error { $$ = NULL; }
						| error { $$ = NULL; }
						;

var_decl_id				: ID {
							yyerrok; 
							$$ = newDeclNode(VarK, yylineno);
							$$->attr.name = $1.id;
							$$->lineNum = $1.lineNum;
						}
						| ID '[' NUMCONST ']' {
							yyerrok; 
							$$ = newDeclNode(VarK, yylineno);
							$$->attr.name = $1.id;
							$$->lineNum = $1.lineNum;
							$$->isArray = 1;
							$$->arrayLen = $3.nconst;
						}
						| ID '[' error { $$ = NULL; }
						| error ']' { yyerrok; $$ = NULL; }
						;

scoped_type_specifier	: STATIC type_specifier { 
							$$ = newDeclNode(VarK, yylineno);	
							$$->isStatic = 1;
							$$->type = (ExpType)$2;	
						}
						| type_specifier {  
							$$ = newDeclNode(VarK, yylineno);	
							$$->isStatic = 0;
							$$->type = (ExpType)$1;
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

fun_declaration			: type_specifier ID '(' params ')' statement {
							$$ = newDeclNode(FunK, $3.lineNum);
							$$->child[0] = $4;
							$$->child[1] = $6;
                            $$->attr.name = $2.id;
                            $$->lineNum = $2.lineNum;
							$$->type = (ExpType)$1;
						} 
						| ID '(' params ')' statement {
							$$ = newDeclNode(FunK, $2.lineNum);
							$$->child[0] = $3;	
							$$->child[1] = $5;
                            $$->attr.name = $1.id;
                            $$->lineNum = $1.lineNum;
						}
						| type_specifier error { $$ = NULL; }
						| type_specifier ID '(' error { $$ = NULL; }
						| type_specifier ID '(' params ')' error { $$ = NULL; }
						| ID '(' error { $$ = NULL; }
						| ID '(' params ')' error { $$ = NULL; }
						;

params					: param_list {
							$$ = $1;
						} 
						| { $$ = NULL; }
						;

param_list				: param_list ';' param_type_list {
							yyerrok;
							TreeNode *t = $1;
							if(t != NULL) {
								while(t->sibling != NULL) t = t->sibling;
								t->sibling = $3;
								$$ = $1;
							} else
								$$ = $3;
						} 
						| param_type_list {
							$$ = $1;
						}
						| error ';' param_type_list { yyerrok; $$ = NULL; }
						| param_list ';' error { $$ = NULL; }
						;

param_type_list			: type_specifier param_id_list { 
							TreeNode *t = $2;
							if(t != NULL) {
								do {
									t->type = (ExpType)$1;
									t = t->sibling;
								} while(t != NULL);
								
								$$ = $2;
							} else
								$$ = NULL;
						}
						| type_specifier error { $$ = NULL; } 
						;

param_id_list			: param_id_list ',' param_id { 
							yyerrok;
							TreeNode *t = $1;
							if(t != NULL) {
								while(t->sibling != NULL) t = t->sibling;
								t->sibling = $3;
								$$ = $1;
							} else
								$$ = $3;
						} 
						| param_id {
							$$ = $1;
						}
						| error ',' param_id { yyerrok; $$ = NULL; }
						| param_id_list ',' error { $$ = NULL; } 
						;

param_id				: ID { 
							yyerrok;
							$$ = newDeclNode(ParamK, yylineno);
                            $$->attr.name = $1.id;
                            $$->lineNum = $1.lineNum;
						} 
						| ID '[' ']' { 
							yyerrok;
							$$ = newDeclNode(ParamK, yylineno);
                            $$->attr.name = $1.id;
                            $$->lineNum = $1.lineNum;
							$$->isArray = 1;
						}
						| error { $$ = NULL; }
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

compound_stmt			: '{' local_declarations statement_list '}' { 
							yyerrok;
							$$ = newStmtNode(CompK, $1.lineNum);
							$$->child[0] = $2;
							$$->child[1] = $3;
						}
						| '{' error statement_list '}' { $$ = NULL; }
						| '{' local_declarations error '}' { yyerrok; $$ = NULL; }
						;

local_declarations		: local_declarations scoped_var_declaration  {
							TreeNode *t = $1;
							if(t != NULL) {
								while(t->sibling != NULL) t = t->sibling;
								t->sibling = $2;
								$$ = $1;
							} else 
								$$ = $2;
						}
						| { $$ = NULL; }
						;

statement_list			: statement_list statement {
							TreeNode *t = $1;
							if(t != NULL) {
								while(t->sibling != NULL) t = t->sibling;
								t->sibling = $2;
								$$ = $1;
							} else 
								$$ = $2;
						}
						| { $$ = NULL; }
						| statement_list error { $$ = NULL; }
						;

expression_stmt			: expression ';' {
							yyerrok;
							$$ = $1;
						}
						| error ';' { yyerrok; $$ = NULL; }
						| ';' {
							yyerrok;
							$$ = NULL;
						}
						;

matched_selection_stmt	: IF '(' simple_expression ')' matched ELSE matched {
							$$ = newStmtNode(IfK, $1.lineNum);
							$$->attr.name = $1.input;
							$$->child[0] = $3;
							$$->child[1] = $5;
							$$->child[2] = $7;
						}
						| IF '(' error ')' matched ELSE matched { $$ = NULL; }
						| error { $$ = NULL; }
						;

unmatched_selection_stmt: IF '(' simple_expression ')' matched {
							$$ = newStmtNode(IfK, $1.lineNum);
							$$->attr.name = $1.input;
							$$->child[0] = $3;
							$$->child[1] = $5;
						}
						| IF '(' simple_expression ')' unmatched {
							$$ = newStmtNode(IfK, $1.lineNum);
							$$->attr.name = $1.input;
							$$->child[0] = $3;
							$$->child[1] = $5;
						}
						| IF '(' simple_expression ')' matched ELSE unmatched {
							$$ = newStmtNode(IfK, $1.lineNum);
							$$->attr.name = $1.input;
							$$->child[0] = $3;
							$$->child[1] = $5;
							$$->child[2] = $7;
						}
						| IF '(' error ')' statement { $$ = NULL; }
						| IF '(' error ')' matched ELSE unmatched { $$ = NULL; }
						;

matched_foreach_stmt	: FOREACH '(' mutable IN simple_expression ')' matched {
							$$ = newStmtNode(ForeachK, $1.lineNum);
							$$->attr.name = $1.input;
							$$->child[0] = $3;
							$$->child[1] = $5;
							$$->child[2] = $7;
						}
						| FOREACH '(' error ')' matched { $$ = NULL; }
						;

unmatched_foreach_stmt	: FOREACH '(' mutable IN simple_expression ')' unmatched {
							$$ = newStmtNode(ForeachK, $1.lineNum);
							$$->attr.name = $1.input;
							$$->child[0] = $3;
							$$->child[1] = $5;
							$$->child[2] = $7;
						}
						| FOREACH '(' error ')' unmatched { $$ = NULL; }
						;

matched_while_stmt		: WHILE '(' simple_expression ')' matched {
							$$ = newStmtNode(WhileK, $1.lineNum);
							$$->attr.name = $1.input;
							$$->child[0] = $3;
							$$->child[1] = $5;
						}
						| WHILE '(' error ')' matched { $$ = NULL; }
						;

unmatched_while_stmt	: WHILE '(' simple_expression ')' unmatched {
							$$ = newStmtNode(WhileK, $1.lineNum);
							$$->attr.name = $1.input;
							$$->child[0] = $3;
							$$->child[1] = $5;
						}
						| WHILE '(' error ')' unmatched { $$ = NULL; }
						;

return_stmt				: RETURN ';' { 
							yyerrok;
							$$ = newStmtNode(ReturnK, $1.lineNum);
							$$->attr.name = $1.input;	
						}
						| RETURN expression ';' {
							yyerrok;
							$$ = newStmtNode(ReturnK, $1.lineNum);
							$$->attr.name = $1.input;	
							$$->child[0] = $2;	
						}
						;

break_stmt				: BREAK ';' {
							yyerrok;
							$$ = newStmtNode(BreakK, $1.lineNum);
							$$->attr.name = $1.input;	
						}
						;

expression				: mutable assignop expression {
							$$ = newExpNode(AssignK, yylineno);
							$$->child[0] = $1;
							$$->child[1] = $3;
							$$->attr.name = $2.input;
							$$->lineNum = $2.lineNum;
						}
						| mutable INC{
							yyerrok;
							$$ = newExpNode(AssignK, yylineno);
							$$->child[0] = $1;
							$$->attr.name = $2.input;
                            $$->lineNum = $2.lineNum;
						}
						| mutable DEC{
							yyerrok;
							$$ = newExpNode(AssignK, yylineno);
							$$->child[0] = $1;
							$$->attr.name = $2.input;
                            $$->lineNum = $2.lineNum;
						}
						| simple_expression {
							$$ = $1; 
						}
						| error assignop expression { yyerrok; $$ = NULL; }
						| mutable assignop error { $$ = NULL; }
						| error assignop error { $$ = NULL; }
						| error INC { yyerrok; $$ = NULL; }
						| error DEC { yyerrok; $$ = NULL; }
						;

assignop				: '=' { $$ = $1; }
						| ADDASS { $$ = $1; }
						| SUBASS { $$ = $1; }
						| MULASS { $$ = $1; }
						| DIVASS { $$ = $1; }
						;

simple_expression		: simple_expression '|' and_expression {
							$$ = newExpNode(OpK, yylineno);
							$$->child[0] = $1;
							$$->child[1] = $3;
							$$->attr.name = $2.input;
                            $$->lineNum = $2.lineNum;
						}
						| and_expression { $$= $1; }
						| error '|' and_expression { yyerrok; $$ = NULL; }
						| simple_expression '|' error { $$ = NULL; }
						| error '|' error { $$ = NULL; }
						;

and_expression			: and_expression '&' unary_rel_expression {
							$$ = newExpNode(OpK, yylineno);
							$$->child[0] = $1;
							$$->child[1] = $3;
							$$->attr.name = $2.input;
                            $$->lineNum = $2.lineNum;
						}
						| unary_rel_expression { $$= $1; }
						| error '&' unary_rel_expression { yyerrok; $$ = NULL; }
						| and_expression '&' error { $$ = NULL; }
						| error '&' error { $$ = NULL; }
						;

unary_rel_expression	: '!' unary_rel_expression {
							$$ = newExpNode(OpK, yylineno);
							$$->child[0] = $2;
							$$->attr.name = $1.input;
                            $$->lineNum = $1.lineNum;
						}
						| rel_expression { $$= $1; }
						| '!' error { $$ = NULL; }
						;

rel_expression			: sum_expression relop sum_expression { 
							$$ = newExpNode(OpK, yylineno);
							$$->child[0] = $1;
							$$->child[1] = $3;
							$$->attr.name = $2.input;
                            $$->lineNum = $2.lineNum;
						}
						| sum_expression { 
							$$ = $1;
						}
						| error relop sum_expression { yyerrok; $$ = NULL; }
						| sum_expression relop error { $$ = NULL; }
						| error relop error { $$ = NULL; }
						;

relop					: LESSEQ { $$ = $1; }
						| '<' { $$ = $1; }
						| '>' { $$ = $1; }
						| GRTEQ { $$ = $1; }
						| EQ { $$ = $1; }
						| NOTEQ { $$ = $1; }
						;

sum_expression			: sum_expression sumop term {
							$$ = newExpNode(OpK, yylineno);
							$$->child[0] = $1;
							$$->child[1] = $3;
							$$->attr.name = $2.input;
                            $$->lineNum = $2.lineNum;
						}
						| term { $$ = $1; }
						| error sumop term { yyerrok; $$ = NULL; }
						| sum_expression sumop error { $$ = NULL; }	
						| error sumop error { $$ = NULL; }
						;

sumop					: '+' { $$ = $1; }
						| '-' { $$ = $1; }
						;

term					: term mulop unary_expression {
							$$ = newExpNode(OpK, yylineno);
							$$->child[0] = $1;
							$$->child[1] = $3;
							$$->attr.name = $2.input;
                            $$->lineNum = $2.lineNum;
						}
						| unary_expression { $$ = $1; }
						| error mulop unary_expression { yyerrok; $$ = NULL; }
						| term mulop error { $$ = NULL; }
						| error mulop error { $$ = NULL; }
						;

mulop					: '*' { $$ = $1; }
						| '/' { $$ = $1; }
						| '%' { $$ = $1; }
						;

unary_expression		: unaryop unary_expression {
							$$ = newExpNode(OpK, yylineno);
							$$->attr.name = $1.input;
                            $$->lineNum = $1.lineNum;
							$$->child[0] = $2;	
						}
						| factor { $$ = $1; }
						| unaryop error { $$ = NULL; }
						;

unaryop					: '-' { $$ = $1; }
						| '*' { $$ = $1; }
						| '?' { $$ = $1; }
						;

factor					: immutable { $$ = $1; }
						| mutable { $$ = $1; }
						;

mutable					: ID {
							yyerrok;
							$$ = newExpNode(IdK, yylineno);
                            $$->attr.name = $1.id;
                            $$->lineNum = $1.lineNum;
						}
						| ID '[' expression ']' {
							yyerrok;
							$$ = newExpNode(IdK, yylineno);
                            $$->attr.name = $1.id;
                            $$->lineNum = $1.lineNum;
							$$->child[0] = $3;
						}
						| ID '[' error { $$ = NULL; }
						| error ']' { yyerrok; $$ = NULL; }
						;

immutable				: '(' expression ')' {
							yyerrok;
							$$ = $2;
						}
						| call { $$ = $1; }
						| constant { $$ = $1; }
						| '(' error { $$ = NULL; }
						;

call					: ID '(' args ')' {
							yyerrok;
							$$ = newExpNode(CallK, $2.lineNum);
                            $$->attr.name = $1.id;
                            $$->lineNum = $1.lineNum; 
							$$->child[0] = $3;	
						}
						| ID '(' error { $$ = NULL; }
						;

args					: arg_list { $$ = $1; }
						| { $$ = NULL; }
						;

arg_list				: arg_list ',' expression {
							yyerrok;
							TreeNode *t = $1;
							if(t != NULL) {
								while(t->sibling != NULL) t = t->sibling;
								t->sibling = $3;
								$$ = $1;
							} else {
								$$ = $3;
							}
						}
						| expression { $$ = $1; }
						| error ',' expression { yyerrok; $$ = NULL; }
						| arg_list ',' error { $$ = NULL; }
						;

constant				: NUMCONST {
							yyerrok;
							$$ = newExpNode(ConstK, yylineno);
							$$->attr.value = $1.nconst;
							$$->type = Integer;
						}
						| CHARCONST {
							yyerrok;
							$$ = newExpNode(ConstK, yylineno);
							$$->attr.cvalue = $1.cconst;
							$$->type = Character;
						}
						| STRINGCONST {
							yyerrok;
							$$ = newExpNode(ConstK, yylineno);
							$$->attr.string = $1.sconst;
							$$->type = String;
						}
						| BOOLCONST {
							yyerrok;
							$$ = newExpNode(ConstK, yylineno);
							$$->attr.value = $1.bconst;
							$$->type = Boolean;
						}
						;
%%

int main(int args, char** argv) {
	initTokenMaps();
	FILE* fptr;

	int c; 
	bool pFlag = false, PFlag = false, oFlag = false;
	while((c = getopt(args, argv, "dpPo")) != EOF) {
		switch(c) {
			case 'd':
				yydebug = 1;
				break;
			case 'p':
				pFlag = true;
				break;
			case 'P':
				PFlag = true;
				break;
			case 'o':
				printf("Incorrect usage of -o flag.\n");
				return 0;
			default:
				break;
		}
	}


	string arg;
	if(yydebug == 1 || pFlag || PFlag) {
		fptr = fopen(argv[2], "r");
		// write tm code to filename.tm
	} else {
		fptr = fopen(argv[1], "r");
		if(argv[2] != NULL && strcmp(argv[2], "-o") == 0) {
			if(argv[3] != NULL) { arg = argv[3]; }
		}
	}

	code = fptr;
	yyin = fptr;

	do {	
		yyparse();
	} while (!feof(yyin));

	// Last arg is the bool NOTYPE or TYPE
	if(pFlag) printTree(stdout, syntaxTree, 0, 0, 0); 
	else if(numErrors == 0) {
		c = scopeAndType(syntaxTree); // semantic analysis (return is global offset)
		if(PFlag) printTree(stdout, syntaxTree, 0, 0, 1);  
	}
	if(PFlag) { printf("Offset for end of global space: %d\n", c); }
	printf("Number of warnings: %d\nNumber of errors: %d\n", numWarnings, numErrors);
	if(numErrors == 0) { codegen(code, syntaxTree, arg); }
	return 0;
	
}

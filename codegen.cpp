#include<stdio.h>
#include<stdlib.h>
#include<string>
#include"emitcode.h"
#include"codegen.h"
#include"globals.h"

using namespace std;

extern FILE *code;

void codegen(FILE* file, TreeNode *t, string arg) {
	code = file;
	if(arg == "-") {} // printf to stdout
	else {} // print to filename provided

	codegenPass(t);
}

void codegenPass(TreeNode *t) {
	while(t != NULL) {
		// EXPRESSIONS	
		if(t->nodekind == ExpK) {
			switch(t->kind.exp) {
				case OpK:
					break;
				case ConstK:
					break;
				case IdK:
					break;
				case CallK:
					break;
				case AssignK:
					break;
			}
		}

		// DECLARATIONS
		if(t->nodekind == DeclK) {
			switch(t->kind.decl) {
				case ParamK:
					break;
				case VarK:
					break;
				case FunK:
					break;
			}
		}
		for(int i = 0; i < MAXCHILDREN; i++) { codegenPass(t->child[i]); }
		t = t->sibling;
	}
}


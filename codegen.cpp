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
	


}


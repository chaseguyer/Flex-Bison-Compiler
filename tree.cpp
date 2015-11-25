#include<string>
#include<stdio.h>
#include<stdlib.h>
#include"globals.h"
#include"tree.h"
#include"struct.h"

#define INDENT indentno+=3
#define UNINDENT indentno-=3

using namespace std;

extern int numErrors, numWarnings;
static int indentno = 0;
int indentIndex = 0, firstRun = 1, cCount = 0;

// Create a new treenode of type stmt
TreeNode *newStmtNode(StmtKind kind, int lineNum) {
    TreeNode *t = (TreeNode *) malloc(sizeof(TreeNode));
    int i;
    if(t == NULL)
        printf("Out of memory error at line\n");
    else {
        for(i = 0; i < MAXCHILDREN; i++) t->child[i] = NULL;
        t->sibling = NULL;
        t->nodekind = StmtK;
        t->kind.stmt = kind;
        t->lineNum = lineNum;
    }
    return t;
}

// Create a new treenode of type exp
TreeNode *newExpNode(ExpKind kind, int lineNum) {
    TreeNode *t = (TreeNode *) malloc(sizeof(TreeNode));
    int i;
    if(t == NULL)
        printf("Out of memory error at line\n");
    else {
        for(i = 0; i < MAXCHILDREN; i++) t->child[i] = NULL;
        t->sibling = NULL;
        t->nodekind = ExpK;
        t->kind.exp = kind;
        t->lineNum = lineNum;
        t->type = Void;
    }
    return t;
}

// Create a new treenode of type decl
TreeNode *newDeclNode(DeclKind kind, int lineNum) {
    TreeNode *t = (TreeNode *) malloc(sizeof(TreeNode));
    int i;
    if(t == NULL)
        printf("Out of memory error at line\n");
    else {
        for(i = 0; i < MAXCHILDREN; i++) t->child[i] = NULL;
        t->sibling = NULL;
        t->nodekind = DeclK;
        t->kind.decl = kind;
        t->lineNum = lineNum;
        t->type = Void;
    }
    return t;
}

/*
string getType(TreeNode *tree) {
	string type;
    if(tree->type == 0) { type = "void"; }
    if(tree->type == 1) { type = "int"; }
    if(tree->type == 2) { type = "bool"; }
    if(tree->type == 3) { type = "char"; }
    if(tree->type == 4) { type = "string"; }
    if(tree->type == 5) { type = "error"; }
    if(tree->type == 6) { type = "undefined type"; }
    if(tree->type == 7) { type = "char or int"; }
    return type;
}
*/

// Used to return a string version of the enum types
char* getType(TreeNode *tree) {
    char *type;
    if(tree->type == 0) { type = strdup("void"); }
    if(tree->type == 1) { type = strdup("int"); }
    if(tree->type == 2) { type = strdup("bool"); }
    if(tree->type == 3) { type = strdup("char"); }
    if(tree->type == 4) { type = strdup("string"); }
    if(tree->type == 5) { type = strdup("error"); }
    if(tree->type == 6) { type = strdup("undefined type"); }
    if(tree->type == 7) { type = strdup("char or int"); }
    return type;
}

static void printSpacing(TreeNode *t, int sCount, int indentIndex) { 
	for(int i = 0; i < (indentno/3); i++) {
		printf("|   ");
	}
	if(cCount > 0) 
		printf("Child: %d  ", cCount-1);

	if(sCount > 0)
		printf("|Sibling: %d  ", sCount);
}

// Things to type: calls, const, id, ops, assign, 
void printTree(FILE* stdout, TreeNode *tree, int sCount, int indentIndex, bool TYPE) {
	if(firstRun != 1) {INDENT;}
	firstRun = 0;
	while(tree != NULL) {
		printSpacing(tree, sCount, indentIndex);
		if(tree -> nodekind == StmtK) {
			switch(tree -> kind.stmt) {
				case IfK:
					printf("If ");
					break;
				case WhileK:
					printf("While ");
					break;
				case ForeachK:
					printf("Foreach ");
					break;
				case CompK:
					printf("Compound");
					if(TYPE) { printf(" with size %d at end of it's declarations", tree->size); }
					break;
				case ReturnK:
					printf("Return ");
					break;
				case BreakK:
					printf("Break ");
					break;
				default:
					printf("I don't know what kind of statement this is.\n");
					break;
			}
		}
		else if(tree -> nodekind == ExpK) {
			switch(tree -> kind.exp) {
				case OpK:
					printf("Op: %s ", tree -> attr.name);
					if(TYPE) printf("Type: %s ", getType(tree));
					break; 	
				case ConstK:
					printf("Const: ");
					if(tree->type == Integer)
						printf("%d ", tree->attr.value);
					if(tree->type == Character) {
						if(tree->isArray == true) {
							printf("\"%s\" ", tree->attr.string);
						} else {
							printf("\'%c\' ", tree->attr.cvalue);
						}
					}
					if(tree->type == Boolean) {
						if(tree->attr.value == 1) {
							printf("true ");
						} else {
							printf("false ");
						}
					}
					if(TYPE) {
						printf("Type: ");
						if(tree->isArray == true && tree->isIndexed == false) printf("is array of ");
						printf("%s ", getType(tree));
					}
					break; 
				case IdK:
					printf("Id: %s ", tree->attr.name);
					if(TYPE) {
						printf("Type: ");
						if(tree->isArray == true && tree->isIndexed == false) printf("is array of ");
						printf("%s ", getType(tree));
					}
					break;
				case CallK:
					printf("Call: %s ", tree -> attr.name);
					if(TYPE) {
						printf("Type: ");
						if(tree->isArray) printf("is array of ");
						printf("%s ", getType(tree));
					}
					break;
				case AssignK:
					printf("Assign: %s ", tree -> attr.name);
					if(TYPE) {
						printf("Type: ");
						if(tree->isArray) printf("is array of ");
						printf("%s ", getType(tree));
					}
					break;
			} 
		}
		else if(tree -> nodekind == DeclK) {
			switch(tree -> kind.decl) {
				case VarK:
					printf("Var %s ", tree -> attr.name);
					if(tree -> isArray == 1)
						printf("is array of ");
					if(!TYPE) {printf("type ");}
					if(tree -> type == Void) 
						printf("void");
					else if(tree -> type == Integer) 
						printf("int");
					else if(tree -> type == Boolean) 
						printf("bool");
					else if(tree -> type == Character) 
						printf("char");

					// Mem loc stuff
					printf(" allocated as");
					if(tree->isGlobal) { printf(" Global"); }
					else { printf(" Local"); }
					if(tree->isStatic) { printf("Static"); }
					printf(" of size %d", tree->size);
					printf(" and data location %d", tree->offset);
					break;
				case FunK:
					printf("Func %s returns type ", tree -> attr.name);
					if(tree -> type == 0) 
						printf("void");
					else if(tree -> type == 1) 
						printf("int");
					else if(tree -> type == 2) 
						printf("bool");
					else if(tree -> type == 3) 
						printf("char");

					// Mem loc stuff
					printf(" allocated as Global");
					printf(" of size %d", tree->size);
					printf(" and exec location %d", tree->offset);



					break;
				case ParamK:
					printf("Param %s ", tree -> attr.name);
					if(tree -> isArray == 1)
						printf("is array of ");
					if(tree -> type == 0) 
						printf("void");
					else if(tree -> type == 1) 
						printf("int");
					else if(tree -> type == 2) 
						printf("bool");
					else if(tree -> type == 3) 
						printf("char");

					// Mem loc stuff
					printf(" allocated as Parameter");
					printf(" of size %d", tree->size);
					printf(" and data location %d", tree->offset);
					break;	
			}
		}
		printf(" [line: %d]\n", tree -> lineNum);

		for(int i = 0; i < MAXCHILDREN; i++) {
			cCount = i+1;
			printTree(stdout, tree -> child[i], 0, 0, TYPE);
			cCount = 0;
		}
		tree = tree -> sibling;
		sCount++;
	}
	UNINDENT;
}

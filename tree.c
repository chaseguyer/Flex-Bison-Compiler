#include<stdio.h>
#include<stdlib.h>
#include"globals.h"
#include"tree.h"
#include"struct.h"

extern int numErrors, numWarnings;

TreeNode * newStmtNode(StmtKind kind, int lineNum) {
    TreeNode *t = (TreeNode *) malloc(sizeof(TreeNode));
    int i;
    if(t == NULL)
        printf("Out of memory error at line\n");
    else {
        for(i = 0; i < MAXCHILDREN; i++) t -> child[i] = NULL;
        t -> sibling = NULL;
        t -> nodekind = StmtK;
        t -> kind.stmt = kind;
        t -> lineNum = lineNum;
    }
    return t;
}

TreeNode * newExpNode(ExpKind kind, int lineNum) {
    TreeNode *t = (TreeNode *) malloc(sizeof(TreeNode));
    int i;
    if(t == NULL)
        printf("Out of memory error at line\n");
    else {
        for(i = 0; i < MAXCHILDREN; i++) t -> child[i] = NULL;
        t -> sibling = NULL;
        t -> nodekind = ExpK;
        t -> kind.exp = kind;
        t -> lineNum = lineNum;
        t -> type = Void;
    }
    return t;
}

TreeNode * newDeclNode(DeclKind kind, int lineNum) {
    TreeNode *t = (TreeNode *) malloc(sizeof(TreeNode));
    int i;
    if(t == NULL)
        printf("Out of memory error at line\n");
    else {
        for(i = 0; i < MAXCHILDREN; i++) t -> child[i] = NULL;
        t -> sibling = NULL;
        t -> nodekind = DeclK;
        t -> kind.decl = kind;
        t -> lineNum = lineNum;
        t -> type = Void;
    }
    return t;
}

static int indentno = 0;
int indentIndex = 0;
int firstRun = 1;
int cCount = 0;

#define INDENT indentno+=3
#define UNINDENT indentno-=3

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
					printf("Compound ");
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
					break; 	
				case ConstK:
					if(tree -> type == Integer) {
						printf("Const: %d ", tree -> attr.value);
						if(TYPE) printf("Type: int "); 
					} else if(tree -> type == Character) {
						printf("Const: \'%c\' ", tree -> attr.cvalue); 
						if(TYPE) printf("Type: char "); 
					} else if(tree -> type == Boolean) {
						if(tree -> attr.value == 1) {
							printf("Const: true ");
							if(TYPE) printf("Type: bool "); 
						} else if(tree -> attr.value == 0) {
							printf("Const: false ");
							if(TYPE) printf("Type: bool "); 
						}
					}
					else if(tree -> type == String) {
						printf("Const: \"%s\" ", tree -> attr.string);
						if(TYPE) printf("Type: string "); 
					}
					break; 
				case IdK:
					printf("Id: %s ", tree -> attr.name);
					break;
				case CallK:
					printf("Call: %s ", tree -> attr.name);
					break;
				case AssignK:
					printf("Assign: %s ", tree -> attr.name);
					break;
			} 
		}
		else if(tree -> nodekind == DeclK) {
			switch(tree -> kind.decl) {
				case VarK:
					printf("Var %s ", tree -> attr.name);
					if(tree -> isArray == 1)
						printf("is array ");
					printf("of type ");
					if(tree -> type == Void) 
						printf("void ");
					else if(tree -> type == Integer) 
						printf("int ");
					else if(tree -> type == Boolean) 
						printf("bool ");
					else if(tree -> type == Character) 
						printf("char ");
					break;
				case FunK:
					printf("Func %s returns type ", tree -> attr.name);
					if(tree -> type == 0) 
						printf("void ");
					else if(tree -> type == 1) 
						printf("int ");
					else if(tree -> type == 2) 
						printf("bool ");
					else if(tree -> type == 3) 
						printf("char ");
					break;
				case ParamK:
					printf("Param %s ", tree -> attr.name);
					if(tree -> isArray == 1)
						printf("is array ");
					printf("of type ");
					if(tree -> type == 0) 
						printf("void ");
					else if(tree -> type == 1) 
						printf("int ");
					else if(tree -> type == 2) 
						printf("bool ");
					else if(tree -> type == 3) 
						printf("char ");
					break;	
			}
		}
		printf("[line: %d]\n", tree -> lineNum);

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

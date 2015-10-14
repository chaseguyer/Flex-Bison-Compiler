#include<stdio.h>
#include<stdlib.h>
#include<iostream>
#include"semantic.h"
#include"symTab.h"

using namespace std;

void pointerPrintStr(void *data) {
    printf("%s ", (char*)(data));
}

TreeNode *ptr;
int childIndex = 0;

bool isFunc, tmpBool;
void treeTraverse(TreeNode *tree, SymbolTable st) {
	int depth = st.depth();
	while(tree != NULL) {
		if(tree->nodekind == StmtK) {
			switch(tree->kind.stmt) {
				case IfK:
					if(tree->child[0] != NULL) { treeTraverse(tree->child[0], st); }
					if(tree->child[1] != NULL) { treeTraverse(tree->child[1], st); }
					if(tree->child[2] != NULL) { treeTraverse(tree->child[2], st); }
					break;
	
				case WhileK:
                    if(tree->child[0] != NULL) { treeTraverse(tree->child[0], st); }
                    if(tree->child[1] != NULL) { treeTraverse(tree->child[1], st); }
					break;

				case ForeachK:
                    if(tree->child[0] != NULL) { treeTraverse(tree->child[0], st); }
                    if(tree->child[1] != NULL) { treeTraverse(tree->child[1], st); }
                    if(tree->child[2] != NULL) { treeTraverse(tree->child[2], st); }
					break;	
		
				case CompK:
					if(isFunc == false) { 
						st.enter("Comp"); 
					} else if(isFunc == true) {
						tmpBool = isFunc;
						isFunc = false;
					}
					if(tree->child[0] != NULL) { treeTraverse(tree->child[0], st); }
					if(tree->child[1] != NULL) { treeTraverse(tree->child[1], st); }
			
					if(tmpBool == false) { st.leave(); }
					break;
				
				case ReturnK:
					// If the func type is int, bool, or char, and there is no return type,
					// print a warning message with the lineno of the func declaration
					//tree->child[0]->attr.type == parent's type
					
					if(tree->child[0] != NULL) { treeTraverse(tree->child[0], st); }
					
					// no arrays returned
					if(tree->child[0] != NULL) {
						if(tree->child[0]->isArray == 1) {
							ptr = (TreeNode *)st.lookup(tree->attr.name);
							errors(tree, 12, ptr);
						}
					}
					break;
			
				case BreakK:	
					// make sure I am in a foreach or while!!
					break;

				default:
					printf("Dont know what this stmtkind is\n");
					break;
			}
		}

		// check to make sure they have the proper type		
		if(tree->nodekind == ExpK) {
			switch(tree->kind.exp) {
				case OpK:
					
					break;

				case ConstK:
					break;

				case CallK:
					break;

				case AssignK:
					break;

				case IdK:
					// check its type, make sure it matches its decl
					// if it is undefined, set it to undefined type
					ptr = (TreeNode *)st.lookup(tree->attr.name);
					if(ptr == NULL) {
						errors(tree, 2, ptr);
					}
					break;
			}
		} 

		if(tree->nodekind == DeclK) {
			switch(tree->kind.decl) {
				case VarK:
					if((st.insert(tree->attr.name, tree)) == false) {
						ptr = (TreeNode *)st.lookup(tree->attr.name);
						errors(tree, 1, ptr);
					}
					break;

				case FunK:
					isFunc = true;
                    if((st.insert(tree->attr.name, tree)) == false) {
                        ptr = (TreeNode *)st.lookup(tree->attr.name);
                        errors(tree, 1, ptr);
                    }
	
					st.enter(tree->attr.name);
					if(tree->child[0] != NULL) { treeTraverse(tree->child[0], st); }	
					if(tree->child[1] != NULL) { treeTraverse(tree->child[1], st); }	
					
					// check for return stuff here?

					st.leave();
					isFunc = false;
					break;

				case ParamK:
					if((st.insert(tree->attr.name, tree)) == false) {
						ptr = (TreeNode *)st.lookup(tree->attr.name);
						errors(tree, 1, ptr);
					}
					break;
			}
		}
		tree = tree->sibling;
	}
	printSymTab(st);
}

void printSymTab(SymbolTable st) {
	st.print(pointerPrintStr);
}

void scopeAndType(TreeNode *tree) {
	SymbolTable st;
	treeTraverse(tree, st);

	ptr = (TreeNode *)st.lookupGlobal("main");
	if(ptr == NULL) { errors(tree, 100, ptr); }
}

void errors(TreeNode *tree, int errorCode, TreeNode *ptr) {
	//DECLARATIONS
	if(errorCode == 1 ) { printf("ERROR(%d): Symbol '%s' is already defined at line %d.\n", tree->lineNum, tree->attr.name, ptr->lineNum); }
	if(errorCode == 2 ) { printf("ERROR(%d): Symbol '%s' is not defined.\n", tree->lineNum, tree->attr.name); }

/*
	//EXPRESSIONS
	printf("ERROR(%d): '%s' requires operands of the same type but lhs is type %s and rhs is %s.\n", lineNum);
	printf("ERROR(%d): '%s' requires operands of type %s but lhs is of type %s.\n", lineNum);
	printf("ERROR(%d): '%s' requires operands of type %s but rhs is of type %s.\n", lineNum);
	printf("ERROR(%d): '%s' requires that if one operand is an array so must the other operand.\n", lineNum);
	printf("ERROR(%d): The operation '%s' does not work with arrays.\n", lineNum);
	printf("ERROR(%d): The operation '%s' only works with arrays.\n", lineNum);
	printf("ERROR(%d): Unary '%s' requires an operand of type %s but was given %s.\n", lineNum);

	//TEST CONDITIONS
	printf("ERROR(%d): Cannot use array as test condition in %s statement.\n", lineNum);
	printf("ERROR(%d): Expecting Boolean test condition in %s statement but got type %s.\n", lineNum);
*/

	//RETURN
	if(errorCode == 12) { printf("ERROR(%d): Cannot return an array.\n", tree->lineNum); }
//	printf("ERROR(%d): Function '%s' at line %d is expecting no return value, but return has return value.\n", lineNum);
//	printf("ERROR(%d): Function '%s' at line %d is expecting to return type %s but got %s.\n", lineNum);
//	printf("ERROR(%d): Function '%s' at line %d is expecting to return type %s but return has no return value.\n", lineNum);
//	printf("WARNING(%d): Expecting to return type %s but function '%s' has no return statement.\n", lineNum);

/*
	//BREAK
	printf("ERROR(%d): Cannot have a break statement outside of loop.\n", lineNum);
	
	//FUNCTION INVOCATION
	printf("ERROR(%d): '%s' is a simple variable and cannot be called.\n", lineNum);
	printf("ERROR(%d): Cannot use function '%s' as a simple variable.\n", lineNum);

	//ARRAY INDEXING
	printf("ERROR(%d): Array index is the unindexed array '%s'.\n", lineNum);
	printf("ERROR(%d): Array '%s' should be indexed by type int but got %s.\n", lineNum);
	printf("ERROR(%d): Cannot index nonarray '%s'.\n", lineNum);

	//PARAMETER LIST
	printf("ERROR(%d): Expecting type %s in parameter %i of call to '%s' defined on line %d but got %s.\n", lineNum);
	printf("ERROR(%d): Expecting array in parameter %i of call to '%s' defined on line %d.\n", lineNum);
	printf("ERROR(%d): Not expecting array in parameter %i of call to '%s' defined on line %d.\n", lineNum);
	printf("ERROR(%d): Too few parameters passed for function '%s' defined on line %d.\n", lineNum);
	printf("ERROR(%d): Too many parameters passed for function '%s' defined on line %d.\n", lineNum);

	//FOREACH
	printf("ERROR(%d): Foreach requires operands of 'in' be the same type but lhs is type %s and rhs array is type %s.\n", lineNum);
	printf("ERROR(%d): If not an array, foreach requires lhs of 'in' be of type int but it is type %s.\n", lineNum);
	printf("ERROR(%d): If not an array, foreach requires rhs of 'in' be of type int but it is type %s.\n", lineNum);
	printf("ERROR(%d): In foreach statement the variable to the left of 'in' must not be an array.\n", lineNum);

	//INITIALIZERS
	printf("ERROR(%d): Array '%s' must be of type char to be initialized, but is of type %s.\n", lineNum);
	printf("ERROR(%d): Initializer for array variable '%s' must be a string, but is of nonarray type %s.\n", lineNum);
	printf("ERROR(%d): Initializer for nonarray variable '%s' of type %s cannot be initialized with an array.\n", lineNum);
	printf("ERROR(%d): Initializer for variable '%s' is not a constant expression.\n", lineNum);
	printf("ERROR(%d): Variable '%s' is of type %s but is being initialized with an expression of type %s.\n", lineNum);
*/
	
	//MISC
	if(errorCode == 100) { printf("ERROR(LINKER): Procedure main is not defined.\n"); }
}

#include<stdio.h>
#include<stdlib.h>
#include<iostream>
#include"semantic.h"
#include"symTab.h"

using namespace std;

void pointerPrintStr(void *data) {
    printf("%s ", (char*)(data));
}

TreeNode *ptr, *lptr, *rptr;

bool isComp = false, isWarning = false;

void scopeAndType(TreeNode *&tree) {
	addIORoutines(tree);
	SymbolTable st;
    treeTraverse(tree, st);

    ptr = (TreeNode *)st.lookupGlobal("main");
    if(ptr == NULL) { errors(tree, 100, ptr); }
}

void addIORoutines(TreeNode *&tree) {
    string routineName[7] = { "output", "outputb", "outputc", "input", "inputb", "inputc", "outnl"};
    ExpType retType[7] = {Void, Void, Void, Integer, Boolean, Character, Void};
    ExpType paramType[7] = {Integer, Boolean, Character, Void, Void, Void, Void};

	TreeNode *array[7]; 

    // add the IO routines with dummy nodes
    for(int i = 0; i < 7; i++) {
        TreeNode *t = newDeclNode(FunK, -1);
	    if(paramType[i] != Void) {
            t->child[0] = newDeclNode(ParamK, -1);
            t->child[0]->attr.name = strdup("*dummy*");
            t->child[0]->type = paramType[i];
        } else {
            t->child[0] = NULL;
        }
        t->child[1] = NULL;
        t->attr.name = strdup(routineName[i].c_str());
        t->type = retType[i];
        t->lineNum = -1;

		array[i] = t;
  }

	for(int i = 0; i < 6; i++) { 
		array[i]->sibling = array[i+1]; 
	}
	array[6]->sibling = tree;
	tree = array[0];
}

void treeTraverse(TreeNode *tree, SymbolTable st) {
	bool newScope = false;
	int depth = st.depth();
	while(tree != NULL) {
		if(tree->nodekind == StmtK) {
			switch(tree->kind.stmt) {
				case IfK:
					// check for boolean test
					for(int i = 0; i < MAXCHILDREN; i++) { treeTraverse(tree->child[i], st); }
					break;
	
				case WhileK:
					// check for boolean test
					for(int i = 0; i < MAXCHILDREN; i++) { treeTraverse(tree->child[i], st); }
					break;

				case ForeachK:
					// should check that the types match the description in the c-Grammar semantics section.  
					for(int i = 0; i < MAXCHILDREN; i++) { treeTraverse(tree->child[i], st); }
                  	break;	
		
				case CompK:
					if(!isComp) { 
						st.enter("comp"); 
						newScope = true;
					}
					isComp = false;
	
					for(int i = 0; i < MAXCHILDREN; i++) { treeTraverse(tree->child[i], st); }
				
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

					tree = setOpType(tree);
					for(int i = 0; i < MAXCHILDREN; i++) { treeTraverse(tree->child[i], st); }
					verifyOpTypes(tree, st, tree->child[0], tree->child[1]);
					break;

				case ConstK:
					break;

				case CallK:
	                if(depth > 1) {
						ptr = (TreeNode *)st.lookup(tree->attr.name);
						if(ptr == NULL) {
						    errors(tree, 2, ptr);
						}
					} else if(depth == 1) {
						ptr = (TreeNode *)st.lookupGlobal(tree->attr.name);
						if(ptr == NULL) {
						    errors(tree, 2, ptr);
						}
					}
					for(int i = 0; i < MAXCHILDREN; i++) { treeTraverse(tree->child[i], st); }
					break;

				case AssignK:
					for(int i = 0; i < MAXCHILDREN; i++) { treeTraverse(tree->child[i], st); }
					break;

				case IdK:
					// check its type, make sure it matches its decl
					// if it is undefined, set it to undefined type
	                ptr = (TreeNode *)st.lookup(tree->attr.name);
                    if(ptr == NULL) {
                        errors(tree, 2, ptr);
                    } else {
						tree->type = ptr->type;
					}
					for(int i = 0; i < MAXCHILDREN; i++) { treeTraverse(tree->child[i], st); }
					break;
			}
		} 

		if(tree->nodekind == DeclK) {
			switch(tree->kind.decl) {
				case ParamK:
				case VarK:
					for(int i = 0; i < MAXCHILDREN; i++) { treeTraverse(tree->child[i], st); }
					if((st.insert(tree->attr.name, tree)) == false) {
						ptr = (TreeNode *)st.lookup(tree->attr.name);
						errors(tree, 1, ptr);
					}
					break;

				case FunK:
                    if((st.insert(tree->attr.name, tree)) == false) {
                        ptr = (TreeNode *)st.lookup(tree->attr.name);
                        errors(tree, 1, ptr);
                    }
					st.enter(tree->attr.name);
					isComp = true;
					newScope = true;

					for(int i = 0; i < MAXCHILDREN; i++) { treeTraverse(tree->child[i], st); }
					break;
			}
		}
		if(newScope) {
			st.leave();
			newScope = false;
		}
		tree = tree->sibling;
	}
	//printSymTab(st);
}

void printSymTab(SymbolTable st) {
	st.print(pointerPrintStr);
}

TreeNode* setTypeString(TreeNode *tree) {
	if(tree->type == 0) {
		tree->attr.typeStr = strdup("void");
		return tree;
	}
	if(tree->type == 1) {
		tree->attr.typeStr = strdup("int");
        return tree;
    }
	if(tree->type == 2) {
		tree->attr.typeStr = strdup("bool");
        return tree;
    }
	if(tree->type == 3) {
		tree->attr.typeStr = strdup("char");
        return tree;
    }
	if(tree->type == 4) {
		tree->attr.typeStr = strdup("string");
        return tree;
    }
	if(tree->type == 5) {
		tree->attr.typeStr = strdup("error");
        return tree;
    }
	if(tree->type == 6) {
		tree->attr.typeStr = strdup("undefined");
        return tree;
    }
	return tree;
}

TreeNode* setOpType(TreeNode* t) {
	string op[] = {">", "<", "+", "|", "==", "!=", "=", "++", "--"};
	// the void is for '=' which returns type of lhs
	ExpType opRetType[] = {Boolean, Boolean, Integer, Boolean, Boolean, Boolean, Void, Integer, Integer};
	if(strcmp(tree->attr.name, ">") == 0)

	return t;
}

// ExpType order -> Void, Integer, Boolean, Character, Error, Undefined
void verifyOpTypes(TreeNode *tree, SymbolTable st, TreeNode *lhs, TreeNode *rhs) {
/*
	   '>' takes Integers and returns a Boolean.
        + takes Integers and returns an Integer.
        | takes Booleans and returns a Boolean.
        The operators == and !=, take arguments that are of the same type (both Boolean or both
            Integer) and return a Boolean.
        = take arguments that are of the same type and returns the type of the lhs. This means
            if there is an undefined operand, the lhs operand even if undefined is the type of
            the assignment. This is because assignment is an expression and can be used in
            cascaded assignment like: a = b = c = 314159265
        ++ and -- takes in Integer and returns and Integer. It is not like in C or C++.
*/

	printf("LHS:%s     OP:%s     RHS:%s\n", lhs->attr.name, tree->attr.name, rhs->attr.name);
	printf("   :%u       :%u        :%u\n\n", lhs->type, tree->type, rhs->type);


	/*
	if(strcmp(tree->attr.name, ">") == 0) {
		if(lhs != 1 || rhs != 1 * and not error type *) {

			
			// 2 different types: 3
			if(lhs != 1 && rhs != 1) {
				errors(tree, 3, ptr);
			
				// set error type
			}

			// lhs wrong type: 4
			else if(lhs != 1 && rhs == 1) {
				errors(tree, 4, ptr);
			} 

			// rhs wrong type: 5
			else if(lhs == 1 && rhs != 1) {
				errors(tree, 5, ptr);
			}

			// lhs is array and rhs is not or vice versa: 5
			else if((tree->child[0]->isArray == 1 && tree->child[1]->isArray == 0) || (tree->child[0]->isArray == 0 && tree->child[1]->isArray == 1)) {
				errors(tree, 6, ptr);
			}
			
			// op does not work with arrays: 7
			// op only works with arrays: 8
			// unaryop requires type x but was given type y: 9
			
		}
	}
	*/
}


void errors(TreeNode *tree, int errorCode, TreeNode *p) {
	if(!isWarning) {
		numErrors++;
	} else {
		numWarnings++;
		isWarning = false;
	}
	//DECLARATIONS
	if(errorCode == 1 ) { printf("ERROR(%d): Symbol '%s' is already defined at line %d.\n", tree->lineNum, tree->attr.name, p->lineNum); }
	if(errorCode == 2 ) { printf("ERROR(%d): Symbol '%s' is not defined.\n", tree->lineNum, tree->attr.name); }

	//EXPRESSIONS
	if(errorCode == 3) { printf("ERROR(%d): '%s' requires operands of the same type but lhs is type %s and rhs is %s.\n", tree->lineNum, tree->attr.name, tree->child[0]->attr.typeStr, tree->child[1]->attr.typeStr); }
	if(errorCode == 4) { printf("ERROR(%d): '%s' requires operands of type %s but lhs is of type %s.\n", tree->lineNum, tree->attr.name, "int", tree->child[0]->attr.typeStr); }
	if(errorCode == 5) { printf("ERROR(%d): '%s' requires operands of type %s but rhs is of type %s.\n", tree->lineNum, tree->attr.name, "int", tree->child[1]->attr.typeStr); }
	if(errorCode == 6) { printf("ERROR(%d): '%s' requires that if one operand is an array so must the other operand.\n", tree->lineNum, tree->attr.name); }
//	if(errorCode == 7) { printf("ERROR(%d): The operation '%s' does not work with arrays.\n", lineNum);
//	if(errorCode == 8) { printf("ERROR(%d): The operation '%s' only works with arrays.\n", lineNum);
//	if(errorCode == 9) { printf("ERROR(%d): Unary '%s' requires an operand of type %s but was given %s.\n", lineNum);

/*
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

#include<stdio.h>
#include<stdlib.h>
#include<iostream>
#include"semantic.h"
#include"symTab.h"

/*	*	*	*	*	*/

// Symbol Table Print Functions

void pointerPrintStr(void *data) {
    printf("%s ", (char*)(data));
}
void printSymTab(SymbolTable st) {
	st.print(pointerPrintStr);
}

/*	*	*	*	*	*/

TreeNode *ptr;
bool isComp = false, isWarning = false, isSet = false, isLoop = false, isReturn = false;
char* funcName;
int paramNum = 0;

// Specifies scopes, types tree members, handles errors
void scopeAndType(TreeNode *&tree) {
	addIORoutines(tree);
	SymbolTable st;
    treeTraverse(tree, st);

    ptr = (TreeNode *)st.lookupGlobal("main");
    if(ptr == NULL) { errors(tree, 37, ptr); }
}

// Adds IO routine names into tree to reserve the names
void addIORoutines(TreeNode *&tree) {
    string routineName[7] = { "input", "output", "inputb", "outputb", "inputc", "outputc", "outnl"};
    ExpType retType[7] = {Integer, Void, Boolean, Void, Character, Void, Void};
    ExpType paramType[7] = {Void, Integer, Void, Boolean, Void, Character, Void};

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

/*	*	*	*	*

TODO

Returning void

ops that dont work with arrays

cannot use func as a simple var

array should be indexed bt x but got y

= requires operands of same type

error line no's (errloc)


*	*	*	*	*/

TreeNode *funcParams, *callParams;

// Recursively called function that handles all siblings and their children
void treeTraverse(TreeNode *tree, SymbolTable st) {
	bool newScope = false;
	int depth = st.depth();
	TreeNode *func, *tmp; 
	int fCount = 0, pCount = 0; // for counting number of parameters
	bool fNull = false, cNull = false; 
	while(tree != NULL) {
		tree->isSimple = false;

		/*** STATEMENT KIND ***/
		if(tree->nodekind == StmtK) {
			switch(tree->kind.stmt) {
				case IfK:
					// check for boolean test
					for(int i = 0; i < MAXCHILDREN; i++) { treeTraverse(tree->child[i], st); }
					tree->attr.name = strdup("if");
                    if(tree->child[0] != NULL) {
                        // if(errorCode == 11) { printf("ERROR(%d): Expecting Boolean test condition in %s statement but got type %s.\n", tree->lineNum, "if", "char"); }
                        if(tree->child[0]->type != Boolean) {
                            errors(tree, 11, tree->child[0]);
                        }
                        // if(errorCode == 10) { printf("ERROR(%d): Cannot use array as test condition in %s statement.\n", tree->lineNum, "if"); }
                        if(tree->child[0]->isArray == true) {
                            errors(tree, 10, tree->child[0]);
                        }
                    }
					break;
	
				case WhileK:
					// check for boolean test
					isLoop = true;
					for(int i = 0; i < MAXCHILDREN; i++) { treeTraverse(tree->child[i], st); }
					tree->attr.name = strdup("while");
					if(tree->child[0] != NULL) {
						// if(errorCode == 11) { printf("ERROR(%d): Expecting Boolean test condition in %s statement but got type %s.\n", tree->lineNum, "if", "char"); }
						if(tree->child[0]->type != Boolean) {
							errors(tree, 11, tree->child[0]);
						}
						// if(errorCode == 10) { printf("ERROR(%d): Cannot use array as test condition in %s statement.\n", tree->lineNum, "if"); }
						if(tree->child[0]->isArray == true) {
							errors(tree, 10, tree->child[0]);
						}
					}
					isLoop = false;
					break;

				case ForeachK:
					isLoop = true;
					for(int i = 0; i < MAXCHILDREN; i++) { treeTraverse(tree->child[i], st); }
					// lhs cannot be an array of any type but int (29)
					if(tree->child[0]->isArray == true) {
						if(tree->child[0]->type != Integer) {
							errors(tree, 29, ptr);
						}
						errors(tree, 31, ptr);
					}

					// rhs of IN can either be an INT or an INT or BOOL array (30)
					if(tree->child[1]->isArray == false) {
						if(tree->child[1]->type != Integer && tree->child[1]->type != Undefined) {
							errors(tree, 30, ptr);
						}
					} else if(tree->child[1]->isArray == true) {
						// types must be the same: bool IN bool | int IN int (28)
						if(tree->child[0]->type != tree->child[1]->type) {
							errors(tree, 28, ptr);
						}	
					}
					isLoop = false;
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
					isReturn = true;
					for(int i = 0; i < MAXCHILDREN; i++) { treeTraverse(tree->child[i], st); }
					func = (TreeNode *)st.lookup(funcName);
				
					// expecting return type x but has no return val: 15  
					if(tree->child[0] == NULL && func->type != Void) { // no return val
		                errors(tree, 15, func);	
					}

					if(tree->child[0] != NULL) {
						TreeNode *child = tree->child[0];

						// no arrays returned: 12
						if(child->isArray == true)
							errors(tree, 12, func);

						// expecting no return val but got x: 13
						if(func->type == Void && child->type != Void) 
							errors(tree, 13, func);

						// expecting return type x but got y: 14
						//if(child->nodekind == ExpK && child->kind.exp == CallK) {
							
						//} else 
						if(func->type != child->type && child->type != Undefined)
							errors(tree, 14, func);	
					}
					break;
			
				case BreakK:
					if(isLoop == false) 
						errors(tree, 17, ptr); 
					break;

				default:
					printf("Dont know what this stmtkind is\n");
					break;
			}
		}

		/*** EXPRESSION KIND ***/
		if(tree->nodekind == ExpK) {
			switch(tree->kind.exp) {
				case OpK:
				case AssignK:
					ExpType expectLHS, expectRHS;
					isSet = true;
					tree = getOpTypes(expectLHS, expectRHS, tree);
					isSet = false;
					for(int i = 0; i < MAXCHILDREN; i++) { treeTraverse(tree->child[i], st); }
					verifyOpTypes(tree, st);
					break;

				case ConstK:
					break;

				case CallK:
					ptr = (TreeNode *)st.lookup(tree->attr.name);
					if(ptr == NULL) {
					    errors(tree, 2, ptr);
						for(int i = 0; i < MAXCHILDREN; i++) { treeTraverse(tree->child[i], st); }
					} else { // with ptr being parent function...
						for(int i = 0; i < MAXCHILDREN; i++) { treeTraverse(tree->child[i], st); }
						ptr = (TreeNode *)st.lookup(tree->attr.name);
						tree->type = ptr->type;

						/*
						// if(errorCode == 18) { printf("ERROR(%d): '%s' is a simple variable and cannot be called.\n", tree->lineNum, p->attr.name); }
						if(ptr->nodekind == DeclK && tree->kind.exp == VarK) {
							errors(tree, 18, ptr);
						}
						*/

						// assign func and call parameters and reinitialize flags
						if(ptr->child[0] != NULL) {
							funcParams = ptr->child[0];
							fNull = false;
							//fCount = 0;
							fCount = 1;
						} else {
							funcParams = NULL;
						}
						if(tree->child[0] != NULL) { 
							callParams = tree->child[0];
							cNull = false;
							//pCount = 0;
							pCount = 1;
						} else {
							callParams = NULL;	
						}
						if(funcParams != NULL) {
							while(funcParams != NULL) {
								//fCount++;
								if(callParams != NULL) {
									//pCount++;
									//if(errorCode == 23) { printf("ERROR(%d): Expecting type %s in parameter %i of call to '%s' defined on line %d but got %s.\n", tree->lineNum); }
									if(funcParams->type != callParams->type) {
										paramNum = pCount;
										errors(tree, 23, ptr);
									}
									//if(errorCode == 24) { printf("ERROR(%d): Expecting array in parameter %i of call to '%s' defined on line %d.\n", tree->lineNum); }
									if(funcParams->isArray == true && callParams->isArray == false) {
										paramNum = pCount;
										errors(tree, 24, ptr);
									//if(errorCode == 25) { printf("ERROR(%d): Not expecting array in parameter %i of call to '%s' defined on line %d.\n", tree->lineNum); }
									} else if(funcParams->isArray == false && callParams->isArray == true) {
										paramNum = pCount;
										errors(tree, 25, ptr);
									}
									
									// Now for next is NULL check
									if(callParams->sibling != NULL) {
										callParams = callParams->sibling;
										pCount++;
									} else {
										cNull = true;
									}
									if(funcParams->sibling != NULL) {
										funcParams = funcParams->sibling;
										fCount++;
									} else {
										fNull = true;
										break;
									}
								} else {cNull = true; }	
								if(fNull != cNull) {
									//if(fNull == true && cNull == false) {
									if(fCount < pCount) {
										//if(errorCode == 27) { printf("ERROR(%d): Too many parameters passed for function '%s' defined on line %d.\n", tree->lineNum); }
										errors(tree, 27, ptr);
										break;
									//} else if(fNull == false && cNull == true) {
									} else if(fCount > pCount) {
										//if(errorCode == 26) { printf("ERROR(%d): Too few parameters passed for function '%s' defined on line %d.\n", tree->lineNum); }
										errors(tree, 26, ptr);
										break;
									}
								}
							}
						} else if(callParams != NULL) { 
							//if(errorCode == 27) { printf("ERROR(%d): Too many parameters passed for function '%s' defined on line %d.\n", tree->lineNum); }
							errors(tree, 27, ptr);
						}
					}
					break;

				case IdK:
					// check its type, make sure it matches its decl
					// if it is undefined, set it to undefined type
	                ptr = (TreeNode *)st.lookup(tree->attr.name);
                    if(ptr == NULL) { // original decl
                        errors(tree, 2, ptr);
						tree->type = Undefined;
						for(int i = 0; i < MAXCHILDREN; i++) { treeTraverse(tree->child[i], st); }
					} else {
						tree->type = ptr->type;
						tree->isArray = ptr->isArray;
						for(int i = 0; i < MAXCHILDREN; i++) { treeTraverse(tree->child[i], st); }

						

						// 22: Cannot index nonarray '%s'.\n (x[496]; or *x; when x is not declared) 
						if(tree->isArray == false) { // Id isnt an array
							if(tree->child[0] != NULL) { // but is being indexed
								errors(tree, 22, ptr);
							}						
						}
						if(tree->isArray == true) {
							if(tree->child[0] != NULL) { // Id has children
								if(tree->child[0]->attr.name != NULL) { // child has an Id itself
									// 20: Array index is the unindexed array '%s'.\n (aa[aa])
									if(tree->attr.name == tree->child[0]->attr.name) { // this cannot be correct...
										errors(tree, 20, ptr);
									}
								}
								else if(tree->child[0]->attr.name == NULL) { // is a const
									// 21: Array '%s' should be indexed by type int but got %s.\n (array init as in x but given y type)
									if(tree->child[0]->type != Integer) {
										errors(tree, 21, tree->child[0]);
									}
								} else {
									tree->isIndexed = true;
									//tree->isArray = false;
								}
							}	
						}
					}
					break;
			}
		} 

		/*** DECLARATION KIND ***/
		if(tree->nodekind == DeclK) {
			switch(tree->kind.decl) {
				case ParamK: 
				case VarK: 
					for(int i = 0; i < MAXCHILDREN; i++) { treeTraverse(tree->child[i], st); }
					if((st.insert(tree->attr.name, tree)) == false) {
						ptr = (TreeNode *)st.lookup(tree->attr.name);
						errors(tree, 1, ptr);
					}

					if(tree->child[0] != NULL) { // decl is being init w/ expression
						if(tree->isArray == true) {
							if(tree->type != tree->child[0]->type) {
								// 33: Initializer for array variable '%s' must be a string, but is of nonarray type %s.\n
								if(tree->type == Character && tree->child[0]->type != String) {
									errors(tree, 33, tree->child[0]); 
								}
								// 32: Array '%s' must be of type char to be initialized, but is of type %s.\n
								else if(tree->type != Character && tree->child[0]->type == String) {
									errors(tree, 32, tree->child[0]);	
								}
								// For array char[]:string case
								else if(tree->type == Character && tree->child[0]->type == String) {	// cheap fix to a harder problem?
								}
								// 35: Initializer for variable '%s' is not a constant expression.\n
								else if(tree->child[0]->attr.name != NULL) {
									errors(tree, 35, tree->child[0]);
								}
							} else {
								tree->isIndexed = true;
								//tree->isArray = false;
							}
						}
						else if(tree->isArray == false) {
							// 34: Initializer for nonarray variable '%s' of type %s cannot be initialized with an array.\n
							if(tree->child[0]->isArray == true) {
								errors(tree, 34, tree->child[0]);
							}
							// 36: Variable '%s' is of type %s but is being initialized with an expression of type %s.\n
							if(tree->type != tree->child[0]->type) {
								errors(tree, 36, tree->child[0]); 
							}
						}
					}		
						
					// 35: Initializer for variable '%s' is not a constant expression.\n
					//if(tree->child[0]->type ) {}
					break;

				case FunK:
                    if((st.insert(tree->attr.name, tree)) == false) {
                        ptr = (TreeNode *)st.lookup(tree->attr.name);
                        errors(tree, 1, ptr);
                    }

					funcName = tree->attr.name; // for checking return value
					st.enter(tree->attr.name);
					isComp = true;
					newScope = true;

					for(int i = 0; i < MAXCHILDREN; i++) { treeTraverse(tree->child[i], st); }
					if(isReturn == false && tree->lineNum != -1 && tree->type != Void) {
						isWarning = true;
						errors(tree, 16, ptr);
					} else {
						isReturn = false;
					}	
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

// 1. Gets the expected types of op children.  2. Sets the type of operator to expected type 
TreeNode* getOpTypes(ExpType &expectLHS, ExpType &expectRHS, TreeNode *tree) {
	std::string opName(tree->attr.name);

	// BINARY OPS
	if(tree->child[1] != NULL) {
		if(opName == "+" || opName == "-" || opName == "*" || opName == "/" || opName == "%" || opName == "+=" || opName == "-=" || opName == "*=" || opName == "/=") {
			expectLHS = Integer;
			expectRHS = Integer;
			if(isSet) { tree->type = Integer; }
			return tree;
		}
		if(opName == "==" || opName == "!=") {
			expectLHS = Undefined;
			expectRHS = Undefined;
			if(isSet) { tree->type = Boolean; }
		}
		if(opName == ">" || opName == "<" || opName == ">=" || opName == "<=") {
			expectLHS = CharOrInt;
			expectRHS = CharOrInt;
			if(isSet) { tree->type = Boolean; }
			return tree;
		}
		if(opName == "|" || opName == "&") {
			expectLHS = Boolean;
			expectRHS = Boolean;
			if(isSet) { tree->type = Boolean; }
			return tree;
		}
		if(opName == "=") {
			expectLHS = Undefined;
			expectRHS = Undefined;
			if(isSet) { tree->type = expectLHS; }
			return tree;
		}
	}

	// UNARY OPS
	else if(tree->child[1] == NULL) {
	    if(opName == "?" || opName == "-" || opName == "++" || opName == "--") {
	        tree->isUnary = true;
			expectLHS = Integer;
			if(isSet) { tree->type = Integer; }
			return tree;
	    }
	    if(opName == "*") {
	        tree->isUnary = true;
			expectLHS = Undefined;
			if(isSet) { tree->type = Integer; }
			return tree;
	    }
		if(opName == "!") {
			tree->isUnary = true;
			expectLHS = Boolean;
			if(isSet) { tree->type = Boolean; }
			return tree;
		}
	}
	return tree;
}

// Compares given types to expected types and reports errors if there are any 
void verifyOpTypes(TreeNode *tree, SymbolTable st) {
	TreeNode *tmp, *error;
	ExpType expectLHS, expectRHS, tmpType;
	tree = getOpTypes(expectLHS, expectRHS, tree);

	// BINARY EXPRESSIONS
	if(tree->isUnary != true) {
		TreeNode *lptr = tree->child[0];
		TreeNode *rptr = tree->child[1];
	
		ExpType lhs = lptr->type;
		ExpType rhs = rptr->type;

		/*
			// Quick check to make sure we are not using a function as a simple variable	
			if(lptr->attr.name != NULL) {
				ptr = (TreeNode*)st.lookup(lptr->attr.name);
				if(ptr != NULL) {
					// if(errorCode == 19) { printf("ERROR(%d): Cannot use function '%s' as a simple variable.\n", tree->lineNum, p->attr.name); }
					if(ptr->nodekind == DeclK && tree->kind.exp == FunK) {
						errors(tree, 19, ptr);
					}
				}
			}

            if(rptr->attr.name != NULL) {
                ptr = (TreeNode*)st.lookup(rptr->attr.name);
                if(ptr != NULL) {
					// if(errorCode == 19) { printf("ERROR(%d): Cannot use function '%s' as a simple variable.\n", tree->lineNum, p->attr.name); }
                    if(ptr->nodekind == DeclK && tree->kind.exp == FunK) {
                        errors(tree, 19, ptr);
                    }
                }
            }
		*/

		// Let's deal with just the =, !=, and ==
		if(expectLHS == Undefined && expectRHS == Undefined) {
			// binary ops that deal with arrays: =, !=, ==
			// lhs is array and rhs is not or vice versa: 6
			if(strcmp(tree->attr.name, "=") == 0) {
				tree->type = lhs;
				if(((lptr->isArray == true && rptr->isArray == false) || (lptr->isArray == false && rptr->isArray == true ))) {
					errors(tree, 6, ptr); 
				}
			}
			if(lhs != rhs) {
				if(lhs == Character && rhs == String) {// cheap fix to a harder problem?
				}
				else if(lhs != Undefined && rhs != Undefined) 
					errors(tree, 3, ptr);
			}
		} else { // to handle >, <, >=, and <=
			if(expectLHS == CharOrInt) {
                if(lhs != rhs && (lhs != Undefined && rhs != Undefined)) {
                    errors(tree, 3, ptr);
                } 
				if(lhs != Character && lhs != Integer && (lhs != Undefined && rhs != Undefined)) {
					tmpType = tree->type;
					tree->type = CharOrInt;
					errors(tree, 4, ptr);
					tree->type = tmpType;
				}
				if(rhs != Character && rhs != Integer && (lhs != Undefined && rhs != Undefined)) {
					tmpType = tree->type;
					tree->type = CharOrInt;
					errors(tree, 5, ptr);
					tree->type = tmpType;
				}
			}
			else { // to handle everything else
				if(lhs != expectLHS && lhs != Undefined) { 
					errors(tree, 4, ptr);
				}
				if(rhs != expectRHS && rhs != Undefined) {
					errors(tree, 5, ptr);
				}
			}
		}
	}

	// UNARY EXPRESSIONS
	if(tree->isUnary == true) {
		TreeNode *lptr = tree->child[0];
		ExpType lhs = lptr->type;

		if(strcmp(tree->attr.name, "*") == 0) {
			// for op isArray == false and yet it uses *: 8
			if(lptr->isArray == false) {
				errors(tree, 8, ptr);
			}
	
		} else {
            // expects %s but is getting %s: 9
            if(expectLHS != lhs) {
                errors(tree, 9, ptr);
            }
			// 7: The operation '%s' does not work with arrays.\n
			if(lptr->isArray == true) {
				errors(tree, 7, ptr);
			}
		}
	}
}

// Error messages who are sent their corresponding error code to print
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
	if(errorCode == 3) { printf("ERROR(%d): '%s' requires operands of the same type but lhs is type %s and rhs is %s.\n", tree->lineNum, tree->attr.name, getType(tree->child[0]), getType(tree->child[1])); }
	if(errorCode == 4) { printf("ERROR(%d): '%s' requires operands of type %s but lhs is of type %s.\n", tree->lineNum, tree->attr.name, getType(tree), getType(tree->child[0])); }
	if(errorCode == 5) { printf("ERROR(%d): '%s' requires operands of type %s but rhs is of type %s.\n", tree->lineNum, tree->attr.name, getType(tree), getType(tree->child[1])); }
	if(errorCode == 6) { printf("ERROR(%d): '%s' requires that if one operand is an array so must the other operand.\n", tree->lineNum, tree->attr.name); }
	if(errorCode == 7) { printf("ERROR(%d): The operation '%s' does not work with arrays.\n", tree->lineNum, tree->attr.name); }
	if(errorCode == 8) { printf("ERROR(%d): The operation '%s' only works with arrays.\n", tree->lineNum, tree->attr.name); }
	if(errorCode == 9) { printf("ERROR(%d): Unary '%s' requires an operand of type %s but was given %s.\n", tree->lineNum, tree->attr.name, getType(tree), getType(tree->child[0])); }

	//TEST CONDITIONS
	if(errorCode == 10) { printf("ERROR(%d): Cannot use array as test condition in %s statement.\n", tree->lineNum, tree->attr.name); }
	if(errorCode == 11) { printf("ERROR(%d): Expecting Boolean test condition in %s statement but got type %s.\n", tree->lineNum, tree->attr.name, getType(p)); }

	//RETURN - remember that p is the original function while tree is the return
	if(errorCode == 12) { printf("ERROR(%d): Cannot return an array.\n", tree->lineNum); }
	if(errorCode == 13) { printf("ERROR(%d): Function '%s' at line %d is expecting no return value, but return has return value.\n", tree->lineNum, p->attr.name, p->lineNum); }
	if(errorCode == 14) { printf("ERROR(%d): Function '%s' at line %d is expecting to return type %s but got %s.\n", tree->lineNum, p->attr.name, p->lineNum, getType(p), getType(tree->child[0])); }
	if(errorCode == 15) { printf("ERROR(%d): Function '%s' at line %d is expecting to return type %s but return has no return value.\n", tree->lineNum, p->attr.name, p->lineNum, getType(p)); }
	if(errorCode == 16) { printf("WARNING(%d): Expecting to return type %s but function '%s' has no return statement.\n", tree->lineNum, getType(tree), tree->attr.name); }

	//BREAK
	if(errorCode == 17) { printf("ERROR(%d): Cannot have a break statement outside of loop.\n", tree->lineNum); }
	
	//FUNCTION INVOCATION
	if(errorCode == 18) { printf("ERROR(%d): '%s' is a simple variable and cannot be called.\n", tree->lineNum, p->attr.name); }
	if(errorCode == 19) { printf("ERROR(%d): Cannot use function '%s' as a simple variable.\n", tree->lineNum, p->attr.name); }

	//ARRAY INDEXING
    if(errorCode == 20) { printf("ERROR(%d): Array index is the unindexed array '%s'.\n", tree->lineNum, p->attr.name); }
    if(errorCode == 21) { printf("ERROR(%d): Array '%s' should be indexed by type int but got %s.\n", tree->lineNum, tree->attr.name, getType(p)); }
    if(errorCode == 22) { printf("ERROR(%d): Cannot index nonarray '%s'.\n", tree->lineNum, tree->attr.name); }

	//PARAMETER LIST
	if(errorCode == 23) { printf("ERROR(%d): Expecting type %s in parameter %i of call to '%s' defined on line %d but got %s.\n", tree->lineNum, getType(funcParams), paramNum, p->attr.name, p->lineNum, getType(callParams)); }
	if(errorCode == 24) { printf("ERROR(%d): Expecting array in parameter %i of call to '%s' defined on line %d.\n", tree->lineNum, paramNum, p->attr.name, p->lineNum); }
	if(errorCode == 25) { printf("ERROR(%d): Not expecting array in parameter %i of call to '%s' defined on line %d.\n", tree->lineNum, paramNum, p->attr.name, p->lineNum); }
	if(errorCode == 26) { printf("ERROR(%d): Too few parameters passed for function '%s' defined on line %d.\n", tree->lineNum, tree->attr.name, p->lineNum); }
	if(errorCode == 27) { printf("ERROR(%d): Too many parameters passed for function '%s' defined on line %d.\n", tree->lineNum, tree->attr.name, p->lineNum); }

	//FOREACH
	if(errorCode == 28) { printf("ERROR(%d): Foreach requires operands of 'in' be the same type but lhs is type %s and rhs array is type %s.\n", tree->lineNum, getType(tree->child[0]), getType(tree->child[1])); }
	if(errorCode == 29) { printf("ERROR(%d): If not an array, foreach requires lhs of 'in' be of type int but it is type %s.\n", tree->lineNum, getType(tree->child[0])); }
	if(errorCode == 30) { printf("ERROR(%d): If not an array, foreach requires rhs of 'in' be of type int but it is type %s.\n", tree->lineNum, getType(tree->child[1])); }
	if(errorCode == 31) { printf("ERROR(%d): In foreach statement the variable to the left of 'in' must not be an array.\n", tree->lineNum); }

	//INITIALIZERS
	if(errorCode == 32) { printf("ERROR(%d): Array '%s' must be of type char to be initialized, but is of type %s.\n", tree->lineNum, tree->attr.name, getType(tree)); }
	if(errorCode == 33) { printf("ERROR(%d): Initializer for array variable '%s' must be a string, but is of nonarray type %s.\n", tree->lineNum, tree->attr.name, getType(p)); }
	if(errorCode == 34) { printf("ERROR(%d): Initializer for nonarray variable '%s' of type %s cannot be initialized with an array.\n", tree->lineNum, tree->attr.name, getType(tree)); }
	if(errorCode == 35) { printf("ERROR(%d): Initializer for variable '%s' is not a constant expression.\n", tree->lineNum, tree->attr.name); }
	if(errorCode == 36) { printf("ERROR(%d): Variable '%s' is of type %s but is being initialized with an expression of type %s.\n", tree->lineNum, tree->attr.name, getType(tree), getType(p)); }
	
	//MISC
	if(errorCode == 37) { printf("ERROR(LINKER): Procedure main is not defined.\n"); }
}

#include<stdio.h>
#include<stdlib.h>
#include<string>
#include"semantic.h"
#include"symTab.h"

using namespace std;

/*	 *	 *	 *	 *	 */

// Symbol Table Print Functions

void pointerPrintStr(void *data) {
    printf("%s ", (char*)(data));
}
void printSymTab(SymbolTable st) {
	st.print(pointerPrintStr);
}

/*	 *	 *	 *	 *	 */

TreeNode *ptr, *child;					// handy ptr for various purposes
bool isComp = false, isWarning = false, isSet = false, isReturn = false, isLoop = false, isFunc = false; // various flags for error handling
int globalOffset = 0, localOffset = 0;	// used in calculating location in memory
string funcName;							// used to pass around a call's original function decl
int paramNum = 0;						// used when comparing params of call to params of original decl
int loopDepth = 1;						// used to figure out what depth we are at in a loop

// Specifies scopes, types tree members, handles errors
int scopeAndType(TreeNode *&tree) {
	addIORoutines(tree);
	SymbolTable st;
    treeTraverse(tree, st);

    ptr = (TreeNode *)st.lookupGlobal("main");
    if(ptr == NULL) { errors(tree, 37, ptr); }

	return globalOffset;
}

// Adds IO routine names into tree to reserve the names
void addIORoutines(TreeNode *&tree) {
    string routineName[7] = {"input", "output", "inputb", "outputb", "inputc", "outputc", "outnl"};
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

	// chain the members of the array together
	for(int i = 0; i < 6; i++) { 
		array[i]->sibling = array[i+1]; 
	}
	array[6]->sibling = tree; // tie bottom of array to top of the tree
	tree = array[0]; // head of the syntax tree is the first IO func
}


// Recursively called function that handles all siblings and their children
void treeTraverse(TreeNode *tree, SymbolTable st) {
	bool newScope = false;					// bool for checking whether we entered a new scope or not
	int depth = st.depth(), paramCount = 0; // depth of scope; number of params in a function decl
	TreeNode *funcParams, *callParams;		// TreeNodes to store values in
	TreeNode *func, *tmp;					// TreeNodes to store values in
	int locOff = localOffset;				// a local local offset variable 
	ExpType expectLHS, expectRHS;			// used when comparing operators' children

	while(tree != NULL) {

		/*** STATEMENT KIND ***/
		if(tree->nodekind == StmtK) {
			switch(tree->kind.stmt) {
				case IfK:
					for(int i = 0; i < MAXCHILDREN; i++) { treeTraverse(tree->child[i], st); }
					tree->attr.name = strdup("if");
                    if(tree->child[0] != NULL) {
                        // 11: Expecting Boolean test condition in %s statement but got type %s.\n
                        if(tree->child[0]->type != Boolean && tree->child[0]->type != Undefined) {
                            errors(tree, 11, tree->child[0]);
                        }
                        // 10: Cannot use array as test condition in %s statement.\n
                        if(tree->child[0]->isArray == true && tree->child[0]->isIndexed == false) {
                            errors(tree, 10, tree->child[0]);
                        }
                    }
					break;
	
				case WhileK:
					isLoop = true;
					loopDepth = depth;
					for(int i = 0; i < MAXCHILDREN; i++) { treeTraverse(tree->child[i], st); }
					tree->attr.name = strdup("while");
					if(tree->child[0] != NULL) {
						// 11: Expecting Boolean test condition in %s statement but got type %s.\n
						if(tree->child[0]->type != Boolean && tree->child[0]->type != Undefined) {
							errors(tree, 11, tree->child[0]);
						}
						// 10: Cannot use array as test condition in %s statement.\n
						if(tree->child[0]->isArray == true && tree->child[0]->isIndexed == false) {
							errors(tree, 10, tree->child[0]);
						}
					}
					if(depth<loopDepth) { isLoop = false; loopDepth = 1; }
					break;

				case ForeachK:
					isLoop = true;
					loopDepth = depth;
					for(int i = 0; i < MAXCHILDREN; i++) { treeTraverse(tree->child[i], st); }
					if(tree->child[1]->isArray == true && tree->child[1]->isIndexed == false) {
						// 28: Foreach requires operands of 'in' be the same type but lhs is type %s and rhs array is type %s.\n"
						if(tree->child[0]->type != tree->child[1]->type && (tree->child[0]->type != Undefined && tree->child[1]->type != Undefined)) {
							errors(tree, 28, ptr);	
						}
					} else { // rhs is not an array
						// 29: If not an array, foreach requires lhs of 'in' be of type int but it is type %s.\n"
						if(tree->child[0]->type != Integer && tree->child[0]->type != Undefined) {
							errors(tree, 29, ptr);
						}
						// 30: If not an array, foreach requires rhs of 'in' be of type int but it is type %s.\n"
						if(tree->child[1]->type != Integer && tree->child[1]->type != Undefined) {
							errors(tree, 30, ptr);
						}
					} 
					// 31: In foreach statement the variable to the left of 'in' must not be an array.\n"
					if(tree->child[0]->isArray == true) {
						errors(tree, 31, ptr);
					}
					if(depth<loopDepth) { isLoop = false; loopDepth = 1; }
                  	break;	
		
				case CompK:
					if(!isComp) { // NOT a function's comp 
						st.enter("comp");
						newScope = true;
						localOffset = locOff;
					} else { isFunc = true; } // IS a function's comp
					isComp = false;
					
					tree->size = locOff;
					child = tree->child[0];
					while(child != NULL) {
						if(!child->isStatic) {
							tree->size -= 1 + child->arrayLen;
						}
						child = child->sibling;
					}

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

						// expecting no return val but got x: 13
						if(func->type == Void && child->type != Void) 
							errors(tree, 13, func);

						// no arrays returned: 12
						if(child->isArray == true && child->isIndexed == false)
							errors(tree, 12, func);

						// expecting return type x but got y: 14
						if(func->type != child->type && func->type != Void && child->type != Undefined)
							errors(tree, 14, func);	
					}
					break;
			
				case BreakK:
					if(isLoop == false) 
						errors(tree, 17, ptr); 
					break;
			}
		}

		/*** EXPRESSION KIND ***/
		if(tree->nodekind == ExpK) {
			switch(tree->kind.exp) {
				case OpK:
				case AssignK:
					isSet = true;
					tree = getOpTypes(expectLHS, expectRHS, tree);
					isSet = false;
					for(int i = 0; i < MAXCHILDREN; i++) { treeTraverse(tree->child[i], st); }
					verifyOpTypes(tree, st);
					break;

				case ConstK:
					// if the node is a string, we set its type to char* array
					if(tree->type == String) {
						tree->isArray = true;
						tree->type = Character;
					}
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
		
						// 18: '%s' is a simple variable and cannot be called.\n"
						if(ptr->nodekind == DeclK && ptr->kind.decl != FunK) {
							errors(tree, 18, ptr);
						}
	
						// check for the parameters to be the same
						if(ptr->child[0] != NULL) {
							funcParams = ptr->child[0];
						} else {
							funcParams = NULL;
						}
						if(tree->child[0] != NULL) {
							callParams = tree->child[0];
						} else {
							callParams = NULL;
						}
						checkParams(funcParams, callParams, ptr, tree);
					}
					break;

				case IdK:
	                ptr = (TreeNode *)st.lookup(tree->attr.name);
                    if(ptr == NULL) { // original decl
                        errors(tree, 2, ptr);
						tree->type = Undefined;
						for(int i = 0; i < MAXCHILDREN; i++) { treeTraverse(tree->child[i], st); }
					} else {
						tree->type = ptr->type;
						tree->isArray = ptr->isArray;
						for(int i = 0; i < MAXCHILDREN; i++) { treeTraverse(tree->child[i], st); }
						ptr = (TreeNode *)st.lookup(tree->attr.name);
				
						// 19: Cannot use function '%s' as a simple variable.\n"
						if(ptr->nodekind == DeclK && ptr->kind.decl == FunK) {
							errors(tree, 19, ptr);
						} else if(tree->isArray == false) {
							// 22: Cannot index nonarray '%s'.\n  
							if(tree->child[0] != NULL) {
								errors(tree, 22, ptr);
							}
						}

						// array checks
						if(tree->isArray == true) { // Id is an array 
							if(tree->child[0] != NULL) { // that is being indexed
								if(tree->child[0]->attr.name != NULL) { // child is an Id
									if(tree->child[0]->isArray == true) { // child is an array
										if(tree->child[0]->isIndexed == false) { // child is an unindexed array
											// 20: Array index is the unindexed array '%s'.\n (aa[aa])
											if(tree->child[0]->type != Integer && tree->child[0]->type != Undefined) { // type int?
												errors(tree, 21, tree->child[0]);
											}
											errors(tree, 20, tree->child[0]);
										} else { // child is an indexed array
							                // 21: Array '%s' should be indexed by type int but got %s.\n
											if(tree->child[0]->type != Integer && tree->child[0]->type != Undefined) { // child is indexed array w/o type int
												errors(tree, 21, tree->child[0]);
											} else { // child is an indexed array with type int - index me
												tree->isIndexed = true;
											}
										}
									} else { // child is not an array
										// 21: Array '%s' should be indexed by type int but got %s.\n 
										if(tree->child[0]->type != Integer && tree->child[0]->type != Undefined) { // child is an id w/o type int
											errors(tree, 21, tree->child[0]);
										} 
										tree->isIndexed = true; // index me anyways
									}
								} else if(tree->child[0]->attr.name == NULL) { // child is a const
									// 21: Array '%s' should be indexed by type int but got %s.\n 
									if(tree->child[0]->type != Integer && tree->child[0]->type != Undefined) { // child is a const w/o type int
										errors(tree, 21, tree->child[0]);
									} 
									tree->isIndexed = true;
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
					tree->size = 1;
					tree->offset = locOff;
					locOff -= tree->size;
					localOffset = locOff;

				case VarK: 
					for(int i = 0; i < MAXCHILDREN; i++) { treeTraverse(tree->child[i], st); }
					if((st.insert(tree->attr.name, tree)) == false) {
						ptr = (TreeNode *)st.lookup(tree->attr.name);
						errors(tree, 1, ptr);
					}

					// Am i global or local?
					tmp = (TreeNode *)st.lookupGlobal(tree->attr.name);
					if(tmp != NULL && depth == 1) { tree->isGlobal = true; } 
					else { tree->isGlobal = false; }

					// semantic checks
					if(tree->child[0] != NULL) { // decl is being init w/ expression
						if(tree->isArray == true) { // decl is of type array
							if(tree->type != tree->child[0]->type) {
								// 33: Initializer for array variable '%s' must be a string, but is of nonarray type %s.\n
								if(tree->type == Character && (tree->child[0]->isArray == false && tree->child[0]->type != Character)) { // char array is a string 
									errors(tree, 33, tree->child[0]); 
								}
								// 32: Array '%s' must be of type char to be initialized, but is of type %s.\n
								else if(tree->type != Character && (tree->child[0]->isArray == true && tree->child[0]->type == Character)) {
									errors(tree, 32, tree->child[0]);	
								}
							}
						}
						else { // init expr is not an array...
							// 35: Initializer for variable '%s' is not a constant expression.\n
							if(tree->child[0]->type == Undefined) { // <-- wert
								errors(tree, 35, tree->child[0]);
							}
							// 36: Variable '%s' is of type %s but is being initialized with an expression of type %s.\n
							if(tree->type != tree->child[0]->type && (tree->type != Undefined && tree->child[0]->type != Undefined)) {
								errors(tree, 36, tree->child[0]); 
							}
							// 34: Initializer for nonarray variable '%s' of type %s cannot be initialized with an array.\n
							if(tree->child[0]->isArray == true) {
								errors(tree, 34, tree->child[0]);
							}
						}
					}		

					// To calculate size and location in memory for var decl's
					if(tree->kind.decl == VarK) {
						tree->size = 1;

						if(tree->isArray == true) { tree->size += tree->arrayLen; }
						//printf("VarK %s: size %d\n", tree->attr.name, tree->size);

						if(tree->isGlobal) { // are we global?
							if(tree->isArray == false) { tree->offset = globalOffset; } 
							else { tree->offset = globalOffset-1; } // arrays start down 1 further
							globalOffset -= tree->size;
						} else { // or are we local?
							if(tree->isStatic == true) { // static goes to global
								if(tree->isArray == false) { tree->offset = globalOffset; }
								else { tree->offset = (globalOffset-1); } // arrays start down 1 further
								globalOffset -= tree->size;
							} else { // non-static goes to local
								if(tree->isArray == false) { tree->offset = locOff; }
								else { tree->offset = (locOff-1); } // arrays start down 1 further
								locOff -= tree->size;
							}
						}
						localOffset = locOff;
					}
					break;

				case FunK:
                    if((st.insert(tree->attr.name, tree)) == false) {
                        ptr = (TreeNode *)st.lookup(tree->attr.name);
                        errors(tree, 1, ptr);
                    }

					tree->isGlobal = true;
					paramCount = 0;

					if(tree->child[0] != NULL) {
						tmp = tree->child[0];
						while(tmp != NULL) {
							paramCount++;
							tmp = tmp->sibling;
						}
					}
					tree->size = 0 - (2 + paramCount); // func offset + num of params = func size

					funcName = tree->attr.name; // for checking return value
					st.enter(tree->attr.name);

					locOff = 0;  // new scope, new localOffset count
					tree->offset = locOff; // we start at 0 in local space
					locOff -= 2; // -2 for the first two spaces the func takes up of local space
					localOffset = locOff;

					isComp = true;
					newScope = true;

					for(int i = 0; i < MAXCHILDREN; i++) { treeTraverse(tree->child[i], st); }

					// 16: Expecting to return type %s but function '%s' has no return statement.\n"
					if(isReturn == false && tree->lineNum != -1 && tree->type != Void) {
						isWarning = true;
						errors(tree, 16, ptr);
					} else {
						isReturn = false;
					}	
					break;
			}
		}
		
		// No matter what, if we have entered a new scope, by the time we get here, we
		// want to be leaving this scope since all if the children have been processed
		if(newScope) {
			st.leave();
			newScope = false;
		}
		tree = tree->sibling;
	}
	//printSymTab(st);
}

// checks the parameters of a call vs a function's actual declaration
void checkParams(TreeNode *funcP, TreeNode *callP, TreeNode *func, TreeNode *call) {
	int paramCount = 0;
	while(funcP != NULL && callP != NULL) {
		paramCount++;
		// 23: Expecting type %s in parameter %i of call to '%s' defined on line %d but got %s.\n
	    if(funcP->type != callP->type && (funcP->type != Undefined && callP->type != Undefined)) {
			ptr = func;
	        paramNum = paramCount;
	        errors(callP, 23, funcP);
		}
	    // 24: Expecting array in parameter %i of call to '%s' defined on line %d.\n
	    if((funcP->isArray == true && funcP->isIndexed == false) && callP->isArray == false) {
	        paramNum = paramCount;
	        errors(call, 24, func);
	    // 25: Not expecting array in parameter %i of call to '%s' defined on line %d.\n
	    } else if(funcP->isArray == false && (callP->isArray == true && callP->isIndexed == false)) {
	        paramNum = paramCount;
	        errors(call, 25, func);
	    }
		funcP = funcP->sibling;
		callP = callP->sibling;
	}
	// 27: Too many parameters passed for function '%s' defined on line %d.\n
	if(funcP == NULL && callP != NULL) {
		errors(call, 27, func);
	// 26: Too few parameters passed for function '%s' defined on line %d.\n
	} else if(funcP != NULL && callP == NULL) {
		errors(call, 26, func);
	}
}

// Gets the expected types of op children; Sets the type of operator to expected type 
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

		// Let's deal with just the =, !=, and == (should work with arrays)
		if(expectLHS == Undefined && expectRHS == Undefined) {
			if(strcmp(tree->attr.name, "=") == 0) {
				tree->type = lhs; // since OpK is an assignment, its type will resolve to lhs
			}
			if(lhs != rhs) {
				if(lhs != Undefined && rhs != Undefined) { 
					errors(tree, 3, ptr);
				}
			}
		
			// lhs is array and rhs is not or vice versa: 6
			if(lptr->isArray == true && rptr->isArray == false) {
				if(lptr->isIndexed == false) {
					errors(tree, 6, ptr); 
				}
			}
			else if(lptr->isArray == false && rptr->isArray == true ) {
				if(rptr->isIndexed == false) {
					errors(tree, 6, ptr); 
				}
			}
		// Now to handle >, <, >=, and <= (nothing else binary should work with arrays)**
		} else { 
			// requires operands of the same type
			if(expectLHS == CharOrInt) {
				// 3: '%s' requires operands of the same type but lhs is type %s and rhs is %s.\n
                if(lhs != rhs && (lhs != Undefined && rhs != Undefined)) {
                    errors(tree, 3, ptr);
                } 
				// 4: '%s' requires operands of type %s but lhs is of type %s.\n
				if((lhs != Character && lhs != Integer) && (lhs != Undefined)) {
					tmpType = tree->type;
					tree->type = CharOrInt;
					errors(tree, 4, ptr);
					tree->type = tmpType;
				}
				// 5: '%s' requires operands of type %s but rhs is of type %s.\n
				if((rhs != Character && rhs != Integer) && (rhs != Undefined)) {
					tmpType = tree->type;
					tree->type = CharOrInt;
					errors(tree, 5, ptr);
					tree->type = tmpType;
				}
			}
			else { // to handle everything else (+[and the like], %, and +=[and the like])
				if(lhs != expectLHS && lhs != Undefined) { 
					errors(tree, 4, ptr);
				}
				if(rhs != expectRHS && rhs != Undefined) {
					errors(tree, 5, ptr);
				}
			}
			// 7: The operation '%s' does not work with arrays.\n
			if((lptr->isArray == true && lptr->isIndexed == false) || (rptr->isArray == true && rptr->isIndexed == false)) {
				errors(tree, 7, ptr);
			}
		}
	}

	// UNARY EXPRESSIONS
	if(tree->isUnary == true) {
		TreeNode *lptr = tree->child[0];
		ExpType lhs = lptr->type;
		if(strcmp(tree->attr.name, "*") == 0) { // is * (array check)
			// 8: The operation '%s' only works with arrays.\n"
			if((lptr->isArray == false || lptr->isIndexed == true) && lptr->type != Undefined) {
				errors(tree, 8, ptr);
			} 
		} else { // remaining unaries
            // expects %s but is getting %s: 9
            if(expectLHS != lhs) {
                errors(tree, 9, ptr);
            }
			// 7: The operation '%s' does not work with arrays.\n (is an array, but not indexed)
			if(lptr->isArray == true && lptr->isIndexed == false) {
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

	switch(errorCode) {
		// DECLARATIONS
		case 1: printf("ERROR(%d): Symbol '%s' is already defined at line %d.\n", tree->lineNum, tree->attr.name, p->lineNum); break;
		case 2: printf("ERROR(%d): Symbol '%s' is not defined.\n", tree->lineNum, tree->attr.name); break;

		// EXPRESSIONS
		case 3: printf("ERROR(%d): '%s' requires operands of the same type but lhs is type %s and rhs is %s.\n", tree->lineNum, tree->attr.name, getType(tree->child[0]), getType(tree->child[1])); break;
		case 4: printf("ERROR(%d): '%s' requires operands of type %s but lhs is of type %s.\n", tree->lineNum, tree->attr.name, getType(tree), getType(tree->child[0])); break;
		case 5: printf("ERROR(%d): '%s' requires operands of type %s but rhs is of type %s.\n", tree->lineNum, tree->attr.name, getType(tree), getType(tree->child[1])); break;
		case 6: printf("ERROR(%d): '%s' requires that if one operand is an array so must the other operand.\n", tree->lineNum, tree->attr.name); break;
		case 7: printf("ERROR(%d): The operation '%s' does not work with arrays.\n", tree->lineNum, tree->attr.name); break;
		case 8: printf("ERROR(%d): The operation '%s' only works with arrays.\n", tree->lineNum, tree->attr.name); break;
		case 9: printf("ERROR(%d): Unary '%s' requires an operand of type %s but was given %s.\n", tree->lineNum, tree->attr.name, getType(tree), getType(tree->child[0])); break;
	
		// TEST CONDITIONS
		case 10: printf("ERROR(%d): Cannot use array as test condition in %s statement.\n", tree->lineNum, tree->attr.name); break;
		case 11: printf("ERROR(%d): Expecting Boolean test condition in %s statement but got type %s.\n", tree->lineNum, tree->attr.name, getType(p)); break;
	
		// RETURN
		case 12: printf("ERROR(%d): Cannot return an array.\n", tree->lineNum); break;
		case 13: printf("ERROR(%d): Function '%s' at line %d is expecting no return value, but return has return value.\n", tree->lineNum, p->attr.name, p->lineNum); break;
		case 14: printf("ERROR(%d): Function '%s' at line %d is expecting to return type %s but got %s.\n", tree->lineNum, p->attr.name, p->lineNum, getType(p), getType(tree->child[0])); break;
		case 15: printf("ERROR(%d): Function '%s' at line %d is expecting to return type %s but return has no return value.\n", tree->lineNum, p->attr.name, p->lineNum, getType(p)); break;
		case 16: printf("WARNING(%d): Expecting to return type %s but function '%s' has no return statement.\n", tree->lineNum, getType(tree), tree->attr.name); break;
	
		// BREAK
		case 17: printf("ERROR(%d): Cannot have a break statement outside of loop.\n", tree->lineNum); break;
		
		// FUNCTION INVOCATION
		case 18: printf("ERROR(%d): '%s' is a simple variable and cannot be called.\n", tree->lineNum, tree->attr.name); break;
		case 19: printf("ERROR(%d): Cannot use function '%s' as a simple variable.\n", tree->lineNum, tree->attr.name); break;
	
		// ARRAY INDEXING
	    case 20: printf("ERROR(%d): Array index is the unindexed array '%s'.\n", tree->lineNum, p->attr.name); break;
	    case 21: printf("ERROR(%d): Array '%s' should be indexed by type int but got %s.\n", tree->lineNum, tree->attr.name, getType(p)); break;
	    case 22: printf("ERROR(%d): Cannot index nonarray '%s'.\n", tree->lineNum, tree->attr.name); break;
	
		// PARAMETER LIST
		case 23: printf("ERROR(%d): Expecting type %s in parameter %i of call to '%s' defined on line %d but got %s.\n", tree->lineNum, getType(p), paramNum, ptr->attr.name, ptr->lineNum, getType(tree)); break;
		case 24: printf("ERROR(%d): Expecting array in parameter %i of call to '%s' defined on line %d.\n", tree->lineNum, paramNum, p->attr.name, p->lineNum); break;
		case 25: printf("ERROR(%d): Not expecting array in parameter %i of call to '%s' defined on line %d.\n", tree->lineNum, paramNum, p->attr.name, p->lineNum); break;
		case 26: printf("ERROR(%d): Too few parameters passed for function '%s' defined on line %d.\n", tree->lineNum, tree->attr.name, p->lineNum); break;
		case 27: printf("ERROR(%d): Too many parameters passed for function '%s' defined on line %d.\n", tree->lineNum, tree->attr.name, p->lineNum); break;

		// FOREACH
		case 28: printf("ERROR(%d): Foreach requires operands of 'in' be the same type but lhs is type %s and rhs array is type %s.\n", tree->lineNum, getType(tree->child[0]), getType(tree->child[1])); break;
		case 29: printf("ERROR(%d): If not an array, foreach requires lhs of 'in' be of type int but it is type %s.\n", tree->lineNum, getType(tree->child[0])); break;
		case 30: printf("ERROR(%d): If not an array, foreach requires rhs of 'in' be of type int but it is type %s.\n", tree->lineNum, getType(tree->child[1])); break;
		case 31: printf("ERROR(%d): In foreach statement the variable to the left of 'in' must not be an array.\n", tree->lineNum); break;
	
		// INITIALIZERS
		case 32: printf("ERROR(%d): Array '%s' must be of type char to be initialized, but is of type %s.\n", tree->lineNum, tree->attr.name, getType(tree)); break;
		case 33: printf("ERROR(%d): Initializer for array variable '%s' must be a string, but is of nonarray type %s.\n", tree->lineNum, tree->attr.name, getType(p)); break;
		case 34: printf("ERROR(%d): Initializer for nonarray variable '%s' of type %s cannot be initialized with an array.\n", tree->lineNum, tree->attr.name, getType(tree)); break;
		case 35: printf("ERROR(%d): Initializer for variable '%s' is not a constant expression.\n", tree->lineNum, tree->attr.name); break;
		case 36: printf("ERROR(%d): Variable '%s' is of type %s but is being initialized with an expression of type %s.\n", tree->lineNum, tree->attr.name, getType(tree), getType(p)); break;
		
		// MISC
		case 37: printf("ERROR(LINKER): Procedure main is not defined.\n"); break;
	}
}

#ifndef _SEMANTIC_H_
#define _SEMANTIC_H_

#include "symTab.h"

string stuff();


void treeTraverse(TreeNode*, SymbolTable);
void printSymTab(SymbolTable);
int scopeAndType(TreeNode*&);
void errors(TreeNode*, int, TreeNode*);
void addIORoutines(TreeNode*&);

// Typing
void checkParams(TreeNode*, TreeNode*, TreeNode*, TreeNode*);
void verifyOpTypes(TreeNode*, SymbolTable);
TreeNode* getOpTypes(ExpType&, ExpType&, TreeNode*);

#endif

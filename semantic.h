#ifndef _SEMANTIC_H_
#define _SEMANTIC_H_

#include "symTab.h"

void treeTraverse(TreeNode*, SymbolTable);
void printSymTab(SymbolTable);
void scopeAndType(TreeNode*&);
void errors(TreeNode*, int, TreeNode*);
void addIORoutines(TreeNode*&);

// Typing
void verifyOpTypes(TreeNode*, SymbolTable);

TreeNode* getOpTypes(ExpType&, ExpType&, TreeNode*);

TreeNode* setOpRetType(TreeNode*);
TreeNode* setOpTakeType(TreeNode*);

#endif

#ifndef _SEMANTIC_H_
#define _SEMANTIC_H_

#include "symTab.h"

void treeTraverse(TreeNode*, SymbolTable);
void printSymTab(SymbolTable);
void scopeAndType(TreeNode*&);
void errors(TreeNode*, int, TreeNode*);
void verifyOpTypes(TreeNode*, SymbolTable, TreeNode*, TreeNode*);
TreeNode* setTypeString(TreeNode*);
void addIORoutines(TreeNode*&);
TreeNode* setOpTypes(TreeNode*);

#endif

TreeNode *newStmtNode(StmtKind, int);
TreeNode *newExpNode(ExpKind, int);
TreeNode *newDeclNode(DeclKind, int);
void printTree(FILE*, TreeNode*, int, int, bool);

TreeNode *newStmtNode(StmtKind, int);
TreeNode *newExpNode(ExpKind, int);
TreeNode *newDeclNode(DeclKind, int);
char* getType(TreeNode*);
void printTree(FILE*, TreeNode*, int, int, bool);

#ifndef _GLOBALS_H_
#define _GLOBALS_H_

#include<stdio.h>
#include<stdlib.h>

typedef enum {StmtK, ExpK, DeclK} NodeKind;
typedef enum {FunK, VarK, ParamK} DeclKind;
typedef enum {IfK, WhileK, ForeachK, CompK, ReturnK, BreakK} StmtKind;
typedef enum {OpK, ConstK, IdK, CallK, AssignK} ExpKind;
typedef enum {EqK, LesseqK, LessK, GrtK, GrteqK, NoteqK, PlusK, MinusK, StarK, FwdslashK, ModK, QuesK} OpKind;
typedef enum {Void, Integer, Boolean, Character, Error, Undefined} ExpType;
typedef enum {numconst, charconst, stringconst, boolconst} ConstType;

#define MAXCHILDREN 3
typedef struct treeNode {
    struct treeNode *child[MAXCHILDREN];	// children of the node
    struct treeNode *sibling;				// siblings for the node
	struct treeNode *param;   
 
	// what kind of node
	int lineNum;				// linenume relevant to this node
	NodeKind nodekind;			// type of node
	union {						// subtype of type
		DeclKind decl;			// used when DeclK
		StmtKind stmt;			// used when StmtK
		ExpKind exp;			// used when ExpK
	} kind;

	// extra props about the node
	union {						// relevant data to type -> attr
		OpKind op;				// type of token 
		int value;				// used when an int const or bool
		unsigned char cvalue;	// used when char
		char *string;			// used when string const
		char *name;				// used when IdK
	} attr;
	ExpType type;				// used when ExpK for type checking
	ConstType ctype;
	bool isStatic;				// is statically allocated?
	bool isArray;				// is this an array
	int arrayLen;				// length of array
} TreeNode;

#endif

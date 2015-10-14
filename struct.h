#ifndef _STRUCT_H_
#define _STRUCT_H_

typedef struct Token{
	char *input, *sconst, *id;
	char cconst;
	int nconst, bconst;
	int stringLen;
} Token;

#endif

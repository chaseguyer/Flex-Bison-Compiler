typedef struct Token{
	char *input, *sconst, *id;
	char cconst;
	int nconst, bconst;
	int stringLen;
} Token;

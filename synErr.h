#ifndef _SYNERR_H_
#define _SYNERR_H_

int split(char*, char*, char);
void trim(char*);
void initTokenMaps();
char* niceTokenStr(char*);
bool elaborate(char*);
void tinySort(char**, int, int, bool);
void yyerror(const char*);

#endif

#ifndef _SYMTAB_H_
#define _SYMTAB_H_

#include<map>
#include<vector>
#include<stdio.h>
#include<stdlib.h>
#include<iostream>

using namespace std;

class Scope {
private:
	string name;
	map<string, void *> symbols;

public:
	Scope(string newname);
	~Scope();
	void print(void (*printData)(void *));
	void applyToAll(void (*action)(string, void *));
	bool insert(string sym, void *ptr);
	
	void *lookup(string sym);

	string scopeName() { return name; };
};

class SymbolTable {
private:
	vector<Scope *> stack;
	
public:
	SymbolTable();
	int depth();
	void print(void (*printData)(void *));
	void applyToAllGlobal(void (*action)(string, void *));
	void enter(string name);
	void leave();
	void *lookup(string sym);

	void *lookupGlobal(string sym);
	void *lookupDepth(string sym, int depth); //looks up string at depth x
												// returns null if it cant find it

	bool insert(string sym, void *ptr);

	bool insertGlobal(string sym, void *ptr);
};

#endif

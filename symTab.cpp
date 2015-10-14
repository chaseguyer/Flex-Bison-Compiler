#include "symTab.h"
#include "tree.h"

using namespace std;

// Class Scope
Scope::Scope(string newname) {
	name = newname;
}

Scope::~Scope() {
}

void Scope::print(void (*printData)(void *)) {
	printf("Scope: %-15s -----------------\n", name.c_str());
	for(map<string, void *>::iterator it = symbols.begin(); it != symbols.end(); it++) {
		printf("%20s: ", (it->first).c_str());
		printData(it->second);
		printf("\n");		
	}
}

void Scope::applyToAll(void (*action)(string, void *)) {
	for(map<string, void *>::iterator it = symbols.begin(); it != symbols.end(); it++) {
		action(it->first, it->second);
	}
}

bool Scope::insert(string sym, void *ptr) {
	if(symbols.find(sym) == symbols.end()) {
		symbols[sym] = ptr;
		return true;
	} else 
		return false;
}

void *Scope::lookup(string sym) {
	if(symbols.find(sym) != symbols.end()) {
		return symbols[sym];
	} else
		return NULL;
}

// Class SymbolTable
SymbolTable::SymbolTable() {
	enter((string)"Global");
}

int SymbolTable::depth() {
	return stack.size();
}

void SymbolTable::print(void (*printData)(void *)) {
    printf("===========  Symbol Table  ===========\n");
	for(vector<Scope *>::iterator it = stack.begin(); it != stack.end(); it++) {
		(*it)->print(printData);
	}
    printf("===========  ============  ===========\n");
}

void SymbolTable::applyToAllGlobal(void (*action)(string, void *)) {
	stack[0]->applyToAll(action);
}

void SymbolTable::enter(string name) {
	stack.push_back(new Scope(name));
}

void SymbolTable::leave() {
	if(stack.size() > 1) {
		delete stack.back();
		stack.pop_back();
	} else 
        printf("ERROR(SymbolTable): You cannot leave global scope.  Number of scopes: %d.\n", (int)stack.size());
}

void * SymbolTable::lookup(string sym) {
	void *data;
	for(vector<Scope *>::reverse_iterator it = stack.rbegin(); it != stack.rend(); it++) {
		data = (*it)->lookup(sym);
		if(data != NULL)
			break;
	}
	return data;
}

void * SymbolTable::lookupGlobal(string sym) {
	void *data;
	data = stack[0]->lookup(sym);
	return data;
}

bool SymbolTable::insert(string sym, void *ptr) {
	return (stack.back())->insert(sym, ptr);
}

bool SymbolTable::insertGlobal(string sym, void *ptr) {
	return stack[0]->insert(sym, ptr);
}	 

#pragma once

#include <map>
#include <vector>
#include <fstream>
#include <sstream>
using namespace std;

#include "Symbol.h"

class SymTab {
public:
	SymTab();
    ~SymTab();
	
    void			put(Symbol *newSymbol);
    Symbol*			get(string key);
    bool			remove(string key);
    Symbol*			getByNumber(int number);

    void			fillTable(vector<Symbol*> *arr);
	
    int				getNumOfSymbols();
    
	static string	toHex(unsigned int num);
	void			dumpTable(ofstream *file);
    void			dumpTable2(ofstream *file);

private:
    int numOfSymbols;

	// entry name is the key
	// Symbol is the value
	map<string, Symbol *> *table;
};

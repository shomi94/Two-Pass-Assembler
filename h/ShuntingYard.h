#pragma once

#include <iostream>
#include <string>
#include <list>
#include <stack>
#include <set>
#include <vector>
#include "Check.h"
#include "Symbol.h"
#include "SymTab.h"
using namespace std;

class ShuntingYard {
public:
	ShuntingYard();
	~ShuntingYard();

	list<string>	itop(string str, vector<Symbol *> *all_symbols, int mode);
	int				resolve(list<string> postfix);
	Symbol*			needsRelocation(string line, SymTab *table, Symbol *currentSection);
	int				getWeight(string op);
	bool			isRightAssoc(string op);
	int				checkPrecedence(string op1, string op2);
	bool			isOperator(string op);
	bool			isOperand(string c);
	
	static vector<string>* separateOps(string str);

	string	getRelocLabel();
	void	setRelocLabel(string label);

private:
	Check *check;

	string relocLabel;
};

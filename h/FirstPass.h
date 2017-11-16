#pragma once

#include "SymTab.h"
#include "Check.h"
#include "Symbol.h"
#include "ShuntingYard.h"

#include <vector>
#include <string>
#include <set>
#include <sstream>
#include <list>
#include <algorithm>
using namespace std;

class FirstPass {
public:
	FirstPass(vector<Symbol *> *symbols);
	~FirstPass();

	SymTab *pass(vector<string> *asmText);

	void parseLabel		(string *line);
	void parseSection	(string *line);
	void parseGlobal	(string *line);
	int  parseDataDef	(string *line);
	void parseORG		(string *line);
	void parseDEF		(string *line);
	void parseDUP		(string *line);

	void parseLoadStore		(string *line);
	void parseFlowControl	(string *line);

	static vector<string>	*read		(const string file);
	static void				removeSpaces(string *str);
	static void				toLowercase	(string *str);
	static unsigned long	fromBinary	(const string &str);
	static unsigned int		fromHex		(const string &str);
	static unsigned int		fromText	(const string &str, vector<Symbol *> *all_symbols);

private:
	SymTab *table;
	int old_lc;
	int lc;

	Check *check;
	ShuntingYard *shuntingYard;

	set<string> *labels;
	set<string> *consts;
	set<string> *sections;
	set<string> *publics;
	set<string> *externs;
	Symbol *section;
	unsigned int org_address;
	bool org_happened;

	vector<Symbol *> *all_sections;
	vector<Symbol *> *all_symbols;
};

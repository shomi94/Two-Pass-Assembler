#pragma once

#include "RelocTab.h"
#include "Check.h"
#include "SymTab.h"
#include "ShuntingYard.h"
#include "FirstPass.h"
#include <sstream>
#include <iomanip>
#include <map>
#include <bitset>
using namespace std;

extern Symbol **symbolsToRelocate;

class SecondPass {
public:
	SecondPass(vector<Symbol *> *symbols);
	~SecondPass();

	void pass(SymTab *tab, vector<string> *asmText, ofstream *file);

	void parseSection		(string *line);
	void parseDUP			(string *line);
	void parseDataDef		(string *line);
	void parseORG			(string *line);
	void parseDB			(string *line);
	void parseDW			(string *line);
	void parseDD			(string *line);
	void parseArithLog		(string *line);
	void parseStack			(string *line);
	void parseLoadStore		(string *line);
	void parseFlowControl	(string *line);

	void toLittleEndian	(string *line);
	void separateCode	(string *line);

	void fillCodes	();
	int	 chooseType	(string *suffix);
	int	 chooseValue(Symbol *symbol);
	char chooseRelType(Symbol *symbol);

private:
	SymTab *table;
	RelocTab *reloc;
	int lc;
	int old_lc;
	stringstream *stream;

	Check *check;
	ShuntingYard *shuntingYard;
	Symbol *section;
	
	unsigned int org_address;
	bool org_happened;

	vector<Symbol *> *all_symbols;
	vector<string> *code;
	vector<RelocTab *> *all_relocations;

	map<string, int> *codes;
};
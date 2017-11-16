#pragma once

#include <vector>
#include <fstream>
#include "Symbol.h"
#include "SymTab.h"
#include "Relocation.h"
using namespace std;

class RelocTab {
public:
    RelocTab(Symbol *sec);
    ~RelocTab();

    void    put(Relocation *rel);
    void    dumpTable(std::ofstream *file);

	void    setSection(Symbol *sec);
    Symbol*	getSection();

    bool    isEmpty();

private:
	vector<Relocation *> *table;
    Symbol *section;
};

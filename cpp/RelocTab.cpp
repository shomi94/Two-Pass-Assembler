#include "RelocTab.h"

// constructor
RelocTab::RelocTab(Symbol *sec) {
    section = sec;
    table = new vector<Relocation *>;
}

// destructor
RelocTab::~RelocTab() {}

// put relocation in table
void RelocTab::put(Relocation *rel) {
    table->push_back(rel);
}

// set current section
void RelocTab::setSection(Symbol *sec) {
    section = sec;
}

// dump table
void RelocTab::dumpTable(ofstream *file) {
    while(table->size() != 0) {
		Relocation *rel = table->front();
		(*file) << SymTab::toHex(rel->getAddress()) << " " << rel->getType() << " " << rel->getNum() << "\n";
		table->erase(table->begin());
    }
}

// return current section
Symbol* RelocTab::getSection() {
    return section;
}

// check whether table is empty or not
bool RelocTab::isEmpty() {
	return table->size() == 0 ? true : false;
}
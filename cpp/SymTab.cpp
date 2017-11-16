#include "SymTab.h"

SymTab::SymTab() {
	table = new map<string, Symbol *>();
	numOfSymbols = 0;
}

SymTab::~SymTab() {}

void SymTab::put(Symbol *newSymbol) {
    numOfSymbols++;
	std::string key = newSymbol->getName();
    (*table)[key] = newSymbol;
}

Symbol *SymTab::get(string key) {
    return (*table)[key];
}

bool SymTab::remove(string key) {
	numOfSymbols--;

	std::map<std::string, Symbol *>::iterator i;
    bool result = false;

    table->erase(key);
    i = table->find(key);
    if(i == table->end())
        result = true;
    return result;
}

// get AbsSym by num
Symbol  *SymTab::getByNumber(int number) {
	Symbol *sym = NULL;
	map<std::string, Symbol *>::iterator i;
    for(i = table->begin(); i != table->end(); ++i)	// TODO: maybe it's i++
        if(i->second->getNum() == number)
            sym = i->second;
    return sym;
}

void SymTab::fillTable(std::vector<Symbol*> *arr) {
    for (vector<Symbol*>::iterator i = arr->begin(); i != arr->end(); ++i)
		put(*i);
}

int SymTab::getNumOfSymbols() {
    return numOfSymbols;
}

string SymTab::toHex(unsigned int num) {
	std::stringstream ss;
	ss << "0x" << std::hex << num;
	std::string result = ss.str();
	return result;
}

void SymTab::dumpTable(std::ofstream *file) {
	(*file) << "#TabelaSimbola\n";
	if (numOfSymbols > 0) {
		for (int i = 0; i < getNumOfSymbols(); i++) {
			Symbol *sym = getByNumber(i);

			(*file) << sym->getType() << " ";
			(*file) << sym->getNum() << " ";
			(*file) << sym->getName() << " ";
			if (sym->getType() == "SEG")
				(*file) << sym->getNumOfSection_sec() << " ";
			else {
				if (sym->getNumOfSection_sym() == -1)
					(*file) << -1 << " ";
				else
					(*file) << sym->getNumOfSection_sym() << " ";
			}
			if (sym->getType() == "SEG") {
				(*file) << SymTab::toHex(sym->getAddress()) << " ";
				(*file) << SymTab::toHex(sym->getSize()) << " ";
			}
			else {
				(*file) << SymTab::toHex(sym->getValue()) << " ";
			}
			(*file) << sym->getFlags();
			(*file) << "\n";
		}
		(*file) << "\n";
	}
}

/*void SymTab::dumpTable2(std::ofstream *file) {
	(*file) << "#TabelaSimbola\n";
    if(numOfSymbols > 0) {
		(*file) << "Tip\t\t";
        (*file) << "R.Br.\t";
        (*file) << "Naziv\t\t\t";
        (*file) << "Sekcija\t\t";
		(*file) << "Pocetna adresa\t";
		(*file) << "Velicina\t";
        (*file) << "Vrednost\t";
		(*file) << "Flegovi\n";
        for(int i = 0; i < getNumOfSymbols(); i++) {
                Symbol *sym = getByNumber(i);

				(*file) << sym->getType() << "\t\t";
				(*file) << sym->getNum() << "\t\t";
				(*file) << sym->getName() << "\t";
				sym->getName().size() <= 14 ? ((*file) << "\t") : ((*file) << "");
				sym->getName().size() <= 7 ? ((*file) << "\t") : ((*file) << "");
				sym->getName().size() < 4 ? ((*file) << "\t") : ((*file) << "");
				if (sym->getType() == "SEG")
					(*file) << sym->getNumOfSection_sec() << "\t\t\t";
				else {
					if(sym->getNumOfSection_sym() == -1)
						(*file) << -1 << "\t\t\t";
					else
						(*file) << sym->getNumOfSection_sym() << "\t\t\t";
				}
				//(*file) << (sym->getType() == "SEG" ? sym->getNumOfSection_sec() : (int)sym->getNumOfSection_sym()) << "\t\t\t";
				if (sym->getType() == "SEG") {
					(*file) << SymTab::toHex(sym->getAddress()) << "\t\t\t";
					SymTab::toHex(sym->getAddress()).size() < 4 ? ((*file) << "\t") : ((*file) << "");
					(*file) << SymTab::toHex(sym->getSize()) << "\t\t";
					SymTab::toHex(sym->getSize()).size() < 4 ? ((*file) << "\t") : ((*file) << "");
					(*file) << "-\t\t\t";
				}
				else {
					(*file) << "-\t\t\t\t";
					(*file) << "-\t\t\t";
					(*file) << SymTab::toHex(sym->getValue()) << "\t\t";
					SymTab::toHex(sym->getValue()).size() < 4 ? ((*file) << "\t") : ((*file) << "");
				}
				(*file) << sym->getFlags() << "\t";
				(*file) << "\n";
        }
    }
}*/
#include "FirstPass.h"

#include <iostream>
#include <fstream>

// constructor
FirstPass::FirstPass(vector<Symbol *> *symbols) {
	table			= new SymTab();
	check			= new Check();
	shuntingYard	= new ShuntingYard();
	labels			= new set<string>;
	consts			= new set<string>;
	sections		= new set<string>;
	publics			= new set<string>;
	externs			= new set<string>;
	section			= NULL;
	all_sections	= new vector<Symbol *>;
	// all_symbols		= new vector<Symbol *>;
	all_symbols		= symbols;
	org_address		= 0;
	org_happened	= false;
	old_lc			= 0;
}

FirstPass::~FirstPass() {}

// FIRST PASS
SymTab* FirstPass::pass(vector<string> *asmText) {
	table->put(new Symbol("UND", 0, ""));

	for (vector<string>::iterator i = asmText->begin(); i != asmText->end(); ++i) {
		string line = *i;
		if (check->isLabel(&line)) {
			parseLabel(&line);
			/*if (check->isGlobal(&line) || check->isSection(&line))
				throw string("Nedozvoljena pozicija labele.");*/
		}
		if (check->isGlobal(&line)) {
			parseGlobal(&line);
		}
		else if (check->isDUP(&line)) {
			parseDUP(&line);
		}
		else if (check->isDataDef(&line)) {
			lc += parseDataDef(&line);
		}
		else if (check->isORG(&line)) {
			parseORG(&line);
		}
		else if (check->isSection(&line)) {
			parseSection(&line);
		}
		else if (check->isDEF(&line)) {
			parseDEF(&line);
		}
		else if (check->isArithLog(&line) || check->isStack(&line)) {
			lc += 4;
		}
		else if (check->isLoadStore(&line)) {
			parseLoadStore(&line);
		}
		else if (check->isFlowControl(&line)) {
			parseFlowControl(&line);
		}
	}

	all_sections->back()->setSize(lc - old_lc);
	for (set<string>::iterator i = externs->begin(); i != externs->end(); i++) {
		// TODO: da vidim koju vrednost treba da imaju globalni simboli
		Symbol *s = new Symbol(*i, 0, 0, "G");
		all_symbols->push_back(s);
	}
	for (vector<Symbol *>::iterator i = all_symbols->begin(); i != all_symbols->end(); i++) {
		set<string>::iterator it = find(publics->begin(), publics->end(), (*i)->getName());
		if (it != publics->end()) {
			(*i)->setFlags("G");
		}
	}
	//for (vector<Symbol *>::iterator i = all_sections->begin(); i != all_sections->end(); i++) {
		// Symbol *s = *i;
		// TODO: provera za ORG direktivu, da li se neke adrese preklapaju
		//		 videcu da l' cu ovo da radim
	//}

	table->fillTable(all_sections);
	table->fillTable(all_symbols);

	return table;
}

// parsing labels
void FirstPass::parseLabel(string *line) {
	int pos = line->find_first_of(":");
	string label = line->substr(0, pos);
	*line = line->substr(pos + 1);
	if (!labels->count(label) && !consts->count(label)) {
		labels->insert(label);
		if (section != NULL) {
			Symbol *sym = NULL;
			if (externs->count(label)) {
				externs->erase(label);
				publics->insert(label);
			}
			if(externs->count(label) || publics->count(label))
				sym = new Symbol(label, (int)section->getNum(), lc, "G");
			else
				sym = new Symbol(label, (int)section->getNum(), lc, "L");
			sym->setValue(lc);
			all_symbols->push_back(sym);
		}
		else throw string("\nLabela je definisana na pogresnom mestu.\n");
	}
	else throw string("\nLabela je vec definisana.\n");
}

// parsing ORG directive
void FirstPass::parseORG(string *line) {
	org_happened = true;
	int i = line->find_first_of("org");
	string address = line->substr(i + 4);
	list<string> l = shuntingYard->itop(address, all_symbols, 1);
	org_address = shuntingYard->resolve(l);
}

// parsing sections
void FirstPass::parseSection(string *line) {
	string temp = *line;
	if (!sections->count(temp)) {
		sections->insert(temp);
		if (section != NULL) {
			all_sections->back()->setSize(lc - old_lc);
		}
		unsigned int addr = org_happened ? org_address : 0;
		org_happened = false;
		org_address = 0;
		string flag = " ";
		if (temp.find("text") != -1)
			flag = "AX";
		else if (temp.find("data") != -1 && temp.find("rodata") == -1)
			flag = "WA";
		else if (temp.find("bss") != -1)
			flag = "WA";
		else if (temp.find("rodata") != -1)
			flag = "A";
		section = new Symbol(temp, addr, flag);
		all_sections->push_back(section);
		lc = addr;
		old_lc = lc;
	}
	else throw string("Sekcija je vec definisana.");
}

void FirstPass::parseGlobal(string *line) {
	if (line->find(" ") != -1) {
		int pos = line->find_first_of(' ');
		string ss = line->substr(pos + 1);
		while(ss.find(',') != -1) {
			FirstPass::removeSpaces(&ss);
			pos = ss.find(',');
			string a = ss.substr(0, pos);
			if (!externs->count(a) && !publics->count(a)) {
				if (labels->count(a) || consts->count(a))
					publics->insert(a);
				else
					externs->insert(a);
			}
			else throw string("\nLabela je vec definisana.\n");
			ss = ss.substr(pos + 1);
		}
		FirstPass::removeSpaces(&ss);
		if (ss != "") {
			if (labels->count(ss))
				publics->insert(ss);
			else
				externs->insert(ss);
		}
	}
}

int FirstPass::parseDataDef(string *line) {
	int pos = line->find_first_of(" ");
	string def = line->substr(0, pos);
	*line = line->substr(pos + 1);
	int result = 0;

	vector<string> *cnt = new vector<string>;
		//int p = line->find_first_of(' ');
		string ss = *line;
		while (ss.find(',') != -1) {
			FirstPass::removeSpaces(&ss);
			int p = ss.find(',');
			string a = ss.substr(0, p);
			cnt->push_back(a);
			ss = ss.substr(p + 1);
		}
		FirstPass::removeSpaces(&ss);
		if (ss != "")
			cnt->push_back(ss);

	if (def == "db") {
		if(cnt->size())
			result = 1 * cnt->size();
		else result = 1;
	}
	if (def == "dw") {
		if (cnt->size())
			result = 2 * cnt->size();
		else
			result = 2;
	}
	if (def == "dd") {
		if (cnt->size())
			result = 4 * cnt->size();
		else
			result = 4;
	}
	return result;
}

void FirstPass::parseDEF(string *line) {
	int pos = line->find_first_of(" ");
	string label = line->substr(0, pos);
	string temp = line->substr(pos + 1);
	if (!labels->count(label) && !consts->count(label)) {
		consts->insert(label);
		if (section != NULL) {
			int pos = temp.find_first_of(" ");
			temp = temp.substr(pos + 1);
			FirstPass::removeSpaces(&temp);
			list<string> l;
			l = shuntingYard->itop(temp, all_symbols, 1);
			int value = shuntingYard->resolve(l);

			Symbol *sym = NULL;
			if (externs->count(label)) {
				externs->erase(label);
				publics->insert(label);
			}
			if (externs->count(label) || publics->count(label))
				sym = new Symbol(label, -1, (unsigned int)value, "G");
			else
				sym = new Symbol(label, -1, (unsigned int)value, "L");

			all_symbols->push_back(sym);
		}
		else throw string("\nKonstanta je definisana na pogresnom mestu.\n");
	}
	else throw string("\nKonstanta je vec definisana.\n");
}

void FirstPass::parseDUP(string *line) {
	int result = parseDataDef(line);
	FirstPass::removeSpaces(line);
	int pos = line->find_first_of("dup");
	*line = line->substr(0, pos - 1);
	list<string> l;
	l = shuntingYard->itop(*line, all_symbols, 1);
	int value = shuntingYard->resolve(l);
	result = result * value;
	lc += result;
}

void FirstPass::parseLoadStore(string *line) {
	if (line->find_first_of('#') != string::npos) {
		lc += 8;
	}
	else {
		int pos = line->find_first_of(",");
		string temp = line->substr(pos + 1);
		FirstPass::removeSpaces(&temp);
		if (check->isRegister(&temp)) {
			lc += 4;
		}
		else if (temp[0] == '[') {
			if (temp.find_first_of('+') != string::npos)
				lc += 8;
			else
				lc += 4;
		}
		else {
			lc += 8;
		}
	}
}

void FirstPass::parseFlowControl(string *line) {
	if (line->substr(0, 3).find("int") == 0 || line->substr(0, 3).find("ret") == 0) {
		lc += 4;
	}
	else if (line->substr(0, 3).find("jmp") == 0 || line->substr(0, 4).find("call") == 0) {
		int pos = line->find_first_of(" ");
		string temp = line->substr(pos + 1);
		FirstPass::removeSpaces(&temp);
		if (check->isRegister(&temp)) {
			lc += 4;
		}
		else if (temp[0] == '[') {
			if (temp.find_first_of('+') != string::npos)
				lc += 8;
			else
				lc += 4;
		}
		else if (!check->isRegister(&temp)) {
			lc += 8;
		}
	}
	else if (line->substr(0, 2).find("jz") == 0 || line->substr(0, 3).find("jnz") == 0 || line->substr(0, 3).find("jgz") == 0 || line->substr(0, 4).find("jgez") == 0 || line->substr(0, 3).find("jlz") == 0 || line->substr(0, 4).find("jlez") == 0) {
		int pos = line->find_first_of(",");
		string temp = line->substr(pos + 1);
		FirstPass::removeSpaces(&temp);
		if (check->isRegister(&temp)) {
			lc += 4;
		}
		else if (temp[0] == '[') {
			if (temp.find_first_of('+') != string::npos)
				lc += 8;
			else
				lc += 4;
		}
		else if (!check->isRegister(&temp)) {
			lc += 8;
		}
	}
}

// reading, parsing and placing the file in a vector of strings
// (each string represent a line)
vector<string> *FirstPass::read(const string file) {
	vector<string> *result = new vector<string>;
	ifstream inputFile(file.c_str());
	string line;

	if (inputFile.is_open()) {
		while (getline(inputFile, line)) {
			// remove tabs
			while (line.find("\t") != -1)
				line.replace(line.find("\t"), 1, " ");

			// remove spaces from the beginning
			removeSpaces(&line);
			
			if (line == "")
				continue;

			// if line ends with a label
			if (line.find(":", line.size() - 1) != string::npos) {
				string temp;
				// then read a new line and merge them
				getline(inputFile, temp);
				line = line + " " + temp;
			}

			// if a comment is found
			if (line.find_first_of(";") != string::npos) {
				line = line.substr(0, (line.find_first_of(";") == 0) ? 0 : line.find_first_of(";"));
			}

			// end parsing if .end found
			if (line.find(".end") != -1) {
				line = line.substr(0, (line.find_first_of(".end") == 0) ? 0 : line.find_first_of(".end"));
				if(line == "")
				break;
			}

			if (line != "") {
				int pos = line.find_last_not_of(" ");
				line = line.substr(0, pos + 1);
			}

			// the assembler is not case sensitive
			FirstPass::toLowercase(&line);
			if(line != "")
				result->push_back(line);
		}
	}
	else
		throw string("ERROR: Unable to open file.");
	inputFile.close();
	return result;
}

void FirstPass::removeSpaces(string *str) {
	while (((*str).substr(0, 1) == " ") || ((*str).substr(0, 1) == "\t"))
		*str = (*str).substr(1);
}

// change all letters to lowercase
void FirstPass::toLowercase(string *str) {
	string::iterator it;
	for (it = str->begin(); it != str->end(); ++it)
		if ((*it) >= 'A' && (*it) <= 'Z')
			(*it) = (*it) - ('A' - 'a');
}

unsigned long FirstPass::fromBinary(const string &str) {
	string s = str.substr(2);
	const char *c = s.c_str();
	return strtol(c, NULL, 2);
}

// convert a hex number to unsigned int
unsigned int FirstPass::fromHex(const string &str) {
	string s = str.substr(2);
	unsigned int x;
	stringstream ss;
	ss << hex << s;
	ss >> x;
	return (unsigned int)x;
}

unsigned int FirstPass::fromText(const string &str, vector<Symbol *> *all_symbols) {
	int pos = str.find(" ");
	string s;
	if (pos != -1)
		s = str.substr(0, pos - 1);
	else
		s = str;
	Symbol *sym = NULL;
	for (vector<Symbol*>::iterator i = all_symbols->begin(); i != all_symbols->end(); ++i)
		if ((*i)->getName() == s) {
			sym = *i;
			break;
		}
	if (sym != NULL)
		return sym->getValue();
	else
		return 0;
}
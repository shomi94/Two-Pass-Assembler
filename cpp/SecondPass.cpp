#include "SecondPass.h"

SecondPass::SecondPass(vector<Symbol *> *symbols) {
	reloc = new RelocTab(new Symbol("UND", 0, ""));
	check = new Check();
	shuntingYard = new ShuntingYard();
	section = NULL;
	lc = 0;
	old_lc = 0;
	org_address = 0;
	org_happened = false;
	all_symbols = symbols;
	code = new vector<string>;
	all_relocations = new vector<RelocTab *>;
	codes = new map<string, int>();
	fillCodes();
}

SecondPass::~SecondPass() { }

void SecondPass::pass(SymTab *tab, vector<string> *asmText, ofstream *file) {
	table = tab;

	for (vector<string>::iterator i = asmText->begin(); i != asmText->end(); ++i) {
		string line = *i;
		if (check->isLabel(&line)) {
			int pos = line.find_first_of(":");
			string label = line.substr(0, pos);
			line = line.substr(pos + 1);
			FirstPass::removeSpaces(&line);
		}
		if (check->isDUP(&line)) {
			parseDUP(&line);
		}
		else if (check->isDataDef(&line)) {
			parseDataDef(&line);
		}
		else if (check->isORG(&line)) {
			parseORG(&line);
		}
		else if (check->isSection(&line)) {
			parseSection(&line);
		}
		else if (check->isArithLog(&line)) {
			lc += 4;
			parseArithLog(&line);
		}
		else if (check->isStack(&line)) {
			lc += 4;
			parseStack(&line);
		}
		else if (check->isLoadStore(&line)) {
			parseLoadStore(&line);
		}
		else if (check->isFlowControl(&line)) {
			parseFlowControl(&line);
		}
	}

	if (section != NULL && section->getName() != ".bss")
		code->push_back(stream->str());
	if (!reloc->isEmpty())
		all_relocations->push_back(reloc);

	for (vector<string>::iterator it = code->begin(); it != code->end(); ++it) {
		int pos = it->find_first_of("\n");
		string name = it->substr(0, pos);
		string temp = it->substr(pos + 1);

		(*file) << "#rel" << name << "\n";
		if (!all_relocations->empty()) {
			if (all_relocations->front()->getSection()->getName() == name) {
				all_relocations->front()->dumpTable(file);
				all_relocations->erase(all_relocations->begin());
			}
		}

		(*file) << name << "\n";
		int counter = 0;
		int j = 0;
		for (unsigned int i = 0; i < temp.size(); i++) {
			j++;
			(*file) << temp[i];
			if (j % 48 == 0 && j != 0 && j != temp.size())
				(*file) << "\n";
		}
		(*file) << "\n";
	}
	(*file) << "#end";
}

void SecondPass::parseSection(string *line) {
	// stavljanje stare tabele relokacije u listu svih tabela
	if (section != NULL && section->getName() != ".bss") {
		code->push_back(stream->str());
		if (!reloc->isEmpty())
			all_relocations->push_back(reloc);
	}

	section = table->get(*line);
	if (line->find(".bss") == -1) {
		stream = new stringstream();
		(*stream) << section->getName() << "\n";
		if (!reloc->isEmpty())
			reloc = new RelocTab(section);
		else
			reloc->setSection(section);
	}

	unsigned int addr = org_happened ? org_address : 0;
	org_happened = false;
	org_address = 0;
	lc = addr;
	old_lc = lc;
}

void SecondPass::parseDUP(string *line) {
	// sluzi samo za povecavanje lc-a
	int pos = line->find_first_of(" ");
	string def = line->substr(0, pos);
	*line = line->substr(pos + 1);
	int result = 0;

	vector<string> *cnt = new vector<string>;
	//int p = line->find_first_of(' ');
	string ss = *line;
	int kind = 0;
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
		if (cnt->size())
			result = 1 * cnt->size();
		else result = 1;
		kind = 1;
	}
	if (def == "dw") {
		if (cnt->size())
			result = 2 * cnt->size();
		else
			result = 2;
		kind = 2;
	}
	if (def == "dd") {
		if (cnt->size())
			result = 4 * cnt->size();
		else
			result = 4;
		kind = 3;
	}

	FirstPass::removeSpaces(line);
	pos = line->find("dup");
	string str = line->substr(0, pos - 1);
	*line = line->substr(pos + 3);
	list<string> l;
	l = shuntingYard->itop(str, all_symbols, 1);
	int duplicate = shuntingYard->resolve(l);
	result = result * duplicate;
	// lc += result;

	FirstPass::removeSpaces(line);
	l = shuntingYard->itop(*line, all_symbols, 1);
	int value = shuntingYard->resolve(l);

	for (int i = 0; i < duplicate; i++) {
		if (shuntingYard->needsRelocation(*line, table, section) != NULL) {
			Symbol *sym = shuntingYard->needsRelocation(*line, table, section);
			Relocation *r = new Relocation(lc, /*chooseRelType(sym)*/'A', chooseValue(sym));
			reloc->put(r);
		}
		switch (kind) {
		case 1:	lc += 1;
				break;
		case 2: lc += 2;
				break;
		case 3:	lc += 4;
				break;
		default:
				break;
		}
	}

	if (kind == 1) {
		for (int i = 0; i < duplicate; i++) {
			(*stream) << setfill('0') << std::setw(2U) << hex << value << " ";
		}
	}
	else if (kind == 2) {
		for (int i = 0; i < duplicate; i++) {
			stringstream sstr;
			sstr << setfill('0') << std::setw(4U) << hex << (short)value;
			string res = sstr.str();
			toLittleEndian(&res);
			(*stream) << res;
		}
	}
	else if (kind == 3) {
		for (int i = 0; i < duplicate; i++) {
			stringstream sstr;
			sstr << setfill('0') << std::setw(8U) << hex << (short)value;
			string res = sstr.str();
			toLittleEndian(&res);
			(*stream) << res;
		}
	}
}

void SecondPass::parseDataDef(string *line) {
	vector<string> *cnt = new vector<string>;
	if (line->find(" ") != -1) {
		string ss = *line;
		while (ss.find(',') != -1) {
			FirstPass::removeSpaces(&ss);
			int i = ss.find(',');
			string a = ss.substr(0, i);
			cnt->push_back(a);
			ss = ss.substr(i + 1);
		}
		FirstPass::removeSpaces(&ss);
		if (ss != "")
			cnt->push_back(ss);
	}
	
	string l = *line;
	if (l.find("db") != -1) {
		if (section->getName() == ".bss" && cnt->size() != 0) {}
			//throw string("Podaci ne mogu da se inicijalizuju u .bss sekciji.");
		l = l.substr(3);
		parseDB(&l);
	}
	else if (l.find("dw") != -1) {
		if (section->getName() == ".bss" && cnt->size() != 0) {}
			//throw string("Podaci ne mogu da se inicijalizuju u .bss sekciji.");
		l = l.substr(3);
		parseDW(&l);
	}
	else if (l.find("dd") != -1) {
		if (section->getName() == ".bss" && cnt->size() != 0) {}
			//throw string("Podaci ne mogu da se inicijalizuju u .bss sekciji.");
		l = l.substr(3);
		parseDD(&l);
	}
}

void SecondPass::parseDB(string *line) {
	vector<string> *arg = new vector<string>;
	string ss = *line;
	while (ss.find(',') != -1) {
		FirstPass::removeSpaces(&ss);
		int i = ss.find(',');
		string a = ss.substr(0, i);
		arg->push_back(a);
		ss = ss.substr(i + 1);
	}
	FirstPass::removeSpaces(&ss);
	if (ss != "")
		arg->push_back(ss);

	if (arg->size() == 0) {
		(*stream) << "00 ";
		lc += 1;
	}
	else {
		//lc += 1 * arg->size();
		for (unsigned int i = 0; i < arg->size(); i++) {
			string a = (*arg)[i];
			list<string> l;
			l = shuntingYard->itop(a, all_symbols, 1);
			int value = shuntingYard->resolve(l);

			if (shuntingYard->needsRelocation(a, table, section) != NULL) {
				Symbol *sym = shuntingYard->needsRelocation(a, table, section);
				Relocation *r = new Relocation(lc, /*chooseRelType(sym)*/'A', chooseValue(sym));
				reloc->put(r);
			}

			stringstream sstr;
			sstr << setfill('0') << std::setw(2U) << hex << value;
			string res = sstr.str();
			res = res.substr(res.size() - 2);
			separateCode(&res);
			(*stream) << res;

			lc += 1;
		}
	}
}

void SecondPass::parseDW(string *line) {
	vector<string> *arg = new vector<string>;
	string ss = *line;
	while (ss.find(',') != -1) {
		FirstPass::removeSpaces(&ss);
		int i = ss.find(',');
		string a = ss.substr(0, i);
		arg->push_back(a);
		ss = ss.substr(i + 1);
	}
	FirstPass::removeSpaces(&ss);
	if (ss != "")
		arg->push_back(ss);

	if (arg->size() == 0) {
		(*stream) << "00 00 ";
		lc += 2;
	}
	else {
		//lc += 2 * arg->size();
		for (unsigned int i = 0; i < arg->size(); i++) {
			string a = (*arg)[i];
			list<string> l;
			l = shuntingYard->itop(a, all_symbols, 1);
			int value = shuntingYard->resolve(l);

			if (shuntingYard->needsRelocation(a, table, section) != NULL) {
				Symbol *sym = shuntingYard->needsRelocation(a, table, section);
				Relocation *r = new Relocation(lc, /*chooseRelType(sym)*/'A', chooseValue(sym));
				reloc->put(r);
			}
			
			stringstream sstr;
			sstr << setfill('0') << std::setw(4U) << hex << value;
			string res = sstr.str();
			res = res.substr(res.size() - 4);
			toLittleEndian(&res);
			(*stream) << res;

			lc += 2;
		}
	}
}

void SecondPass::parseDD(string *line) {
	vector<string> *arg = new vector<string>;
	string ss = *line;
	while (ss.find(',') != -1) {
		FirstPass::removeSpaces(&ss);
		int i = ss.find(',');
		string a = ss.substr(0, i);
		arg->push_back(a);
		ss = ss.substr(i + 1);
	}
	FirstPass::removeSpaces(&ss);
	if (ss != "")
		arg->push_back(ss);

	if (arg->size() == 0) {
		(*stream) << "00 00 00 00 || ";
		lc += 4;
	}
	else {
		//lc += 4 * arg->size();
		for (unsigned int i = 0; i < arg->size(); i++) {
			string a = (*arg)[i];
			list<string> l;
			l = shuntingYard->itop(a, all_symbols, 1);
			int value = shuntingYard->resolve(l);

			if (shuntingYard->needsRelocation(a, table, section) != NULL) {
				Symbol *sym = shuntingYard->needsRelocation(a, table, section);
				Relocation *r = new Relocation(lc, /*chooseRelType(sym)*/'A', chooseValue(sym));
				reloc->put(r);
			}

			stringstream sstr;
			sstr << setfill('0') << std::setw(8U) << hex << value;
			string res = sstr.str();
			toLittleEndian(&res);
			(*stream) << res;

			lc += 4;
		}
	}
}

void SecondPass::parseORG(string *line) {
	org_happened = true;
	int i = line->find_first_of("org");
	string address = line->substr(i + 4);
	list<string> l = shuntingYard->itop(address, all_symbols, 1);
	org_address = shuntingYard->resolve(l);
}

void SecondPass::parseArithLog(string *line) {
	FirstPass::removeSpaces(line);
	int pos = line->find_first_of(" ");
	string op = line->substr(0, pos);

	string l = line->substr(pos + 1);
	FirstPass::removeSpaces(&l);
	
	pos = l.find_first_of(",");
	string res = l.substr(0, pos);
	l = l.substr(pos + 1);
	FirstPass::removeSpaces(&l);

	pos = l.find_first_of(",");
	string op1 = l.substr(0, pos);
	l = l.substr(pos + 1);
	FirstPass::removeSpaces(&l);

	string op2 = l;

	stringstream sstr;
	bitset<8> op_code(codes->find(op)->second);
	bitset<3> addr_mode(0);
	bitset<5> reg0(codes->find(res)->second);
	bitset<5> reg1(codes->find(op1)->second);
	bitset<5> reg2(codes->find(op2)->second);
	bitset<6> un(0);

	string binary_string = "0b" + op_code.to_string() + addr_mode.to_string() + reg0.to_string() + reg1.to_string() + reg2.to_string() + un.to_string();
	long value = FirstPass::fromBinary(binary_string);
	sstr << setfill('0') << setw(8U) << hex << value;
	string r = sstr.str();
	separateCode(&r);
	(*stream) << r;
}

void SecondPass::parseStack(string *line) {
	FirstPass::removeSpaces(line);
	int pos = line->find_first_of(" ");
	string op = line->substr(0, pos);

	string l = line->substr(pos + 1);
	FirstPass::removeSpaces(&l);
	string reg = l;

	stringstream sstr;
	bitset<8> op_code(codes->find(op)->second);
	bitset<3> addr_mode(0);
	bitset<5> reg0(codes->find(reg)->second);
	bitset<16> un(0);

	string binary_string = "0b" + op_code.to_string() + addr_mode.to_string() + reg0.to_string() + un.to_string();
	long value = FirstPass::fromBinary(binary_string);
	sstr << setfill('0') << setw(8U) << hex << value;
	string r = sstr.str();
	separateCode(&r);
	(*stream) << r;
}

int SecondPass::chooseType(string *suffix) {
	if (*suffix == "ub" || *suffix == "b")
		return codes->find("dbz")->second;
	else if (*suffix == "sb")
		return codes->find("dbs")->second;
	else if (*suffix == "uw" || *suffix == "w")
		return codes->find("dwz")->second;
	else if (*suffix == "sw")
		return codes->find("dws")->second;
	else
		return 0;
}

void SecondPass::parseLoadStore(string *line) {
	FirstPass::removeSpaces(line);
	int pos = line->find_first_of(" ");
	string op = line->substr(0, pos);
	string suffix = "";

	if (op != "load" && op != "store") {
		if (op.substr(0, 4) == "load")
			suffix = op.substr(4);
		else
			suffix = op.substr(5);
	}

	if (op.substr(0, 4) == "load")
		op = op.substr(0, 4);
	else if (op.substr(0, 5) == "store")
		op = op.substr(0, 5);

	string l = line->substr(pos + 1);
	FirstPass::removeSpaces(&l);
	pos = l.find_first_of(",");

	string op1 = l.substr(0, pos);

	string op2 = l.substr(pos + 1);
	FirstPass::removeSpaces(&op2);

	if (op2.find_first_of('#') != string::npos) {
		if (op == "store")
			throw string("Ne moze se koristiti neposredno adresiranje u instrukciji store.");

		op2 = op2.substr(1);

		list<string> list;
		list = shuntingYard->itop(op2, all_symbols, 1);
		int v = shuntingYard->resolve(list);

		if (shuntingYard->needsRelocation(op2, table, section) != NULL) {
			Symbol *sym = shuntingYard->needsRelocation(*line, table, section);
			Relocation *r = new Relocation(lc + 4, /*chooseRelType(sym)*/'A', chooseValue(sym));
			reloc->put(r);
		}

		stringstream sstr1;
		stringstream sstr2;
		bitset<8> op_code(codes->find(op)->second);
		bitset<3> addr_mode(codes->find("immed")->second);
		bitset<5> reg0(0);
		bitset<5> reg1(codes->find(op1)->second);
		bitset<5> un1(0);
		bitset<3> type(chooseType(&suffix));
		bitset<3> un2(0);
		bitset<32> disp(v);

		string binary_string = "0b" + op_code.to_string() + addr_mode.to_string() + reg0.to_string() + reg1.to_string() + un1.to_string() + type.to_string() + un2.to_string();
		long value = FirstPass::fromBinary(binary_string);
		sstr1 << setfill('0') << setw(8U) << hex << value;
		string r = sstr1.str();
		separateCode(&r);
		(*stream) << r;

		binary_string = "0b" + disp.to_string();
		//value = FirstPass::fromBinary(binary_string);
		unsigned long val = disp.to_ulong();
		sstr2 << setfill('0') << setw(8U) << hex << val;
		r = sstr2.str();
		toLittleEndian(&r);
		(*stream) << r;

		lc += 8;
	}
	else {
		if (check->isRegister(&op2)) {
			stringstream sstr;
			bitset<8> op_code(codes->find(op)->second);
			bitset<3> addr_mode(codes->find("regdir")->second);
			bitset<5> reg0(codes->find(op1)->second);
			bitset<5> reg1(codes->find(op2)->second);
			bitset<5> un1(0);
			bitset<3> type(chooseType(&suffix));
			bitset<3> un2(0);

			string binary_string = "0b" + op_code.to_string() + addr_mode.to_string() + reg0.to_string() + reg1.to_string() + un1.to_string() + type.to_string() + un2.to_string();
			long value = FirstPass::fromBinary(binary_string);
			sstr << setfill('0') << setw(8U) << hex << value;
			string r = sstr.str();
			separateCode(&r);
			(*stream) << r;

			lc += 4;
		}
		else if (op2[0] == '[') {
			if (op2.find_first_of('+') != string::npos) {
				pos = op2.find_first_of("+");
				string op2_resolve = op2.substr(pos + 1, op2.size() - pos - 2);
				FirstPass::removeSpaces(&op2_resolve);

				list<string> list;
				list = shuntingYard->itop(op2_resolve, all_symbols, 1);
				int v = shuntingYard->resolve(list);

				if (shuntingYard->needsRelocation(op2_resolve, table, section) != NULL) {
					Symbol *sym = shuntingYard->needsRelocation(op2_resolve, table, section);
					Relocation *r = new Relocation(lc + 4, /*chooseRelType(sym)*/'A', chooseValue(sym));
					reloc->put(r);
				}

				op2 = op2.substr(1, 2);
				FirstPass::removeSpaces(&op2);

				// TODO: proveri jos jednom, ali trebalo bi da se obrnu mesta argumenata za reg0 i reg1
				// sada je kao u postavci!!!!!!!!
				stringstream sstr1;
				stringstream sstr2;
				bitset<8> op_code(codes->find(op)->second);
				bitset<3> addr_mode(codes->find("regindpom")->second);
				bitset<5> reg0(codes->find(op2)->second);
				bitset<5> reg1(codes->find(op1)->second);
				bitset<5> un1(0);
				bitset<3> type(chooseType(&suffix));
				bitset<3> un2(0);
				bitset<32> disp(v);

				string binary_string = "0b" + op_code.to_string() + addr_mode.to_string() + reg0.to_string() + reg1.to_string() + un1.to_string() + type.to_string() + un2.to_string();
				long value = FirstPass::fromBinary(binary_string);
				sstr1 << setfill('0') << setw(8U) << hex << value;
				string r = sstr1.str();
				separateCode(&r);
				(*stream) << r;

				binary_string = "0b" + disp.to_string();
				//value = FirstPass::fromBinary(binary_string);
				unsigned long val = disp.to_ulong();
				sstr2 << setfill('0') << setw(8U) << hex << val;
				r = sstr2.str();
				toLittleEndian(&r);
				(*stream) << r;

				lc += 8;
			}
			else {
				pos = op2.find_first_of("]");
				op2 = op2.substr(1, pos - 1);
				FirstPass::removeSpaces(&op2);

				stringstream sstr;
				bitset<8> op_code(codes->find(op)->second);
				bitset<3> addr_mode(codes->find("regind")->second);
				bitset<5> reg0(codes->find(op1)->second);
				bitset<5> reg1(codes->find(op2)->second);
				bitset<5> un1(0);
				bitset<3> type(chooseType(&suffix));
				bitset<3> un2(0);

				string binary_string = "0b" + op_code.to_string() + addr_mode.to_string() + reg0.to_string() + reg1.to_string() + un1.to_string() + type.to_string() + un2.to_string();
				long value = FirstPass::fromBinary(binary_string);
				sstr << setfill('0') << setw(8U) << hex << value;
				string r = sstr.str();
				separateCode(&r);
				(*stream) << r;
				lc += 4;
			}
		}
		else if(op2[0] == '$') {
			op2 = op2.substr(1);

			list<string> list;
			list = shuntingYard->itop(op2, all_symbols, 1);
			int v = shuntingYard->resolve(list) - lc - 8;

			if (shuntingYard->needsRelocation(op2, table, section) != NULL) {
				Symbol *sym = shuntingYard->needsRelocation(op2, table, section);
				Relocation *r = new Relocation(lc + 4, /*chooseRelType(sym)*/'R', chooseValue(sym));
				reloc->put(r);
			}

			stringstream sstr1;
			stringstream sstr2;
			bitset<8> op_code(codes->find(op)->second);
			bitset<3> addr_mode(codes->find("regindpom")->second);
			bitset<5> reg0(codes->find(op1)->second);
			bitset<5> reg1(codes->find("pc")->second);
			bitset<5> un1(0);
			bitset<3> type(chooseType(&suffix));
			bitset<3> un2(0);
			bitset<32> disp(v);

			string binary_string = "0b" + op_code.to_string() + addr_mode.to_string() + reg0.to_string() + reg1.to_string() + un1.to_string() + type.to_string() + un2.to_string();
			long value = FirstPass::fromBinary(binary_string);
			sstr1 << setfill('0') << setw(8U) << hex << value;
			string r = sstr1.str();
			separateCode(&r);
			(*stream) << r;

			binary_string = "0b" + disp.to_string();
			//value = FirstPass::fromBinary(binary_string);
			unsigned long val = disp.to_ulong();
			sstr2 << setfill('0') << setw(8U) << hex << val;
			r = sstr2.str();
			toLittleEndian(&r);
			(*stream) << r;

			lc += 8;
		}
		else {
			list<string> list;
			list = shuntingYard->itop(op2, all_symbols, 1);
			int v = shuntingYard->resolve(list);

			if (shuntingYard->needsRelocation(op2, table, section) != NULL) {
				Symbol *sym = shuntingYard->needsRelocation(op2, table, section);
				Relocation *r = new Relocation(lc + 4, /*chooseRelType(sym)*/'A', chooseValue(sym));
				reloc->put(r);
			}

			stringstream sstr1;
			stringstream sstr2;
			bitset<8> op_code(codes->find(op)->second);
			bitset<3> addr_mode(codes->find("memdir")->second);
			bitset<5> reg0(0);
			bitset<5> reg1(codes->find(op1)->second);
			bitset<5> un1(0);
			bitset<3> type(chooseType(&suffix));
			bitset<3> un2(0);
			bitset<32> disp(v);

			string binary_string = "0b" + op_code.to_string() + addr_mode.to_string() + reg0.to_string() + reg1.to_string() + un1.to_string() + type.to_string() + un2.to_string();
			long value = FirstPass::fromBinary(binary_string);
			sstr1 << setfill('0') << setw(8U) << hex << value;
			string r = sstr1.str();
			separateCode(&r);
			(*stream) << r;

			binary_string = "0b" + disp.to_string();
			//value = FirstPass::fromBinary(binary_string);
			unsigned long val = disp.to_ulong();
			sstr2 << setfill('0') << setw(8U) << hex << val;
			r = sstr2.str();
			toLittleEndian(&r);
			(*stream) << r;

			lc += 8;
		}
	}
}

void SecondPass::parseFlowControl(string *line) {

	if (line->substr(0, 3).find("int") == 0 || line->substr(0, 3).find("ret") == 0) {
		if (line->substr(0, 3).find("int") == 0) {
			FirstPass::removeSpaces(line);
			int pos = line->find_first_of(" ");
			string op = line->substr(0, 3);

			string l = line->substr(pos + 1);
			FirstPass::removeSpaces(&l);
			string op1 = l;

			stringstream sstr;
			bitset<8> op_code(codes->find(op)->second);
			bitset<3> addr_mode(codes->find("regdir")->second);
			bitset<5> reg0(codes->find(op1)->second);
			bitset<16> un(0);

			string binary_string = "0b" + op_code.to_string() + addr_mode.to_string() + reg0.to_string() + un.to_string();
			long value = FirstPass::fromBinary(binary_string);
			sstr << setfill('0') << setw(8U) << hex << value;
			string r = sstr.str();
			separateCode(&r);
			(*stream) << r;
		}
		else {
			stringstream sstr;
			bitset<8> op_code(codes->find("ret")->second);
			bitset<24> addr_mode(0);

			string binary_string = "0b" + op_code.to_string() + addr_mode.to_string();
			long value = FirstPass::fromBinary(binary_string);
			sstr << setfill('0') << setw(8U) << hex << value;
			string r = sstr.str();
			separateCode(&r);
			(*stream) << r;
		}
		lc += 4;
	}
	else if (line->substr(0, 3).find("jmp") == 0 || line->substr(0, 4).find("call") == 0) {
		FirstPass::removeSpaces(line);
		int pos = line->find_first_of(" ");
		string op = line->substr(0, pos);

		string l = line->substr(pos + 1);
		FirstPass::removeSpaces(&l);
		string op1 = l;

		if (check->isRegister(&op1)) {
			stringstream sstr;
			bitset<8> op_code(codes->find(op)->second);
			bitset<3> addr_mode(codes->find("regdir")->second);
			bitset<5> reg0(codes->find(op1)->second);
			bitset<16> un(0);

			string binary_string = "0b" + op_code.to_string() + addr_mode.to_string() + reg0.to_string() + un.to_string();
			long value = FirstPass::fromBinary(binary_string);
			sstr << setfill('0') << setw(8U) << hex << value;
			string r = sstr.str();
			separateCode(&r);
			(*stream) << r;

			lc += 4;
		}
		else if (op1[0] == '[') {
			if (op1.find_first_of('+') != string::npos) {
				pos = op1.find_first_of("+");
				string op1_resolve = op1.substr(pos + 1, op1.size() - pos - 2);
				FirstPass::removeSpaces(&op1_resolve);

				list<string> list;
				list = shuntingYard->itop(op1_resolve, all_symbols, 1);
				int v = shuntingYard->resolve(list);

				if (shuntingYard->needsRelocation(op1_resolve, table, section) != NULL) {
					Symbol *sym = shuntingYard->needsRelocation(op1_resolve, table, section);
					Relocation *r = new Relocation(lc + 4, /*chooseRelType(sym)*/'A', chooseValue(sym));
					reloc->put(r);
				}

				op1 = op1.substr(1, 2);
				FirstPass::removeSpaces(&op1);

				stringstream sstr1;
				stringstream sstr2;
				bitset<8> op_code(codes->find(op)->second);
				bitset<3> addr_mode(codes->find("regindpom")->second);
				bitset<5> reg0(codes->find(op1)->second);
				bitset<16> un(0);
				bitset<32> disp(v);
				
				unsigned long test = disp.to_ulong();

				string binary_string = "0b" + op_code.to_string() + addr_mode.to_string() + reg0.to_string() + un.to_string();
				long value = FirstPass::fromBinary(binary_string);
				sstr1 << setfill('0') << setw(8U) << hex << value;
				string r = sstr1.str();
				separateCode(&r);
				(*stream) << r;

				binary_string = "0b" + disp.to_string();
				//value = FirstPass::fromBinary(binary_string);
				unsigned long val = disp.to_ulong();
				sstr2 << setfill('0') << setw(8U) << hex << val;
				r = sstr2.str();
				toLittleEndian(&r);
				(*stream) << r;

				lc += 8;
			}
			else {
				pos = op1.find_first_of("]");
				op1 = op1.substr(1, pos - 1);
				FirstPass::removeSpaces(&op1);

				stringstream sstr;
				bitset<8> op_code(codes->find(op)->second);
				bitset<3> addr_mode(codes->find("regind")->second);
				bitset<5> reg0(codes->find(op1)->second);
				bitset<16> un(0);

				string binary_string = "0b" + op_code.to_string() + addr_mode.to_string() + reg0.to_string() + un.to_string();
				long value = FirstPass::fromBinary(binary_string);
				sstr << setfill('0') << setw(8U) << hex << value;
				string r = sstr.str();
				separateCode(&r);
				(*stream) << r;

				lc += 4;
			}
		}
		else if (op1[0] == '$') {
			op1 = op1.substr(1);

			list<string> list;
			list = shuntingYard->itop(op1, all_symbols, 1);
			int v = shuntingYard->resolve(list) - lc - 8;

			if (shuntingYard->needsRelocation(op1, table, section) != NULL) {
				Symbol *sym = shuntingYard->needsRelocation(op1, table, section);
				Relocation *r = new Relocation(lc + 4, /*chooseRelType(sym)*/'R', chooseValue(sym));
				reloc->put(r);
			}

			stringstream sstr1;
			stringstream sstr2;
			bitset<8> op_code(codes->find(op)->second);
			bitset<3> addr_mode(codes->find("regindpom")->second);
			bitset<5> reg0(codes->find("pc")->second);
			bitset<16> un(0);
			bitset<32> disp(v);

			string binary_string = "0b" + op_code.to_string() + addr_mode.to_string() + reg0.to_string() + un.to_string();
			long value = FirstPass::fromBinary(binary_string);
			sstr1 << setfill('0') << setw(8U) << hex << value;
			string r = sstr1.str();
			separateCode(&r);
			(*stream) << r;

			binary_string = "0b" + disp.to_string();
			//value = FirstPass::fromBinary(binary_string);
			unsigned long val = disp.to_ulong();
			sstr2 << setfill('0') << setw(8U) << hex << val;
			r = sstr2.str();
			toLittleEndian(&r);
			(*stream) << r;

			lc += 8;
		}
		else if (!check->isRegister(&op1)) {
			list<string> list;
			list = shuntingYard->itop(op1, all_symbols, 1);
			int v = shuntingYard->resolve(list);

			if (shuntingYard->needsRelocation(op1, table, section) != NULL) {
				Symbol *sym = shuntingYard->needsRelocation(op1, table, section);
				Relocation *r = new Relocation(lc + 4, /*chooseRelType(sym)*/'A', chooseValue(sym));
				reloc->put(r);
			}

			stringstream sstr1;
			stringstream sstr2;
			bitset<8> op_code(codes->find(op)->second);
			bitset<3> addr_mode(codes->find("memdir")->second);
			bitset<5> reg0(0);
			bitset<16> un(0);
			bitset<32> disp(v);

			string binary_string = "0b" + op_code.to_string() + addr_mode.to_string() + reg0.to_string() + un.to_string();
			long value = FirstPass::fromBinary(binary_string);
			sstr1 << setfill('0') << setw(8U) << hex << value;
			string r = sstr1.str();
			separateCode(&r);
			(*stream) << r;

			binary_string = "0b" + disp.to_string();
			//value = FirstPass::fromBinary(binary_string);
			unsigned long val = disp.to_ulong();
			sstr2 << setfill('0') << setw(8U) << hex << val;
			r = sstr2.str();
			toLittleEndian(&r);
			(*stream) << r;

			lc += 8;
		}
	}
	else if (line->substr(0, 2).find("jz") == 0 || line->substr(0, 3).find("jnz") == 0 || line->substr(0, 3).find("jgz") == 0 || line->substr(0, 4).find("jgez") == 0 || line->substr(0, 3).find("jlz") == 0 || line->substr(0, 4).find("jlez") == 0) {
		FirstPass::removeSpaces(line);
		int pos = line->find_first_of(" ");
		string op = line->substr(0, pos);

		string l = line->substr(pos + 1);
		FirstPass::removeSpaces(&l);
		pos = l.find_first_of(",");

		string op1 = l.substr(0, pos);
		
		string op2 = l.substr(pos + 1);
		FirstPass::removeSpaces(&op2);
		
		if (check->isRegister(&op2)) {
			stringstream sstr;
			bitset<8> op_code(codes->find(op)->second);
			bitset<3> addr_mode(codes->find("regdir")->second);
			bitset<5> reg0(codes->find(op1)->second);
			bitset<5> reg1(codes->find(op2)->second);
			bitset<11> un(0);

			string binary_string = "0b" + op_code.to_string() + addr_mode.to_string() + reg0.to_string() + reg1.to_string() + un.to_string();
			long value = FirstPass::fromBinary(binary_string);
			sstr << setfill('0') << setw(8U) << hex << value;
			string r = sstr.str();
			separateCode(&r);
			(*stream) << r;

			lc += 4;
		}
		else if (op2[0] == '[') {
			if (op2.find_first_of('+') != string::npos) {
				pos = op2.find_first_of("+");
				string op2_resolve = op2.substr(pos + 1, op2.size() - pos - 2);
				FirstPass::removeSpaces(&op2_resolve);

				list<string> list;
				list = shuntingYard->itop(op2_resolve, all_symbols, 1);
				int v = shuntingYard->resolve(list);

				if (shuntingYard->needsRelocation(op2_resolve, table, section) != NULL) {
					Symbol *sym = shuntingYard->needsRelocation(op2_resolve, table, section);
					Relocation *r = new Relocation(lc + 4, /*chooseRelType(sym)*/'A', chooseValue(sym));
					reloc->put(r);
				}

				op2 = op2.substr(1, 2);
				FirstPass::removeSpaces(&op2);

				stringstream sstr1;
				stringstream sstr2;
				bitset<8> op_code(codes->find(op)->second);
				bitset<3> addr_mode(codes->find("regindpom")->second);
				bitset<5> reg0(codes->find(op1)->second);
				bitset<5> reg1(codes->find(op2)->second);
				bitset<11> un(0);
				bitset<32> disp(v);

				string binary_string = "0b" + op_code.to_string() + addr_mode.to_string() + reg0.to_string() + reg1.to_string() + un.to_string();
				long value = FirstPass::fromBinary(binary_string);
				sstr1 << setfill('0') << setw(8U) << hex << value;
				string r = sstr1.str();
				separateCode(&r);
				(*stream) << r;

				binary_string = "0b" + disp.to_string();
				//value = FirstPass::fromBinary(binary_string);
				unsigned long val = disp.to_ulong();
				sstr2 << setfill('0') << setw(8U) << hex << val;
				r = sstr2.str();
				toLittleEndian(&r);
				(*stream) << r;

				lc += 8;
			}
			else {
				pos = op2.find_first_of("]");
				op2 = op2.substr(1, pos - 1);
				FirstPass::removeSpaces(&op2);

				stringstream sstr;
				bitset<8> op_code(codes->find(op)->second);
				bitset<3> addr_mode(codes->find("regind")->second);
				bitset<5> reg0(codes->find(op1)->second);
				bitset<5> reg1(codes->find(op2)->second);
				bitset<11> un(0);

				string binary_string = "0b" + op_code.to_string() + addr_mode.to_string() + reg0.to_string() + reg1.to_string() + un.to_string();
				long value = FirstPass::fromBinary(binary_string);
				sstr << setfill('0') << setw(8U) << hex << value;
				string r = sstr.str();
				separateCode(&r);
				(*stream) << r;

				lc += 4;
			}
		}
		else if (op2[0] == '$') {
			op2 = op2.substr(1);

			list<string> list;
			list = shuntingYard->itop(op2, all_symbols, 1);
			int v = shuntingYard->resolve(list) - lc - 8;

			if (shuntingYard->needsRelocation(op2, table, section) != NULL) {
				Symbol *sym = shuntingYard->needsRelocation(op2, table, section);
				Relocation *r = new Relocation(lc + 4, /*chooseRelType(sym)*/'R', chooseValue(sym));
				reloc->put(r);
			}

			stringstream sstr1;
			stringstream sstr2;
			bitset<8> op_code(codes->find(op)->second);
			bitset<3> addr_mode(codes->find("regindpom")->second);
			bitset<5> reg0(codes->find(op1)->second);
			bitset<5> reg1(codes->find("pc")->second);
			bitset<11> un(0);
			bitset<32> disp(v);

			string binary_string = "0b" + op_code.to_string() + addr_mode.to_string() + reg0.to_string() + reg1.to_string() + un.to_string();
			long value = FirstPass::fromBinary(binary_string);
			sstr1 << setfill('0') << setw(8U) << hex << value;
			string r = sstr1.str();
			separateCode(&r);
			(*stream) << r;

			binary_string = "0b" + disp.to_string();
			//value = FirstPass::fromBinary(binary_string);
			unsigned long val = disp.to_ulong();
			sstr2 << setfill('0') << setw(8U) << hex << val;
			r = sstr2.str();
			toLittleEndian(&r);
			(*stream) << r;

			lc += 8;
		}
		else if (!check->isRegister(&op2)) {
			list<string> list;
			list = shuntingYard->itop(op2, all_symbols, 1);
			int v = shuntingYard->resolve(list);

			if (shuntingYard->needsRelocation(op2, table, section) != NULL) {
				Symbol *sym = shuntingYard->needsRelocation(op2, table, section);
				Relocation *r = new Relocation(lc + 4, /*chooseRelType(sym)*/'A', chooseValue(sym));
				reloc->put(r);
			}

			stringstream sstr1;
			stringstream sstr2;
			bitset<8> op_code(codes->find(op)->second);
			bitset<3> addr_mode(codes->find("memdir")->second);
			bitset<5> reg0(codes->find(op1)->second);
			bitset<5> reg1(0);
			bitset<11> un(0);
			bitset<32> disp(v);

			string binary_string = "0b" + op_code.to_string() + addr_mode.to_string() + reg0.to_string() + reg1.to_string() + un.to_string();
			long value = FirstPass::fromBinary(binary_string);
			sstr1 << setfill('0') << setw(8U) << hex << value;
			string r = sstr1.str();
			separateCode(&r);
			(*stream) << r;

			binary_string = "0b" + disp.to_string();
			//value = FirstPass::fromBinary(binary_string);
			unsigned long val = disp.to_ulong();
			sstr2 << setfill('0') << setw(8U) << hex << val;
			r = sstr2.str();
			toLittleEndian(&r);
			(*stream) << r;

			lc += 8;
		}
	}
}

void SecondPass::toLittleEndian(string *line) {
	string *result = new string();
	for (int i = line->size() - 2; i >= 0; i -= 2) {
		string s = line->substr(i, 2);
		*result += s + ' ';
	}

	*line = *result;
}

void SecondPass::separateCode(string *line) {
	string *result = new string();
	for (unsigned int i = 0; i <= line->size() - 2; i += 2) {
		string s = line->substr(i, 2);
		*result += s + ' ';
	}

	*line = *result;
}

void SecondPass::fillCodes() {
	// Address Mode Codes
	(*codes)["immed"]		= 4;
	(*codes)["regdir"]		= 0;
	(*codes)["memdir"]		= 6;
	(*codes)["regind"]		= 2;
	(*codes)["regindpom"]	= 7;

	// Register Codes
	(*codes)["r0"]	= 0;
	(*codes)["r1"]	= 1;
	(*codes)["r2"]	= 2;
	(*codes)["r3"]	= 3;
	(*codes)["r4"]	= 4;
	(*codes)["r5"]	= 5;
	(*codes)["r6"]	= 6;
	(*codes)["r7"]	= 7;
	(*codes)["r8"]	= 8;
	(*codes)["r9"]	= 9;
	(*codes)["r10"]	= 10;
	(*codes)["r11"] = 11;
	(*codes)["r12"] = 12;
	(*codes)["r13"] = 13;
	(*codes)["r14"] = 14;
	(*codes)["r15"] = 15;
	(*codes)["sp"]	= 16;
	(*codes)["pc"]	= 17;

	// Data Type Codes
	(*codes)["dd"]	= 0;
	(*codes)["dwz"] = 1;
	(*codes)["dws"] = 5;
	(*codes)["dbz"] = 3;
	(*codes)["dbs"] = 7;

	// ArithLog
	(*codes)["add"] = 48;
	(*codes)["sub"] = 49;
	(*codes)["mul"] = 50;
	(*codes)["div"] = 51;
	(*codes)["mod"] = 52;
	(*codes)["and"] = 53;
	(*codes)["or"]	= 54;
	(*codes)["xor"] = 55;
	(*codes)["not"] = 56;
	(*codes)["asl"] = 57;
	(*codes)["asr"] = 58;

	// Stack
	(*codes)["push"]	= 32;
	(*codes)["pop"]		= 33;

	// FlowControl
	(*codes)["int"] = 0;
	(*codes)["jmp"] = 2;
	(*codes)["call"] = 3;
	(*codes)["ret"] = 1;
	(*codes)["jz"] = 4;
	(*codes)["jnz"] = 5;
	(*codes)["jgz"] = 6;
	(*codes)["jgez"] = 7;
	(*codes)["jlz"] = 8;
	(*codes)["jlez"] = 9;

	(*codes)["load"] = 16;
	(*codes)["store"] = 17;
}

int	SecondPass::chooseValue(Symbol *symbol) {
	/*if (symbol->getNumOfSection_sym() == 0 || (symbol->getNumOfSection_sym() != 0 && symbol->getNumOfSection_sym() != -1 && symbol->getFlags() == "G"))
		return symbol->getNum();
	else if (symbol->getNumOfSection_sym() != 0 && symbol->getNumOfSection_sym() != -1)
		return symbol->getNumOfSection_sym();*/
	if (symbol->getFlags() == "G")
		return symbol->getNum();
	else
		return symbol->getNumOfSection_sym();
}

char SecondPass::chooseRelType(Symbol *symbol) {
	if (table->get(symbol->getName())->getNumOfSection_sym() != section->getNumOfSection_sec())
		return 'A';
	else
		return 'R';
}
#include "Check.h"
#include "FirstPass.h"

Check::Check() {}

Check::~Check() {}

// checking if the line is a label definition
bool Check::isLabel(string *line) {
	bool result = (line->find_first_of(":") != string::npos) ? true : false;
	return result;
}

// checking if the line is a section definition
bool Check::isSection(string *line) {
	FirstPass::removeSpaces(line);
	int pos = line->find_first_of(" ");
	string temp = line->substr(0, pos);

	bool result = ((temp.find(".text") == 0) || (temp.find(".data") == 0) || (temp.find(".rodata") == 0) || (temp.find(".bss") == 0)) ? true : false;
	return result;
}

bool Check::isGlobal(string * line) {
	FirstPass::removeSpaces(line);
	int pos = line->find_first_of(" ");
	string temp = line->substr(0, pos);

	bool result = (temp == ".global") ? true : false;
	return result;
}

bool Check::isDataDef(string *line) {
	FirstPass::removeSpaces(line);
	int pos = line->find_first_of(" ");
	string temp = line->substr(0, pos);

	bool result = ((temp.find("db") == 0) || (temp.find("dw") == 0) || (temp.find("dd") == 0)/* || (temp.find(".bss") == 0)*/) ? true : false;
	return result;
}

bool Check::isORG(string *line) {
	FirstPass::removeSpaces(line);
	int pos = line->find_first_of(" ");
	string temp = line->substr(0, pos);

	bool result = (temp.find("org") == 0);
	return result;
}

bool Check::isDEF(string *line) {
	FirstPass::removeSpaces(line);
	int pos = line->find_first_of(" ");
	string temp = line->substr(pos + 1);
	pos = temp.find_first_of(" ");
	temp = temp.substr(0, pos);
	bool result = (temp.find("def") == 0);
	return result;
}

bool Check::isDUP(string *line) {
	FirstPass::removeSpaces(line);
	int pos = line->find_first_of(" ");
	string temp = line->substr(pos + 1);
	istringstream iss(temp);
	std::string token;
	while (getline(iss, token, ' ')) {
		if (token.find("dup") == 0)
			return true;
	}
	return false;
}

bool Check::isBinary(string *line) {
	FirstPass::removeSpaces(line);
	string temp = line->substr(0, 2);
	bool result = (temp.find("0b") == 0);
	return result;
}

bool Check::isHex(string *line) {
	FirstPass::removeSpaces(line);
	string temp = line->substr(0, 2);
	bool result = (temp.find("0x") == 0);
	return result;
}

bool Check::isText(string *line) {
	const char *chars = line->c_str();
	int num = atoi(line->c_str());
	if (num == 0 && chars[0] != '0')
		return true;
	else
		return false;
}

bool Check::isFlowControl(string *line) {
	FirstPass::removeSpaces(line);
	string temp = line->substr(0, 4);
	if (temp.find("call") == 0 || temp.find("jgez") == 0 || temp.find("jlez") == 0)
		return true;
	temp = line->substr(0, 3);
	if (temp.find("int") == 0 || temp.find("jmp") == 0 || temp.find("ret") == 0 || temp.find("jnz") == 0 || temp.find("jgz") == 0 || temp.find("jlz") == 0)
		return true;
	temp = line->substr(0, 2);
	if (temp.find("jz") == 0)
		return true;
	return false;
}

bool Check::isLoadStore(string *line) {
	FirstPass::removeSpaces(line);
	bool result = (line->substr(0, 5).find("store") == 0 || line->substr(0, 4).find("load") == 0);
	return result;
}

bool Check::isStack(string *line) {
	FirstPass::removeSpaces(line);
	string temp = line->substr(0, 4);
	bool result = (temp.find("push") == 0 || temp.substr(0, 3).find("pop") == 0);
	return result;
}

bool Check::isArithLog(string *line) {
	FirstPass::removeSpaces(line);
	string temp = line->substr(0, 3);
	bool result = (temp.find("add") == 0 || temp.find("sub") == 0 || temp.find("mul") == 0 || temp.find("div") == 0 ||
		temp.find("mod") == 0 || temp.find("and") == 0 || temp.substr(0, 2).find("or") == 0 || temp.find("xor") == 0 ||
		temp.find("not") == 0 || temp.find("asl") == 0 || temp.find("asr") == 0);
	return result;
}

bool Check::isRegister(string *line) {
	FirstPass::removeSpaces(line);
	string temp = line->substr(0, 3);
	bool result = (temp.substr(0, 2).find("r0") == 0 || temp.substr(0, 2).find("r1") == 0 || temp.substr(0, 2).find("r2") == 0 || temp.substr(0, 2).find("r3") == 0 ||
		temp.substr(0, 2).find("r4") == 0 || temp.substr(0, 2).find("r5") == 0 || temp.substr(0, 2).find("r6") == 0 || temp.substr(0, 2).find("r7") == 0 ||
		temp.substr(0, 2).find("r8") == 0 || temp.substr(0, 2).find("r9") == 0 || temp.find("r10") == 0 || temp.find("r11") == 0 || temp.find("r12") == 0 ||
		temp.find("r13") == 0 || temp.find("r14") == 0 || temp.find("r15") == 0 || temp.substr(0, 2).find("pc") == 0 || temp.substr(0, 2).find("sp") == 0);
	return result;
}

#pragma once

#include <string>
#include <sstream>
#include <cstdlib>
using namespace std;

class Check {
public:
	Check();
	~Check();

	bool static isLabel		(string *line);
	bool static isSection	(string *line);
	bool static isGlobal	(string *line);
	bool static isDataDef	(string *line);
	bool static isORG		(string *line);
	bool static isDEF		(string *line);
	bool static isDUP		(string *line);
	bool static isBinary	(string *line);
	bool static isHex		(string *line);
	bool static isText		(string *line);

	bool static isFlowControl	(string *line);
	bool static isLoadStore		(string *line);
	bool static isStack			(string *line);
	bool static isArithLog		(string *line);

	bool static isRegister	(string *line);

	// bool instInt	(string* line);
	bool static instRac	(string* line);
	bool static instBit	(string* line);
	bool static instLS	(string* line);
	bool static instCall(string* line);
	bool static instIO	(string* line);
	bool static instPom	(string* line);
	bool static instLdc	(string* line);
};

#include "Symbol.h"

int Symbol::increment = 0;

// constructor for section
Symbol::Symbol(string name, unsigned int addr, string flags) : name(name), address(addr), flags(flags) {
	type = "SEG";
	num = numOfSection_sec = increment++;
	size = 0;
}

// constructor for symbol
Symbol::Symbol(string name, int section, unsigned int value, string flag) : name(name), numOfSection_sym(section), value(value), flags(flag) {
	type = "SYM";
	num = increment++;
}

// destructor
Symbol::~Symbol() {}

// getters
string			Symbol::getType()				{ return type; }
unsigned int	Symbol::getNum()				{ return num; }
string			Symbol::getName()				{ return name; }
string			Symbol::getFlags()				{ return flags; }
unsigned int	Symbol::getNumOfSection_sec()	{ return numOfSection_sec; }
int				Symbol::getNumOfSection_sym()	{ return numOfSection_sym; }
unsigned int	Symbol::getAddress()			{ return address; }
unsigned int	Symbol::getSize()				{ return size; }
unsigned int	Symbol::getValue()				{ return value; }

// setters
void Symbol::setType(string t)						{ type = t; }
void Symbol::setNum(unsigned int n)					{ num = n; }
void Symbol::setName(string n)						{ name = n; }
void Symbol::setFlags(string f)						{ flags = f; }
void Symbol::setNumOfSection_sec(unsigned int s)	{ numOfSection_sec = s; }
void Symbol::setNumOfSection_sym(unsigned int s)	{ numOfSection_sym = s; }
void Symbol::setAddress(unsigned int a)				{ address = a; }
void Symbol::setSize(unsigned int s)				{ size = s; }
void Symbol::setValue(unsigned int v)				{ value = v; }
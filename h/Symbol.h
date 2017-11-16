#pragma once

#include <string>
using namespace std;

class Symbol {
public:
	// constructor for sections
    Symbol(string name, unsigned int addr, string flags);
	// constructor for symbols
	Symbol(string name, int section, unsigned int value, string flag);
	~Symbol();
	
	string			getType();
	unsigned int	getNum();
	string			getName();
	string			getFlags();

	unsigned int	getNumOfSection_sec();
	int				getNumOfSection_sym();

	unsigned int	getAddress();
	unsigned int	getSize();

	unsigned int	getValue();


	void setType(string t);
	void setNum(unsigned int n);
	void setName(string n);
	void setFlags(string f);

	void setNumOfSection_sec(unsigned int s);
	void setNumOfSection_sym(unsigned int s);
	
	void setAddress(unsigned int a);
	void setSize(unsigned int s);
	void setValue(unsigned int v);

private:
	string			type;
	unsigned int	num;
	string			name;
	string			flags;
	
	unsigned int	numOfSection_sec;
	int				numOfSection_sym;

	unsigned int	address;
	unsigned int	size;

	unsigned int	value;

	static int increment;
};

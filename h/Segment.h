#pragma once

#include <string>
using namespace std;

class Segment {
public:
	Segment(unsigned int num, string name, unsigned int sec, unsigned int addr, unsigned int s, string f);
	~Segment();

	void	setSection(unsigned int s);
	void	setAddress(unsigned int addr);
	void	setSize(unsigned int s);
	void	setFlags(string f);

	unsigned int	getSection();
	unsigned int	getAddress();
	unsigned int	getSize();
	string			getFlags();

private:
	unsigned int	section;
	unsigned int	address;	// 0x...
	unsigned int	size;		// 0x...
	string			flags;
};

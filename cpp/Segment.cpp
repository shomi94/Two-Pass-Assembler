#include "Segment.h"

Segment::Segment(unsigned int num, string name, unsigned int sec, unsigned int addr, unsigned int s, string f) : /*AbsSym("Segment", num, name), */section(sec), address(addr), size(s), flags(f) {}

Segment::~Segment() {}

void	Segment::setSection(unsigned int s)		{ section = s; }
void	Segment::setAddress(unsigned int addr)	{ address = addr; }
void	Segment::setSize(unsigned int s)		{ size = s; }
void	Segment::setFlags(string f)				{ flags = f; }

unsigned int	Segment::getSection()	{ return section; }
unsigned int	Segment::getAddress()	{ return address; }
unsigned int	Segment::getSize()		{ return size; }
string			Segment::getFlags()		{ return flags; }
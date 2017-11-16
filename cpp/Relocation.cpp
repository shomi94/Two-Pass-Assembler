#include "Relocation.h"

// constructor
Relocation::Relocation(int off, char t, int val) : address(off), type(t), num(val) { }

// destructor
Relocation::~Relocation() { }

// setters
void Relocation::setAddress(int off)	{ address = off; }
void Relocation::setType(char t)		{ type = t; }
void Relocation::setNum(int val)		{ num = val; }

//getters
unsigned int Relocation::getAddress()	{ return address; }
char Relocation::getType()				{ return type; }
unsigned int Relocation::getNum()		{ return num; }
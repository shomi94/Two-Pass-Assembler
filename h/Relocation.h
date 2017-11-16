#pragma once

class Relocation {
public:
    Relocation(int off, char t, int val);
    ~Relocation();

    void setAddress(int off);
    void setType(char t);
    void setNum(int val);

    unsigned int    getAddress();
    char			getType();
    unsigned int	getNum();

private:
    unsigned int address;
    char type;
    int num;
};

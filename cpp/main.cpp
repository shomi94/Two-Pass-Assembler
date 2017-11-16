#include "SymTab.h"
#include "FirstPass.h"
#include "SecondPass.h"
#include "ShuntingYard.h"

#include <iostream>
#include <string>
#include <vector>
#include <fstream>

using namespace std;

int main(int argc, char *argv[]) {
    string      readFile = argv[1];
	//string		readFile = "test6.txt";
	ofstream    writeFile;
	writeFile.open(argv[2]);
	//writeFile.open("test6_output.txt");

	cout << readFile << "\n\n\n";
    SymTab *table = new SymTab();
	vector<Symbol *> *symbols = new vector<Symbol *>;
    try {
		vector<string> *asmText = new vector<string>;
		asmText = FirstPass::read(readFile);

		/*for (vector<string>::iterator it = asmText->begin(); it != asmText->end(); ++it)
			cout << *it << "\n";*/

		FirstPass   *firstPass = new FirstPass(symbols);
		table = firstPass->pass(asmText);
		table->dumpTable(&writeFile);
        
		SecondPass  *secondPass = new SecondPass(symbols);
		secondPass->pass(table, asmText, &writeFile);

		string s = "0x20 +(( 5-2)-(x/50)) * 0b101";
		//string s = "a";
		ShuntingYard *sy = new ShuntingYard();

		/*vector<Symbol *> *v = new vector<Symbol *>;
		v->push_back(new Symbol("x", 1, 853, "L"));
		list<string> lista = sy->itop(s, v, 1);
		cout << "\n\n";
		for (std::list<string>::iterator it = lista.begin(); it != lista.end(); ++it)
			std::cout << ' ' << *it;
		cout << " = " << sy->resolve(lista) << '\n';*/

		cout << "Asembliranje je zavrseno." << "\n";
	}
	catch (string err) {
		cout << err;
	}

	writeFile.close();

	return 0;
}

#include "ShuntingYard.h"
#include "FirstPass.h"

ShuntingYard::ShuntingYard() {
	check = new Check();
}

ShuntingYard::~ShuntingYard() { }

list<string> ShuntingYard::itop(string str, vector<Symbol *> *all_symbols, int mode) {
	relocLabel = " ";
	stack<string> stack;
	string operand;
	bool end = false;
	string push = "";
	list<string> postfix;

	vector<string> *ops = ShuntingYard::separateOps(str);

	for (vector<string>::iterator i = ops->begin(); i != ops->end(); ++i) {
		string scan = *i;
		if (scan == " " || scan == ",") {
			continue;
		}
		else if (isOperator(scan)) {
			if (!operand.empty())
				postfix.push_back(operand);
			operand = "";
			end = false;
			while (!stack.empty() && stack.top() != "(" && checkPrecedence(stack.top(), scan)) {
				if (!stack.top().empty())
					postfix.push_back(stack.top());
				stack.pop();
			}
			push += scan;
			stack.push(push);
			push = "";
		}
		else if (scan == "(") {
			if (!operand.empty()) {
				postfix.push_back(operand);
				operand = "";
				end = false;
			}
			push += scan;
			stack.push(push);
			push = "";
		}
		else if (scan == ")") {
			if (!operand.empty())
				postfix.push_back(operand);
			operand = "";
			end = false;
			while (!stack.empty() && stack.top() != "(") {
				if (!stack.top().empty())
					postfix.push_back(stack.top());
				stack.pop();
			}
			stack.pop();
		}
		else if (!end) {
			if (mode == 1) {
				int number;
				if (check->isBinary(&scan)) {
					number = FirstPass::fromBinary(scan);
					scan = to_string(number);
				}
				else if (check->isHex(&scan)) {
					number = FirstPass::fromHex(scan);
					scan = to_string(number);
				}
				else if (scan[0] == '\'') {
					number = scan[1];
					scan = to_string(number);
				}
				else if (check->isText(&scan)) {
					std::vector<Symbol *>::iterator it;
					string help = "~";
					for (it = all_symbols->begin(); it != all_symbols->end(); ++it) {
						if ((*it)->getName() == scan) {
							help = to_string((*it)->getValue());
							break;
						}
					}
					if (help == "~")
						scan = to_string(0); // ako je labela extern
					else
						scan = help;
				}
				operand += scan;
			}
			else {
				int number;
				if (check->isBinary(&scan)) {
					number = FirstPass::fromBinary(scan);
					scan = to_string(number);
				}
				else if (check->isHex(&scan)) {
					number = FirstPass::fromHex(scan);
					scan = to_string(number);
				}
				operand += scan;
			}
		}
	}

	if (!operand.empty())
		postfix.push_back(operand);
	while (!stack.empty()) {
		if (!stack.top().empty())
			postfix.push_back(stack.top());
		stack.pop();
	}

	return postfix;
}

int ShuntingYard::getWeight(string op) {
	int weight = -1;
	if (op == "+" || op == "-")
		weight = 1;
	else if (op == "*" || op == "/")
		weight = 2;
	else if (op == "$")
		weight = 3;
	return weight;
}

bool ShuntingYard::isRightAssoc(string op) {
	return op == "$" ? true : false;
}

int ShuntingYard::checkPrecedence(string op1, string op2) {
	int weight1 = getWeight(op1);
	int weight2 = getWeight(op2);
	if (weight1 == weight2)
		return isRightAssoc(op1) ? false : true;
	return weight1 > weight2 ? true : false;
}

bool ShuntingYard::isOperator(string op) {
	if(op == "+" || op == "-" || op == "*" || op == "/")
		return true;
	return false;
}

// TODO: za sad se nigde ne koristi, ne znam da l' ce u buducnosti da mi treba
bool ShuntingYard::isOperand(string op) {
	if (!check->isBinary(&op) && !check->isHex(&op) && !check->isText(&op) && atoi(op.c_str()) == 0 && op[0] != '#' && op[0] != '$')
		return false;
	else return true;
}

vector<string>* ShuntingYard::separateOps(string str) {
	vector<string> *ops = new vector<string>;
	while (str.find(" ") != -1)
		str.replace(str.find(" "), 1, "");
	// analyzing the string to get operands
	FirstPass::removeSpaces(&str);
	string temp = str;
	std::size_t prev = 0, pos;
	while ((pos = str.find_first_of("+-*/()", prev)) != string::npos) {
		if (pos > prev) {
			ops->push_back(str.substr(prev, pos - prev));
			ops->push_back(str.substr(pos, 1));
		}
		else
			ops->push_back(str.substr(pos, 1));
		prev = pos + 1;
	}
	if (prev < str.length())
		ops->push_back(str.substr(prev, string::npos));
	return ops;
}

int ShuntingYard::resolve(list<string> postfix) {
	stack<int> stack;
	int op1;
	int op2;
	int r;
	int result = 0;
	stack.push(0);
	for (list<string>::iterator i = postfix.begin(); i != postfix.end(); ++i) {
		string next = *i;
		if (atoi(next.c_str()) != 0 || next == "0")
			stack.push(atoi(next.c_str()));
		else if (next == "+" || next == "-" || next == "*" || next == "/") {
			op2 = stack.top();
			stack.pop();
			op1 = stack.top();
			stack.pop();
			if (next == "+")
				r = op1 + op2;
			else if (next == "-")
				r = op1 - op2;
			else if (next == "*")
				r = op1 * op2;
			else
				r = op1 / op2;
			stack.push(r);
		} 
	}
	result = stack.top();
	stack.pop();
	return result;
}

string ShuntingYard::getRelocLabel() {
	return relocLabel;
}

void ShuntingYard::setRelocLabel(string label) {
	relocLabel = label;
}

Symbol* ShuntingYard::needsRelocation(string line, SymTab *table, Symbol *currentSection) {
	vector<string> *ops = ShuntingYard::separateOps(line);
	vector<string> candidates;
	string no_spaces = " ";
	int count = 0;

	for (vector<string>::iterator it = ops->begin(); it != ops->end(); it++) {
		string temp = *it;
		if (!check->isBinary(&temp) && !check->isHex(&temp) && !isOperator(temp) && temp != "(" && temp != ")" && check->isText(&temp) && temp != "?" && temp[0] != '\'') {
			if (table->get(temp)->getNumOfSection_sym() != -1) {// ako nije konstanta
				candidates.push_back(temp);
				count++;
			}
		}
		no_spaces += temp;
	}

	if ((count/* % 2*/) != 0) {
		if (count == 1) {
			Symbol *section = table->getByNumber(table->get(candidates.front())->getNumOfSection_sym());
			/*if (section->getAddress() != 0 || section == currentSection)
				return NULL;
			else*/
				return table->get(candidates.front());
		}

		while (!candidates.empty()) {
			string temp = candidates.front();

			int pos = no_spaces.find(temp);
			if (no_spaces[pos - 1] == '+' || no_spaces[pos - 1] == ' ' || (no_spaces[pos - 1] != '-' && no_spaces[pos - 1] != '(')) {
				/*Symbol *section = table->getByNumber(table->get(temp)->getNumOfSection_sym());
				if(section->getAddress() == 0)*/
					return table->get(temp);
			}
			no_spaces = no_spaces.substr(pos + temp.size());
			candidates.erase(candidates.begin());
		}
		return NULL;
	} else
		return NULL;
}
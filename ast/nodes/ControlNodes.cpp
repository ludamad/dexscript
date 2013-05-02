/*   DexScript scripting language, a value-based scripting language.
 *   Mutation must be made explicit. The language makes heavy use of
 *   copy-on-write techniques.
 *
 *   Copyright (C) 2012 Adam Domurad
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.*/

#include "ControlNodes.h"
#include "nodes.h"
#include <iostream>
#include "../../types/dexstdops.h"
using namespace std;

BaseNode* ForNode::optimize(int optlevel){
	if (optlevel){
		optimize_node(value, optlevel);
		optimize_node(collection, optlevel);
		optimize_all(statements, optlevel);
	}
	return this;
}
BaseNode* IfNode::optimize(int optlevel){
	if (optlevel){
		optimize_node(condition, optlevel);
		optimize_all(trueb, optlevel);
		optimize_all(falseb, optlevel);
	}
	return this;
}
BaseNode* WhileNode::optimize(int optlevel){
	if (optlevel){
		optimize_node(condition, optlevel);
		optimize_all(statements, optlevel);
	}
	return this;
}

DexRef ForNode::eval(VarContext& vars) {
	DexRef col = collection->eval(vars);
	size_t init_data[4];
	DexRef iterval;
	bool domore = dex_init_iter(iterval, col, init_data);
	while (domore) {
		*value->set_eval(vars) = iterval;
		if (!ifcondition || dex_is_true(ifcondition->eval(vars)) ){
			DexRef val = handle_statements(vars, statements, true);
			if (val.ref != DNONE_NOACTION) {
				if (val.ref == DNONE_BREAK)
					return DNONE_NOACTION;
				return val;
			}
		}
		domore = dex_incr_iter(iterval, col, init_data);
	}
	return DNONE_NOACTION;

}

DexRef IfNode::eval(VarContext& vars){
	DexRef val = DNONE_NOACTION;
	bool istrue = dex_is_true(condition->eval(vars));
	val = handle_statements(vars, istrue ? trueb : falseb, false);
	return val;
}

DexRef WhileNode::eval(VarContext& vars){
	while (dex_is_true(condition->eval(vars))){
		DexRef val = handle_statements(vars, statements, true);
		if (val.ref != DNONE_NOACTION) {
			if (val.ref == DNONE_BREAK)
				return DNONE_NOACTION;
			return val;
		}
	}
	return DNONE_NOACTION;
}

DexRef JumpNode::eval(VarContext& vars){
	if (optional)
		return optional->eval(vars);
	if (type == CONTINUE)
		return DNONE_CONTINUE;
	else if (type == BREAK)
		return DNONE_BREAK;
	return NONE;
}

ForNode::~ForNode() {
	delete value;
	delete collection;
	delete ifcondition;
	for (size_t i = 0; i < statements.size(); i++)
		delete statements[i];
}

IfNode::~IfNode() {
	delete condition;
	for (size_t i = 0; i < trueb.size(); i++)
		delete trueb[i];
	for (size_t i = 0; i < falseb.size(); i++)
		delete falseb[i];
}


WhileNode::~WhileNode() {
	delete condition;
	for (size_t i = 0; i < statements.size(); i++)
		delete statements[i];
}

JumpNode::~JumpNode() {
	delete optional;
}




void ForNode::print(int indent) {
	int i = indent;
	while (i--)
		cout << '\t';

	cout << "for ";
	value->print(0);
	cout << " in ";
	collection->print(0);
	cout << ":\n";
	int s = statements.size();
	for (i = 0; i < s; i++) {
		statements[i]->print(indent + 1);
		if (i != s)
			cout << '\n';
	}
}
void IfNode::print(int indent){
	int i = indent;
	while(i--) cout << '\t';

	cout << "if ";
	condition->print(0);
	cout << ":\n";
	size_t s = trueb.size();
	for (size_t i = 0; i < s; i++){
		trueb[i]->print(indent+1);
		cout << '\n';
	}
	if (!falseb.empty()){
		cout << "else:\n";
		s = falseb.size();
		for (size_t i = 0; i < s; i++){
			falseb[i]->print(indent+1);
			cout << '\n';
		}
	}
}

void WhileNode::print(int indent){
	int i = indent;
	while(i--) cout << '\t';

	cout << "while ";
	condition->print(0);
	cout << ":\n";
	size_t s = statements.size();
	for (size_t i = 0; i < s; i++){
		statements[i]->print(indent+1);
		if (i != s)
			cout << '\n';
	}
}

void JumpNode::print(int indent){
	while(indent--) cout << '\t';
	if (type == CONTINUE)
		cout << "continue" << endl;
	else if (type == BREAK)
		cout << "break" << endl;
	else {
		cout << "return ";
		optional->print(0);
	}
}


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

#include "ast.h"
#include "../parsing/lex.h"
#include "../parsing/nparse.h"
#include "../interp/error.h"
#include "../types/dexmodule.h"
#include "../types/util/strmake.h"

#include <cstring>
#include <iostream>

using namespace std;

void compile_file(AST& ast, const char* filename) {
	FILE* f = fopen(filename, "r");
	if (f == NULL) {
		error(IO_ERROR, str_make("Error loading module from file \"", filename,
				"\"."));
		return;
	}
	int line_num = 1;
	LexContext lc;
	while (!feof(f)) {
		char buff[500] = "";
		if (!fgets(buff, 499, f))
			break;
		size_t siz = strlen(buff);
		LexLine ll = lex_line(buff, siz, line_num);
		if (error())
			break;
		lc.update(ll);
		if (error())
			break;
		line_num++;
	}
	//lc.print();
	if (!error())
		parse_to_tree(ast, lc);
	ast.optimize(3);

	//ast.print();

	fclose(f);
}

DexRef AST::eval(VarContext& vars) {
	DexRef& last = dex_state.module;
	if (root.empty())
		return NONE;
	DexRef n;
	for (size_t i = 0; i < root.size(); i++) {
		n = root[i]->eval(vars);
		if (error())
			return NONE;
	}
	if (n->type == &noneType)
		n = NONE;
	dex_state.module = last;
	return n;
}

AST::~AST() {
	for (size_t i = 0; i < root.size(); i++)
		delete root[i];
}
DexRef AST::eval() {
	VarContext varc(dex_state.module);
	return eval(varc);
}

DexRef AST::partial_eval(VarContext& vars, bool stmnts, bool defs) {
	if (root.empty())
		return NONE;
	DexRef n;
	for (size_t i = 0; i < root.size(); i++) {
		bool isdef = root[i]->is_def();
		if ((defs && isdef) || (stmnts && !isdef)){
			//root[i]->print(0);
			n = root[i]->eval(vars);
		}
		if (error())
			return NONE;
	}
	if (n->type == &noneType)
		n = NONE;
	return n;
}
void AST::optimize(int optlevel) {
	optimize_all(&root[0], root.size(), optlevel);
}
void AST::print() {
	for (size_t i = 0; i < root.size(); i++) {
		root[i]->print(0);
		std::cout << '\n';
	}
}


AST::AST(const char* string, int optlevel){
	LexContext lc;
	lc.set_from_string(string);
	parse_to_tree(*this, lc, optlevel);
}

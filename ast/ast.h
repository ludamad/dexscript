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

#ifndef AST_H_
#define AST_H_
#include "nodes/BaseNode.h"
#include <vector>
#include <iostream>
#include "../interp/VarContext.h"
//Used to represent a series of commands
struct AST {
	std::vector<BaseNode*> root;
	AST(const char* string, int optlevel = 1);
	AST(){}
	~AST();
	DexRef eval(VarContext& vars);
	DexRef eval();
	DexRef partial_eval(VarContext& vars, bool stmnts, bool defs);
	void optimize(int optlevel);
	void print();
	void concat(const AST& ast){
		root.insert(root.end(), ast.root.begin(), ast.root.end());
	}
};
void compile_file(AST& ast, const char* filename);
#endif /* AST_H_ */

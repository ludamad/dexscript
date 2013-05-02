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

#ifndef NPARSE_H_
#define NPARSE_H_
#include "lex.h"
#include "../ast/ast.h"

void parse_to_tree(AST& ast, const LexContext& lc, int optlevel = 0);

void parse_and_add_methods(DexType* type, const LexContext& lc, int optlevel = 0);

inline void parse_and_add_methods(DexType* type, const char* str, int optlevel = 0){
	LexContext lc;
	lc.set_from_string(str);
	parse_and_add_methods(type, lc, optlevel);
}
#endif /* NPARSE_H_ */

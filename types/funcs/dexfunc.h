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

#ifndef DEXFUNC_H_
#define DEXFUNC_H_
#include "../dextype.h"
#include "../typetools.h"
#include "../../DexRef.h"
#include "../../types/util/VarToVect.h"
#include "../../interp/interp_state.h"
#include <vector>

struct BaseNode;
struct ASTParam {
	DexRef name;
	BaseNode* opt;
	ASTParam(){
		opt = NULL;
	}
	~ASTParam();
};

//Base for AST function
struct DexFunc {
	DexRef module;
	VarToVect varmap;
	size_t minargs, maxargs;
	DexRef name, fname;
	std::vector<BaseNode*> body;
	ASTParam* params;
	void print(int indent);
	DexFunc(){
		module = dex_state.module;
		minargs = 0;
		maxargs = 0;
		params = NULL;
	}
	~DexFunc();
};
DexObj* create_func();
inline DexFunc* get_dex_func(DexObj* dobj) {
	return (DexFunc*) get_dex_memb(dobj, 0);
}
DexRef func_call(const DexRef& fn, DexRef* args, size_t n);
void meth_eqcall(const DexRef& fn, DexRef& obj, DexRef* args, size_t n);
DexRef meth_call(const DexRef& fn, const DexRef& obj, DexRef* args, size_t n);
//Keep the data the same but have a separate type for methods
//Methods cannot be called directly
//method.bind(a)(b,c,d) = a.method(b,c,d)
extern DexType methType;

extern DexType funcType;
#endif /* DEXFUNC_H_ */

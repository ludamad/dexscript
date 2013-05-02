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

#ifndef VARCONTEXT_H_
#define VARCONTEXT_H_
#include "../DexRef.h"
#include "../types/funcs/dexfunc.h"
#include "../types/util/VoidDict.h"

/**
 * Represents the current function/global context
 * Does no validation of access
 */
struct VarContext {
	DexRef mod;
	DexRef* self;
	VoidDict* vars;
	DexRef* info;

	DexRef get_global(const DexRef& str);
	DexRef get_global(int i);
	int get_global_ind(const DexRef& str);
	int get_local_ind(const DexRef& str);
	DexRef* pget_global(const DexRef& str);
	void set_global(const DexRef& str, const DexRef& val);

	inline VarContext(DexRef module) : mod(module){
		vars = NULL;
		info = NULL;
		self = NULL;
	}
	inline VarContext(DexRef module, VoidDict* v, DexRef* ptr, DexRef* s = NULL) : mod(module) {
		vars = v;
		info = ptr;
		self = s;
	}
	inline VarContext(DexFunc* func, DexRef* ptr, DexRef* s = NULL) : mod(func->module){
		vars = &func->varmap.varDict;
		info = ptr;
		self = s;
	}

	inline DexRef get_local(const DexRef& str) {
		return info[(int)(long)vars->get((void*) str.ref)-1];
	}
	inline DexRef get_local(int i) {
		return info[i];
	}

	inline DexRef* pget_local(const DexRef& str) {
		int* ptr = (int*)vars->pget((void*)str.ref);
		if (*ptr){
			return &info[*ptr - 1];
		}
		else
			return NULL;
	}
	inline void set_local(const DexRef& str, const DexRef& val) {
		info[(int)(long)vars->get((void*) str.ref)-1] = val;
	}
};
#endif /* VARCONTEXT_H_ */

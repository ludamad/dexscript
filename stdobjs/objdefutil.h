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

#ifndef OBJDEFUTIL_H_
#define OBJDEFUTIL_H_

#include "math.h"
#include "../mainh.h"

inline bool is_num(DexRef* args, size_t n, size_t argneed) {
	if (n != argneed)
		return false;
	return (args[0]->type != &numType);
}

template<typename F>
DexRef dex_num1f(DexRef* args, size_t n) {
	bool islist = args[0]->type == &listType;
	bool isref1 = args[0]->refc <= 1;
	if (n != 1 || (args[0]->type != &numType && !islist)) {
		error(ARG_ERROR, "Number or numeric list expected.");
		stack_trace(StackFrame(create_string(F::name()), F::file()));
		return NONE;
	}

	if (islist) {
		DexRef dobj = args[0];
		if (!isref1)
			dobj = dex_copy(dobj.ref, sizeof(DexList));
		DexList* dl = get_dex_list(dobj.ref);
		for (int i = 0; i < dl->len; i++) {
			DexObj* num = dl->data[i].ref;
			if (num->type != &numType) {
				error(ARG_ERROR, "Number or numeric list expected.");
				stack_trace(StackFrame(create_string(F::name()), F::file()));
				return NONE;
			}
			dl->data[i] = F::func(get_dex_num(dl->data[i].ref));
		}
		return dobj;
	}

	double& d = get_dex_num(args->ref);
	double val = F::func(d);

	if (args->ref->refc == 1) {
		d = val;
		return args->ref;
	}

	return create_number(val);
}
template<typename F>
DexRef dex_num2f(DexRef* args, size_t n) {
	DexType* det = args[0]->type;
	bool islist = det == &listType;
	bool isref1 = args->ref->refc <= 1;
	if (n != 2 || (det != args[1]->type) || (det != &numType && islist)) {
		error(ARG_ERROR, "Number or numeric list expected.");
		stack_trace(StackFrame(create_string(F::name()), F::file()));
		return NONE;
	}
	double& d = get_dex_num(args[0].ref);
	double val = F::func(d, get_dex_num(args[1].ref));
	if (isref1) {
		d = val;
		return args->ref;
	}
	return create_number(val);
}

template<typename F>
void dex_add_num1f(DexRef& dict) {
	dict_put(dict, dex_intern(F::name()), create_builtin(&dex_num1f<F> ,
			F::name()));
}

template<typename F>
void dex_add_num2f(DexRef& dict) {
	dict_put(dict, dex_intern(F::name()), create_builtin(&dex_num2f<F> ,
			F::name()));
}

inline void dex_put_builtin(DexRef& dict, const DexRef& blt) {
	DexBuiltin* db = get_dex_builtin(blt.ref);
	dict_put(dict, dex_intern(db->name), blt);
}

inline void dex_put_gen(DexRef& dict, const DexRef& blt) {
	DexGen* dg = get_dex_gen(blt.ref);
	dict_put(dict, dex_intern(dg->name), blt);
}

inline void dex_put_intern(DexRef& dict, const DexRef& str, const DexRef& dr) {
	dict_put(dict, dex_intern(str), dr);
}

inline void dex_put_intern(DexRef& dict, const char* c, size_t n,
		const DexRef& dr) {
	dict_put(dict, dex_intern(c, n), dr);
}

#endif /* OBJDEFUTIL_H_ */

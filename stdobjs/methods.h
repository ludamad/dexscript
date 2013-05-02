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

#ifndef METHODS_H_
#define METHODS_H_

#include "../DexRef.h"
#include "../types/dextype.h"
#include "str.h"
#include "../types/dexlist.h"

void dm_objstr(DexRef& self, DexRef* args, size_t n);
void dm_objhash(DexRef& self, DexRef* args, size_t n);
void dm_objreinit(DexRef& self, DexRef* args, size_t n);

inline void add_method(DexType* det, const DexRef& str, DexBnM* method) {
	void** ret = det->cmethMap.pget((void*) str.ref);
	if (*ret == NULL)
		list_push(det->mstrList, str);
	*ret = (void*) method;
}
inline void add_getter(DexType* det, const DexRef& str, void* getter) {
	void** ret = det->cmembMap.pget((void*) str.ref);
	if (*ret == NULL)
		list_push(det->gettersList, str);
	*ret = getter;
}
inline void add_method(DexType* det, const DexRef& str, const DexRef& meth) {
	if (str.ref == DISTR_INIT.ref) {
		det->uconstr = meth;
	} else {
		void** ret = det->dmethMap.pget((void*) str.ref);
		if (*ret == NULL) {
			list_push(det->mdexList, meth);
			list_push(det->mstrList, str);
		}
		*ret = (void*) meth.ref;
	}
}

inline int add_member(DexType* det, const DexRef& str) {
	int* ret = (int*) det->dmembMap.pget(str.ref);
	//Check the key
	if (*ret == 0) {
		list_push(det->mstrList, str);
		*ret = ++det->membn;
	}
	return *ret - 1;
}

extern const char NOARGS[];
extern const char ONEARGS[];
extern const char TWOARGS[];
#endif /* METHODS_H_ */

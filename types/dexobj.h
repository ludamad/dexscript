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

#ifndef DEXOBJ_H_
#define DEXOBJ_H_
#include <cstdlib>
const int INTERN_REF = 1 << 30;
const int REST_MASK = ~INTERN_REF;

struct DexType;
struct DexRef;
struct DexCStr {
	const char* str;
	size_t len;
	inline DexCStr(const char* s, size_t n) {
		str = s;
		len = n;
	}
};

struct DexObj {
	DexType* type;
	int refc;//Intern-ing ors INTERN_REF to this
	size_t len;
};

DexRef create_dex_object(DexType* dext, DexRef* args, size_t n);
void destroy_dex_object(DexObj* dobj);

inline void dex_ref(DexObj* d) {
	d->refc++;
}
void remove_interned(DexObj* d);



const int TEMP_REF = 10000;//Keeps object temporarily alive
inline void dex_unref(DexObj* d) {
	d->refc--;
	if (d->refc & INTERN_REF) {
		//Only intern ref + this ref left
		//Take out of intern...
		//Delete the object
		if ((d->refc & REST_MASK) <= 1) {
			d->refc ^= INTERN_REF;
			remove_interned(d);
		}
	} else {

		if (d->refc <= 0) {
			destroy_dex_object(d);
		}
	}
}
inline bool is_interned(DexObj* d){
	return d->refc & INTERN_REF;
}
//Wont destroy object or change intern
inline void dex_safe_unref(DexObj* d) {
	d->refc--;
}

void default_destroy(DexObj*);

DexRef none_construct(DexType* dext, DexRef* args, size_t argn);
extern DexType noneType;
extern DexObj tombobj;//in dexnone.cpp
extern DexObj noneobj;

static DexObj* const NONE = (&noneobj);
static DexObj* const TOMBSTONE = (&tombobj);

//Guarantees the very important intern-set is initialized first
//This allows for interned constants to be defined easily
//This is later defined in interp_state.h
class InitGuard {
    static int refc;
public:
    InitGuard();
};

static InitGuard __interp_init;

void preinit_none();

#endif /* DEXOBJ_H_ */

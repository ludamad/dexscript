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

#ifndef TYPETOOLS_H_
#define TYPETOOLS_H_
#include <cstring>
#include "dexobj.h"
#include <cstdio>
using namespace std;
#include "../DexRef.h"

struct DexType;
inline void init_dex_base(DexObj* dobj, DexType* t, size_t alen) {
	dobj->type = t;
	dobj->refc = 0;
	dobj->len = alen;

}

inline DexObj* alloc_dex_base(DexType* t, size_t alen) {
	DexObj* dobj = (DexObj*) malloc(alen);
	init_dex_base(dobj, t, alen);
	return dobj;
}

inline void* get_dex_memb(DexObj* dobj, size_t wordn) {
	return sizeof(DexObj) + (char*) (wordn + (size_t*) dobj);
}
#ifdef _MSC_VER
#include <intrin.h>

inline size_t roundUpPower2(unsigned long n) {
   unsigned long r;
   int i = _BitScanReverse(&r, n);
	return 1 << (r+1);
}
#else
inline size_t roundUpPower2(size_t n) {
	//TODO: make non-gnu backup version
	return 1 << ((sizeof(size_t) * 8) - __builtin_clz(n));
}
#endif
inline DexObj* alloc_dex_copy_base(DexObj* d, size_t clen, size_t alen) {
	DexObj* r = (DexObj*) malloc(alen);
	std::memcpy(r, d, clen);//Copies type ptr etc
	r->refc = 0;
	return r;
}

//Fills an object with 0's / None's
//Assumes plain data until objects start
inline void dex_clr(DexObj* d, size_t byte_border = 0) {
	char* ds = (char*) (d + 1);
	std::memset(ds, 0, byte_border);
	DexObj** dr = (DexObj**) (byte_border + ds);
	DexObj** end = (DexObj**) (d->len + (char*) d);
	NONE->refc += (end - dr);
	while (dr < end) {
		dex_unref(*dr);
		*dr = NONE;
		dr++;
	}
}

inline DexObj* dex_copy(DexObj* d, size_t byte_border = 0) {
	DexObj* dob = alloc_dex_copy_base(d, d->len, d->len);
	DexObj** dr = (DexObj**) (byte_border + (char*) (dob + 1));
	DexObj** end = (DexObj**) (d->len + (char*) dob);
	while (dr < end) {
		dex_ref(*dr);
		dr++;
	}
	return dob;
}

inline void dex_init_clr(DexObj* d, size_t byte_border = 0) {
	char* ds = (char*) (d + 1);
	std::memset(ds, 0, byte_border);
	DexRef* dr = (DexRef*) (byte_border + ds);
	dex_manual_initv(dr, ((DexRef*)(d->len+(char*)d))- dr);
}
#endif /* TYPETOOLS_H_ */

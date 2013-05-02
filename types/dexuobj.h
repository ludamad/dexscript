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

#ifndef DEXUOBJ_H_
#define DEXUOBJ_H_
#include "../DexRef.h"
#include "dextype.h"
#include "typetools.h"

//Object for a user defined type
struct DexUObj{
	//Used to add unknown members
	DexRef dataDict;
	//Used for known members (with static indices)
	DexRef membData[0];
};
inline DexUObj* get_dex_uobj(DexObj* d){
	return (DexUObj*)get_dex_memb(d,0);
}
DexRef uobj_get(const DexRef& uobj, const DexRef& memb);
DexRef* uobj_pget(DexRef& uobj, const DexRef& memb);

void uobj_memb_eqcall(const DexRef& memb, DexRef& uobj, DexRef* args, size_t n);

void add_uobj_methods(DexType* det);

int uobj_compare(const DexRef& u1, const DexRef& u2);

bool uobj_incr_iter(DexRef& it, const DexRef& cont, void* iter);
DexRef uobj_call(const DexRef& d, DexRef* args, size_t n);
DexRef uobj_oget(const DexRef& d, const DexRef& k);
void uobj_oput(DexRef& d, const DexRef& k, const DexRef& v);


inline bool is_utype(DexType* det){
	return det->typeinit == &add_uobj_methods;
}
inline DexRef& uobj_get(const DexRef& uobj, size_t membn){
	return get_dex_uobj(uobj.ref)->membData[membn];
}
DexRef create_uobj(DexType* utype, DexRef* args, size_t n);
DexType* create_utype(const DexRef& name, const DexRef& methMap, const DexRef& membList);


#endif /* DEXUOBJ_H_ */

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

#include "dexbound.h"
#include <cstdio>
#include "../dexstdops.h"
#include "../dextype.h"
#include "../util/strmake.h"
#include <new>

using namespace std;

DexObj* create_bound(const DexRef& self, const DexRef& memb) {
	DexObj* ret = alloc_dex_base(&boundType, sizeof(DexObj) + sizeof(DexBound));
	DexBound* db = get_dex_bound(ret);
	new (db) DexBound(self, memb);
	return ret;
}
DexRef bound_call(const DexRef& bn, DexRef* args, size_t n) {
	DexBound* db = get_dex_bound(bn.ref);
	return dex_call_member(db->memb, db->self, args, n);
}
static DexRef bound_str(const DexRef& dr) {
	DexBound* db = get_dex_bound(dr.ref);
	return str_make("<bound ", db->memb, ">");
}

void destroy_bound(DexObj* dobj) {
	get_dex_bound(dobj)->~DexBound();
}
DexType boundType = make_dex_type(&add_std_methods, &none_construct,
		&destroy_bound, &bound_str, "bound");

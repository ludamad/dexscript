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

#include "dexgen.h"
#include "../dexstr.h"
#include "../util/strmake.h"
#include "../util/hashutil.h"
#include <iostream>
using namespace std;

DexObj* create_gen(DexIterF* is, DexIterF* in, const char* name, size_t mem) {
	DexObj* nobj = alloc_dex_base(&genType, sizeof(DexObj) + sizeof(DexGen)
			+ mem * sizeof(size_t));
	DexGen* genobj = get_dex_gen(nobj);
	genobj->initf = is;
	genobj->iterf = in;
	genobj->name = name;
	return nobj;
}

static bool range_iter(DexRef& dr, const DexRef& cont, void* iter) {
	int* s = (int*) iter;
	++*s;
	DexGen* g = get_dex_gen(cont.ref);
	if (*s >= ((int*) g->memb_ptr())[1]) {
		return false;
	}
	dr = *s;
	return true;
}

static bool range_iter_init(DexRef& dr, const DexRef& cont, void* iter) {
	int* s = (int*) iter;
	*s = ((int*) get_dex_gen(cont.ref)->memb_ptr())[0] - 1;
	return range_iter(dr, cont, iter);
}

static bool gen_equal(const DexRef& g, const DexRef& g2) {
	size_t l = g->len;
	if (l != g2->len)
		return false;
	if (get_dex_gen(g.ref)->name != get_dex_gen(g2.ref)->name)
		return false;
	return memcmp(g.ref + 1, g2.ref + 1, l - sizeof(DexObj)) == 0;
}

size_t gen_hash(const DexRef& d) {
	DexGen* dg = get_dex_gen(d.ref);
	size_t l = d->len;
	size_t hash = size_t(dg->initf) ^ size_t(dg->iterf);
	size_t* end = (size_t*) (l + (char*) d.ref);
	size_t* iter = (size_t*) (d.ref + 1);
	return hash ^ ordered_hash(iter, end);
}

DexObj* create_range(int min, int max) {
	DexObj* ret = create_gen(&range_iter_init, &range_iter, "range", 2);
	DexGen* genobj = get_dex_gen(ret);
	((int*) genobj->memb_ptr())[0] = min;
	((int*) genobj->memb_ptr())[1] = max;
	return ret;

}
static DexRef strgen(const DexRef& g) {
	DexGen* dg = get_dex_gen(g.ref);
	return str_make("<iterable ", dg->name, ">");
}

DexType genType = make_dex_type(&add_std_methods, &none_construct,
		&default_destroy, &strgen, "iterable", &gen_equal, &gen_hash);

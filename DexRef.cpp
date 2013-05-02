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

#include "DexRef.h"
#include "types/dexnum.h"
#include "types/dexstr.h"
#include "types/dexstdops.h"
#include <cstring>
#include "parsing/op.h"

using namespace std;
void DexRef::operator=(const double& d) {
	if (ref->refc == 1 && ref->type == &numType) {
		get_dex_num(ref) = d;
	} else
		*this = create_number(d);
}

bool DexRef::operator<(const DexRef& o) const {
	if (o->type == &numType && ref->type == &numType)
		return get_dex_num(ref) < get_dex_num(o.ref);
	DexRef v = *this;
	dex_operation((char) opLT, v, o);
	return v.ref == dex_true;
}

void DexRef::set(const char* s, size_t n) {
	if (ref->refc > 1 || ref->type != &strType) {
		*this = create_string(s, n);
		return;
	}
	DexStr* ds = get_dex_str(ref);
	size_t maxn = (ref->len - sizeof(DexObj) - sizeof(DexStr));
	if (n >= maxn) {
		*this = create_string(s, n);
		return;
	}
	ds->strlen = n;
	memcpy(ds->data, s, n);
	ds->data[n] = '\0';
}

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

#include "dexnum.h"
#include "typetools.h"
#include "dextype.h"
#include "dexstr.h"
#include <cstdio>
#include "../objdef/dex_conv.h"

static DexRef create_numobj(DexType* dext, DexRef* dobjs, size_t len) {
	if (len != 1)
		return NONE;
	if (dobjs->ref->type == &strType) {
		DexStr* dd = get_dex_str(dobjs->ref);
		char* end;
		double val = strtod(dd->data, &end);
		if (end != dd->data + dd->strlen) {
			return NONE;
		}
		return create_number(val);
	}
	return NONE;
}
size_t num_hash(const double& val) {
	int s;
	if (is_int(val, s))
		return (size_t) s;
	size_t h = (size_t&) val;
	if (sizeof(size_t) * 2 == sizeof(double)) {
		h ^= ((size_t*) &val)[1];
	}
	return h;
}
size_t num_hash(const DexRef& d1) {
	double val = get_dex_num(d1.ref);
	return num_hash(val);
}
static bool numequal(const DexRef& d1, const DexRef& d2) {
	return get_dex_num(d1.ref) == get_dex_num(d2.ref);
}
#ifdef _MSC_VER
#define snprintf sprintf_s
#endif

size_t num_to_cstr(const double& val, char* buff){
	return snprintf(buff, 50, "%.16g", val, 1);
}
size_t int_to_cstr(int val, char* buff){
	return snprintf(buff, 50, "%d", val);
}
DexRef numstr(const DexRef& d) {
	char buff[50];
	size_t strlen = num_to_cstr(get_dex_num(d.ref), buff);
	return create_string(buff, strlen);
}
DexType numType = make_dex_type(&add_std_methods, &create_numobj,
		&default_destroy, &numstr, "num", &numequal, &num_hash);

DexObj* create_number(double d) {
	DexObj* ret = alloc_dex_base(&numType, sizeof(DexObj) + sizeof(double));
	get_dex_num(ret) = d;
	return ret;
}

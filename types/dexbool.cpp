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

#include "dexbool.h"
#include "typetools.h"
#include "dextype.h"
#include "../interp/error.h"
#include "../stdobjs/str.h"

inline static DexObj make_bool(bool b) {
	DexObj d;
	init_dex_base(&d, &boolType, size_t(b));
	dex_ref(&d);
	//len is unused for unallocated objects, we use it for the bool value here
	return d;
}
static DexObj dextrue = make_bool(true), dexfalse = make_bool(false);

static DexRef boolstr(const DexRef& d) {
	return dex_get_bool(d.ref) ? DISTR_TRUE : DISTR_FALSE;
}

static DexRef create_boolobj(DexType* dext, DexRef* dobjs, size_t n) {
	if (n > 1) {
		arg_amnt_err("bool", n, 0, 1);
		return NONE;
	}
	DexRef ret = &dexfalse;
	if (n && dex_is_true(dobjs[0])) {
		ret = &dextrue;
	}
	return ret;
}

//DexType boolType = make_dex_type(&boolstr, "bool");

DexObj* dex_true = &dextrue, *dex_false = &dexfalse;

DexType boolType = make_dex_type(&add_std_methods, &create_boolobj,
		&default_destroy, &boolstr, "bool", &dex_identity_equal,
		&dex_identity_hash);

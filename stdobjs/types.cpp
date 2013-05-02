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

#include "types.h"
#include "../objdef/dex_conv.h"
#include "../types/funcs/dexgen.h"
#include "../types/dexuobj.h"
#include "../interp/error.h"
#include "../mainh.h"
#include "struct.h"
#include "objdefutil.h"

static void put_t(DexRef& d, DexType* t) {
	dict_put(d, dex_intern(t->tname), (DexObj*) t);
}
static DexRef dex_type_of(DexRef* args, size_t n) {
	if (n != 1) {
		arg_amnt_err("typeof", n, 1);
		return NONE;
	}
	return (DexObj*) args->ref->type;
}
static DexRef dex_hasm(DexRef* args, size_t n) {
	if (n != 2) {
		arg_amnt_err("has_member", n, 1);
		return NONE;
	}
	if (args[1]->type != &strType) {
		arg_type_err("has_member", "str");
		return NONE;
	}
	//If the object is already interned this is fast
	DexRef strarg = dex_intern(args[1]);
	return dex_has_member(args[0], strarg) ? dex_true : dex_false;
}
static DexRef dex_getm(DexRef* args, size_t n) {
	static const char* NAME = "get_member";
	if (n != 2) {
		arg_amnt_err(NAME, n, 1);
		return NONE;
	}
	if (!dex_validate(args[1], &strType, NAME)) {
		return NONE;
	}
	//If the object is already interned this is fast
	DexRef strarg = dex_intern(args[1]);
	return dex_get_member(args[0], strarg);
}
static DexRef dex_callm(DexRef* args, size_t n) {
	static const char* NAME = "call_member";
	if (n < 2) {
		arg_amnt_err(NAME, n, 2);
		return NONE;
	}
	if (!dex_validate(args[1], &strType, NAME)) {
		return NONE;
	}
	//If the object is already interned this is fast
	DexRef strarg = dex_intern(args[1]);
	return dex_call_member(strarg, args[0], args + 2, n - 2);
}

DexRef rangef(DexRef* dr, size_t n) {
	static const char* NAME = "range";
	int mn, mx;
	if (n == 0)
		return NONE;
	if (dr->ref->type != &numType)
		return NONE;
	if (n == 1) {
		mn = 0;
		if (!dex_econv(&mx,dr[0], NAME))
			return NONE;
	} else if (n == 2) {
		if (dr[1]->type != &numType)
			return NONE;
		if (!dex_econv(&mn, dr[0], NAME))
			return NONE;
		if (!dex_econv(&mx, dr[1], NAME))
			return NONE;
	} else
		return NONE;
	return create_range(mn, mx);
}

DexRef gridf(DexRef* dr, size_t n) {
	static const char* NAME = "range";
	int mx, my;
	DexRef fill = DNUM_ZERO;

	if (n > 3)
		return NONE;
	if (n == 0)
		return EMPTY_LIST;

	dex_econv(&mx,dr[0], NAME);
	if (n > 1)
		dex_econv(&my, dr[1], NAME);
	else
		my = mx;

	if (n > 2)
		fill = dr[2];

	DexRef ret = create_list(mx), fillv = create_list(my);
	DexList* df = get_dex_list(fillv.ref);
	dex_manual_initv(df->data, fill, my);
	df->len = my;
	DexList* dl = get_dex_list(ret.ref);
	dl->len = mx;
	dex_manual_initv(dl->data, fillv,my);
	return ret;
}

DexRef make_type(DexRef* dr, size_t n) {
	if (n != 3 || dr[0]->type != &strType || dr[1]->type != &dictType
			|| dr[2]->type != &listType) {
		error(ARG_ERROR, "Expected name, method dict, member list.");
		return NONE;
	}
	dict_intern_keys(dr[1]);
	list_intern_all(dr[2]);
	return (DexObj*) create_utype(dr[0], dr[1], dr[2]);
}

void put_type_consts(DexRef& d) {
	put_t(d, &numType);
	put_t(d, &boolType);
	put_t(d, &strType);
	put_t(d, &dictType);
	put_t(d, &listType);
	put_t(d, &setType);
	put_t(d, &fileType);
	dex_put_builtin(d, create_builtin(&rangef, "range"));
	dex_put_builtin(d, create_builtin(&gridf, "grid"));
	dex_put_builtin(d, create_builtin(&dex_type_of, "typeof"));
	dex_put_builtin(d, create_builtin(&dex_hasm, "has_member"));
	dex_put_builtin(d, create_builtin(&dex_getm, "get_member"));
	dex_put_builtin(d, create_builtin(&dex_callm, "call_member"));
	dex_put_builtin(d, create_builtin(&make_type, "make_type"));
}

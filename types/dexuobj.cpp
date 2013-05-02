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

#include "dexuobj.h"
#include "dextype.h"
#include "dexdict.h"
#include "dexlist.h"
#include "funcs/dexfunc.h"
#include "funcs/dexbound.h"
#include "dextype.h"
#include "dexstdops.h"
#include "util/strmake.h"
#include "util/hashutil.h"
#include "../stdobjs/str.h"
#include "../stdobjs/struct.h"
#include "../stdobjs/math.h"
#include "../stdobjs/methods.h"
#include "../interp/error.h"
#include <new>

static void memb_err(const DexRef& memb) {
	error(MEMB_ERROR, str_make("Unidentified member '", memb, "' in object."));
}
DexRef uobj_copy(const DexRef& uobj) {
	return dex_copy(uobj.ref);
}
DexRef* uobj_pget(DexRef& uobj, const DexRef& memb) {
	if (uobj->refc > 1)
		uobj = uobj_copy(uobj);
	DexType* det = uobj->type;
	DexUObj* uo = get_dex_uobj(uobj.ref);
	void* ret = det->dmembMap.get((void*) memb.ref);
	if (ret) {
		//Adjust since ret returns 1 for memb 0
		DexRef* ptr = uo->membData + (size_t(ret) - 1);
		DexRef* obj_end = (DexRef*) (uobj->len + (char*) uobj.ref);
		if (ptr < obj_end)//Make sure our object has this member
			return ptr;
	}
	return dict_pget(uo->dataDict, memb);
}

DexRef uobj_get(const DexRef& uobj, const DexRef& memb) {
	DexType* det = uobj->type;
	DexUObj* uo = get_dex_uobj(uobj.ref);
	void* ret = det->dmembMap.get((void*) memb.ref);
	if (ret) {
		//Adjust since ret returns 1 for memb 0
		DexRef* ptr = uo->membData + (size_t(ret) - 1);
		DexRef* obj_end = (DexRef*) (uobj->len + (char*) uobj.ref);
		if (ptr < obj_end)//Make sure our object has this member
			return *ptr;
	}
	DexRef data = dict_get(uo->dataDict, memb);
	if (data.ref == NONE) {
		DexObj* method = lookup_user_method(det, memb);
		if (!method)
			return NONE;
		return create_bound(uobj, memb);
	}
	return data;
}
/*
 DexRef uobj_memb_call(const DexRef& memb, const DexRef& uobj, DexRef* args,
 size_t n) {
 DexType* utype = uobj->type;
 DexObj* method = lookup_user_method(utype, memb);
 if (!method) {
 DexRef mem = uobj_get(uobj, memb);
 return NONE;
 }
 return meth_call(method, uobj, args, n);
 }
 void uobj_memb_eqcall(const DexRef& memb, DexRef& uobj, DexRef* args, size_t n) {
 DexType* utype = uobj->type;
 DexObj* method = lookup_user_method(utype, memb);
 if (method)
 meth_eqcall(method, uobj, args, n);
 else
 memb_err(memb);
 }*/

static void baseclass_init(DexType* type, DexRef& ret, DexRef* args, size_t n){
	DexObj* base_type = type->base_type.ref;
	if (base_type != NONE)
		baseclass_init((DexType*)base_type, ret, args, n);
	meth_eqcall(type->uconstr, ret, args, n);
}
//The fact that we are passed the type is used here
DexRef create_uobj(DexType* utype, DexRef* args, size_t n) {
	bool noinit = utype->uconstr.ref == NONE;
	if (noinit && n != 0) {
		arg_amnt_err("init", n, 0, utype);
		return NONE;
	}
	size_t mems = utype->membn;
	//cout << "Mems: " << mems << endl;
	size_t objsize = sizeof(DexObj) + sizeof(DexUObj) + sizeof(DexRef) * mems;
	//cout << "Objsize: " << objsize << endl;

	DexObj* uobj = alloc_dex_base(utype, objsize);
	dex_init_clr(uobj);//Sets all members to NONE automagically
	DexUObj* duo = get_dex_uobj(uobj);
	duo->dataDict = EMPTY_DICT;
	DexRef ret = uobj;
	if (!noinit) {
		meth_eqcall(utype->uconstr, ret, args, n);
	}
	return ret;
}

static void destroy_uobj(DexObj* dobj) {
	DexUObj* duo = get_dex_uobj(dobj);
	DexType* dt = dobj->type;
	size_t n = dt->membn;
	for (size_t i = 0; i < n; i++)
		dex_unref(duo->membData[i].ref);
}

static bool uobj_call_or_ret(DexRef& ret, const DexRef& d,
		const DexRef& str, DexRef* args, size_t n) {
	DexType* det = d->type;
	DexUObj* duo = get_dex_uobj(d.ref);
	DexObj* method = lookup_user_method(det, str);
	if (!method) {
		DexRef m = dict_get(duo->dataDict, str);
		if (m.ref != NONE) {
			if (m->type == &methType)
				ret = meth_call(m, d, args, n);
			else
				ret = dex_call(m, args, n);
		} else
			return false;
	} else
		ret = meth_call(method, d, args, n);
	return true;
}

DexRef uobj_call(const DexRef& d, DexRef* args, size_t n){

	DexRef ret;
	if (uobj_call_or_ret(ret, d, DISTR_CALL, args, n))
		return ret;
	DexType* det = d->type;
	error(OP_ERROR, str_make("Undefined 'call' operator in object of type '",
			det->tname, "'."));
	return NONE;
}

int uobj_compare(const DexRef& u1, const DexRef& u2) {

	DexRef ret;
	if (uobj_call_or_ret(ret, u1, DISTR_CMP, (DexRef*) &u2, 1)) {
		if (ret->type != &numType) {
			error(OP_ERROR, str_make(
					"Non-numeric return in 'cmp' operator in object of type '",
					u1->type->tname, "'."));
			return 0;
		}
		double& d = get_dex_num(ret.ref);
		if (!d)
			return 0;
		if (d < 0)
			return -1;
		return 1;
	}
	DexType* det = u1->type;
	error(OP_ERROR, str_make("Undefined 'cmp' operator in object of type '",
			det->tname, "'."));
	return 0;
}

static DexRef uobjstr(const DexRef& d) {
	DexType* det = d->type;
	DexUObj* duo = get_dex_uobj(d.ref);
	//dex_print(duo->dataDict);
	DexRef ret;
	if (uobj_call_or_ret(ret, d, DISTR_STR, NULL, 0))
		return ret;
	return str_make("<'", det->tname, "' object>");
}
void add_uobj_methods(DexType* det) {
	add_std_methods(det);
}
static size_t utype_membvsize(size_t len) {
	return (len - sizeof(DexObj) - sizeof(DexUObj)) / sizeof(DexRef);
}
static size_t utype_membvn(const DexRef& dr) {
	return utype_membvsize(dr->len);
}
bool utype_equal(const DexRef& u1, const DexRef& u2) {
	DexUObj* duo1 = get_dex_uobj(u1.ref);
	DexUObj* duo2 = get_dex_uobj(u2.ref);
	if (!dict_equal(duo1->dataDict, duo2->dataDict))
		return false;
	size_t d1 = utype_membvn(u1), d2 = utype_membvn(u2);
	size_t len;
	if (d1 < d2) {
		len = d1;
		while (d1 < d2) {
			if (duo2->membData[d1].ref != NONE)
				return false;
			d1++;
		}
	} else {
		len = d2;
		while (d2 < d1) {
			if (duo1->membData[d2].ref != NONE)
				return false;
			d2++;
		}
	}
	for (int i = 0; i < len; i++) {
		if (!dex_equal(duo1->membData[i], duo2->membData[i]))
			return false;
	}
	return true;
}

bool uobj_incr_iter(DexRef& it, const DexRef& cont, void* iter) {

	DexType* det = cont->type;
	DexUObj* duo = get_dex_uobj(cont.ref);
	DexRef* ditr = (DexRef*) iter;

	//dex_print(duo->dataDict);
	DexObj* method = lookup_user_method(det, DISTR_INCR);
	if (!method) {
		DexRef m = dict_get(duo->dataDict, DISTR_INCR);
		if (m.ref != NONE) {
			if (m->type == &methType)
				it = meth_call(m, cont, ditr, 1);
			else
				it = dex_call(m, ditr, 1);
		} else {
			memb_err(DISTR_INCR);
			return false;
		}
	}
	it = meth_call(method, cont, ditr, 1);
	if (it.ref == NONE)
		return false;
	*ditr = it;
	return true;
}

size_t utype_hash(const DexRef& o) {
	DexType* utype = o->type;
	DexUObj* duo = get_dex_uobj(o.ref);
	DexRef* iM = duo->membData;
	size_t entryHash = dict_hash_entries(duo->dataDict);
	DexRef* endM = (DexRef*) (o->len + (char*) o.ref);
	size_t i = 0;
	DexList* names = get_dex_list(utype->mdexList.ref);
	while (iM < endM) {
		entryHash ^= hash_keyval(names->data[i], *iM);
		iM++;
		i++;
	}
	entryHash ^= 1927868237L * (dict_size(duo->dataDict) + i);
	return entryHash;
}
DexRef uobj_oget(const DexRef& d, const DexRef& k) {
	DexType* det = d->type;
	DexRef ret;
	if (uobj_call_or_ret(ret, d, DISTR_GET, (DexRef*) &k, 1))
		return ret;
	error(OP_ERROR, str_make("Undefined 'get' operator in object of type '",
			det->tname, "'."));
	return NONE;
}/*/
 void uobj_oput(DexRef& d, const DexRef& k, const DexRef& v){
 DexType* det = d->type;
 DexUObj* duo = get_dex_uobj(d.ref);
 //dex_print(duo->dataDict);
 DexRef ret;
 if (uobj_call_or_ret(ret, d, DISTR_STR, NULL, 0))
 return ret;
 return str_make("<'", det->tname, "' object>");
 }
 */
void add_meths(DexType* det, const DexRef& idict) {
	DexRef* s = dict_raw_array(idict) - 2;//One entry before
	while ((s = dict_next_entry(idict, s))) {
		add_method(det, s[0], s[1]);
	}
}
void add_membs(DexType* det, DexRef* args, size_t n) {
	while (n > 0) {
		add_member(det, args[0]);
		args++;
		n--;
	}
}
DexType* create_utype(const DexRef& name, const DexRef& methMap,
		const DexRef& membList) {
	DexType* t = (DexType*) malloc(sizeof(DexType));
	new (t) DexType(make_dex_type(&add_uobj_methods, &create_uobj,
			&destroy_uobj, &uobjstr, name, &utype_equal, &utype_hash));
	DexList* dl = get_dex_list(membList.ref);
	add_membs(t, dl->data, dl->len);
	add_meths(t, methMap);
	init_type(t);
	return t;
}

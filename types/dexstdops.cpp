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

#include "dexstdops.h"
#include "dexnumops.h"

#include "dextype.h"
#include "dexbool.h"
#include "dexstr.h"
#include "dexnum.h"
#include "dexdict.h"
#include "dexset.h"
#include "dexlist.h"
#include "dexuobj.h"

#include "funcs/dexbuiltin.h"
#include "funcs/dexgen.h"
#include "funcs/dexfunc.h"

#include "../interp/error.h"
#include "util/strmake.h"
#include "../stdobjs/str.h"
#include "../parsing/op.h"

#include <iostream>
#include <new>

ostream &operator<<(ostream &o, const DexRef& val) {
	DexRef str = dex_to_string(val);
	o << str_get_cstr(str.ref);
	return o;
}
void dex_print(const DexRef& val) {
	cout << val;
}
int dex_compare(const DexRef& a, const DexRef& b) {
	DexType* type = a->type, *ot = b->type;
	if (ot != type) {
		DexRef err_string = str_make("Incomparable types '", type->tname,
				"' and '");
		str_concat(err_string, ot->tname);
		strc_concat(err_string, "'.", 2);
		error(OP_ERROR, err_string);
		return 0;
	}
	int cmp;
	if (type == &strType) {
		cmp = strcmp(str_get_cstr(a.ref), str_get_cstr(b.ref));
	} else if (type == &numType) {
		double ad = get_dex_num(a.ref);
		double bd = get_dex_num(b.ref);
		if (ad < bd)
			cmp = -1;
		else if (ad == bd)
			cmp = 0;
		else
			cmp = 1;
	} else {
		return uobj_compare(a, b);
	}
	return cmp;
}

void dex_str_print(const DexRef& val) {
	DexRef str = dex_to_string(val);
	if (error()) {
		stack_trace("dex_str_print", str_make("dexstdops.cpp"));
	} else {
		if (str.ref == val.ref)
			cout << '"';
		cout << str_get_cstr(str.ref);
		if (str.ref == val.ref)
			cout << '"';
	}
}
bool dex_equal(const DexRef& d1, const DexRef& d2) {
	if (d1.ref == d2.ref)
		return true;
	DexType* type = d1->type;
	if (type != d2->type)
		return false;
	return type->equality(d1, d2);
}
size_t dex_hash(const DexRef& d1) {
	return d1->type->hashfunc(d1);
}

bool dex_is_seq(const DexRef& v) {
	DexType* type = v->type;
	return type == &listType || type == &setType || type == &dictType || type
			== &genType;
}
void dex_set_subscr(DexRef& d1, const DexRef& k, const DexRef& v) {
	DexType* type = d1->type;
	if (type == &dictType) {
		dict_put(d1, k, v);
	} else if (type == &listType) {
		int index;
		if (!valid_list_key(k, index)) {
			error(DOMAIN_ERROR, "Expected positive index.");
			return;
		}
		list_set(d1, (size_t) index, v);
	} else {
		error(OP_ERROR, "Subscript error.");
	}
}
DexRef dex_get_subscr(const DexRef& d1, const DexRef& k) {
	DexType* type = d1->type;
	if (type == &dictType) {
		return dict_get(d1, k);
	} else if (type == &listType) {
		int index;
		if (!valid_list_key(k, index)) {
			error(DOMAIN_ERROR, "Expected positive index.");
			return NONE;
		}
		return list_get(d1, (size_t) index);
	} else if (is_utype(type)) {
		return uobj_oget(d1, k);
	}
	error(OP_ERROR, "Subscript error.");
	return NONE;
}
DexRef* dex_pget(DexRef& d1, const DexRef& k) {
	DexType* type = d1->type;
	if (type == &dictType) {
		return dict_pget(d1, k);
	} else if (type == &listType) {
		int index;
		if (!valid_list_key(k, index)) {
			error(DOMAIN_ERROR, "Expected positive index.");
			return NULL;
		}
		return list_pget(d1, (size_t) index);
	}
	error(OP_ERROR, "Subscript error.");
	return NULL;
}
bool dex_init_iter(DexRef& it, const DexRef& cont, void* iter) {
	DexType* det = cont->type;
	if (det == &listType || det == &dictType || det == &setType) {
		size_t* s = (size_t*) iter;
		*s = (size_t) -1;
		return dex_incr_iter(it, cont, iter);
	} else if (cont->type == &genType) {
		return gen_init_iter(it, cont, iter);
	} else if (is_utype(it->type)) {
		new ((DexRef*) iter) DexRef();
		return uobj_incr_iter(it, cont, iter);
	}/*else if (cont->type == &funcType) {
	 DexRef val = dex_call(cont, (DexRef*) &NONE, 1);
	 if (val.ref == NONE)
	 return false;
	 new ((DexRef*) iter) DexRef(val);
	 }*/
	error(OP_ERROR, str_make("Object of '", cont->type->tname,
			"' is not iterable"));
	return false;
}
bool dex_incr_iter(DexRef& it, const DexRef& cont, void* iter) {
	DexType* det = cont->type;
	if (det == &listType) {
		size_t* s = (size_t*) iter;
		++*s;
		if (*s == list_size(cont)) {
			return false;
		}
		it = get_dex_list(cont.ref)->data[*s];
		return true;
	} else if (det == &dictType) {
		return dict_incr_iter(it, cont, (size_t*) iter);
	} else if (det == &setType) {
		return set_incr_iter(it, cont, (size_t*) iter);
	} else if (det == &genType) {
		return gen_incr_iter(it, cont, iter);
	} else if (det == &funcType) {
		DexRef val = dex_call(cont, (DexRef*) iter, 1);
		if (val.ref == NONE) {
			((DexRef*) iter)->~DexRef();
			return false;
		}
	} else if (is_utype(det)) {
		return uobj_incr_iter(it, cont, iter);
	}
	return false;
}

DexRef dex_to_string(const DexRef& d1) {
	DexRef ret = d1;
	while (ret->type != &strType) {
		ret = ret->type->tostr(ret);//Unlikely to chain far
	}

	return ret;
}

bool dex_identity_equal(const DexRef& d1, const DexRef& d2) {
	return d1.ref == d2.ref;//Returns if addresses equal
}
size_t dex_identity_hash(const DexRef& d1) {
	return size_t(d1.ref);//Returns address
}
bool dex_is_true(const DexRef& d1) {
	if (d1.ref == dex_false || d1.ref == NONE)
		return false;
	if (d1.ref == dex_true)
		return true;
	DexType* det = d1->type;
	if (det == &numType)
		return get_dex_num(d1.ref) != 0;
	return true;
}

static void dex_list_arithmetic_oper(char opcode, DexRef& dst, const DexRef& o) {
	DexList* dl1 = get_dex_list(dst.ref);
	DexList* dl2 = get_dex_list(o.ref);
	int len = dl1->len;
	if (len != dl2->len) {
		error(OP_ERROR,
				"Lists of equal length are required for arithmetic operations.");
		dst = NONE;
		return;
	}
	if (dst->refc > 1) {
		dst = create_list(dl1->data, len, len);
		dl1 = get_dex_list(dst.ref);
	}
	for (int i = 0; i < len; i++) {
		if (dl1->data[i]->type == &listType)
			dex_list_arithmetic_oper(opcode, dl1->data[i], dl2->data[i]);
		else
			dex_operation(opcode, dl1->data[i], dl2->data[i]);
	}
}

static inline bool dexmeth_eqcall(DexRef& self, const DexRef& str,
		DexRef* args, size_t n) {
	DexObj* method = lookup_user_method(self->type, str);
	if (method)
		meth_eqcall(method, self, args, n);
	return method != NULL;
}
static inline bool cmeth_eqcall(DexRef& self, const DexRef& str, DexRef* args,
		size_t n) {
	DexBnM* method = lookup_method(self->type, str);
	if (method)
		method(self, args, n);
	return method != NULL;
}

static inline void dex_operation_by_name(const DexRef& std_name, DexRef& dst,
		const DexRef& o) {
	if (!dexmeth_eqcall(dst, std_name, (DexRef*) &o, 1) && !cmeth_eqcall(dst,
			std_name, (DexRef*) &o, 1)) {
		error(OP_ERROR, str_make("Operator '", std_name,
				"' not defined for object."));
	}
}

DexRef dex_call(const DexRef& d1, DexRef* args, size_t n) {
	if (error()) {
		return NONE;
	}
	DexType* det = d1->type;
	if (det == &builtinType) {
		return builtin_call(d1, args, n);
	} else if (det == &funcType) {
		return func_call(d1, args, n);
	} else if (det == &typeType) {
		return create_dex_object((DexType*) d1.ref, args, n);
	} else if (det == &boundType) {
		return bound_call(d1, args, n);
	} else if (is_utype(det)) {
		return uobj_call(d1, args, n);
	} else {
		DexRef obj = d1;
		DexBnM* method = lookup_method(d1->type, DISTR_CALL);
		if (method) {
			(*method)(obj, args, n);
		} else {
			DexObj* omethod = lookup_user_method(d1->type, DISTR_CALL);
			if (omethod)
				meth_eqcall(omethod, obj, args, n);
		}
		return obj;
	}
	error(OP_ERROR,
			str_make("Error calling object of type '", det->tname, "'."));
	return NONE;
}
static void num_operation(char opcode, DexRef& dst, const DexRef& o) {
	switch (opcode) {
	case opLT: {
		dst = get_dex_num(dst.ref) < get_dex_num(o.ref) ? dex_true : dex_false;
		break;
	}
	case opGT: {
		dst = get_dex_num(dst.ref) > get_dex_num(o.ref) ? dex_true : dex_false;
		break;
	}
	case opEQL: {
		dst = get_dex_num(dst.ref) <= get_dex_num(o.ref) ? dex_true : dex_false;
		break;
	}
	case opEQG: {
		dst = get_dex_num(dst.ref) >= get_dex_num(o.ref) ? dex_true : dex_false;
		break;
	}
	case opADD: {
		num_add(dst, o);
		break;
	}
	case opSUB: {
		num_sub(dst, o);
		break;
	}
	case opMUL: {
		num_mul(dst, o);
		break;
	}
	case opMOD: {
		num_mod(dst, o);
		break;
	}
	case opDIV: {
		num_div(dst, o);
		break;
	}
	case opPOW: {
		num_pow(dst, o);
		break;
	}
	case opBITOR: {
		num_bitor(dst, o);
		break;
	}
	case opBITXOR: {
		num_bitxor(dst, o);
		break;
	}
	case opBITAND: {
		num_bitand(dst, o);
		break;
	}
	case opBITSDOWN: {
		num_bitsdown(dst, o);
		break;
	}
	case opBITSUP: {
		num_bitsup(dst, o);
		break;
	}
	default: {
		error(OP_ERROR, "Operator not defined for numeric types.");
		dst = NONE;
	}
	}
}
void dex_operation(char opcode, DexRef& dst, const DexRef& o) {

	int cmp;
	OpType ot = OpType(opcode);
	if (opcode == opEQU) {
		dst = dex_equal(dst, o) ? dex_true : dex_false;
		return;
	}
	if (opcode == opNEQ) {
		dst = dex_equal(dst, o) ? dex_false : dex_true;
		return;
	}

	DexType* type = dst->type, *otype = o->type;

	if (type == &numType) {
		if (otype != &numType)
			error(OP_ERROR, "Incompatible types in operator.");
		num_operation(opcode, dst, o);
		return;
	}

	switch (ot) {

	case opLT: {
		cmp = dex_compare(dst, o);
		dst = cmp < 0 ? dex_true : dex_false;
		break;
	}
	case opGT: {
		cmp = dex_compare(dst, o);
		dst = cmp > 0 ? dex_true : dex_false;
		break;
	}
	case opEQL: {
		cmp = dex_compare(dst, o);
		dst = cmp <= 0 ? dex_true : dex_false;
		break;
	}
	case opEQG: {
		cmp = dex_compare(dst, o);
		dst = cmp >= 0 ? dex_true : dex_false;
		break;
	}
	case opADD: {
		if (type == &listType)
			dex_list_arithmetic_oper(opcode, dst, o);
		else
			dex_operation_by_name(DISTR_ADD, dst, o);
		break;
	}
	case opSUB: {
		if (type == &listType)
			dex_list_arithmetic_oper(opcode, dst, o);
		else
			dex_operation_by_name(DISTR_SUBTRACT, dst, o);
		break;
	}
	case opMUL: {
		if (type == &listType)
			dex_list_arithmetic_oper(opcode, dst, o);
		else
			dex_operation_by_name(DISTR_MULTIPLY, dst, o);
		break;
	}
	case opMOD: {
		if (type == &listType)
			dex_list_arithmetic_oper(opcode, dst, o);
		else
			dex_operation_by_name(DISTR_MODULOS, dst, o);
		break;
	}
	case opDIV: {
		if (type == &listType)
			dex_list_arithmetic_oper(opcode, dst, o);
		else
			dex_operation_by_name(DISTR_DIVIDE, dst, o);
		break;
	}
	case opPOW: {
		if (type == &listType)
			dex_list_arithmetic_oper(opcode, dst, o);
		else
			dex_operation_by_name(DISTR_POWER, dst, o);
		break;
	}
	case opCAT: {
		if (type == &strType)
			str_concat(dst, o);
		else if (type == &listType)
			list_concat(dst, o);
		else if (type == &dictType)
			dict_merge(dst, o);
		else if (type == &setType)
			set_merge(dst, o);
		else
			dex_operation_by_name(DISTR_CONCAT, dst, o);
		break;
	}
	default:
		error(OP_ERROR, "Operator not intended for this type");
		break;
	}
}
DexRef dex_identity_func(const DexRef& d1) {
	return d1;
}
DexRef dex_get_member(const DexRef & obj, const DexRef & str) {
	DexType* type = obj->type;
	DexRef ret;
	if (is_utype(type)) {
		ret = uobj_get(obj, str);
	} else if (type == &dictType) {
		ret = dict_get(obj, str);

	} else if (type == &moduleType) {
		ret = mod_get(obj, str);
	} else if (type == &funcType) {
		if (str.ref == DISTR_CALL.ref)
			return obj;
		if (str.ref == DISTR_NAME.ref)
			return get_dex_func(obj.ref)->name;
	} else {
		void* getter = type->cmembMap.get((void*)str.ref);
		if (getter)
			return (*(DexStrF*)getter)(obj);
	}
	if (ret.ref == NONE) {
		if (has_method(type, str)) {
			return create_bound(obj, str);
		} else
			error(MEMB_ERROR, str_make("Member error retrieving '", str_make(
					type->tname, ".", str), "'."));
	}
	return ret;
}
bool dex_has_member(const DexRef& obj, const DexRef& str) {

	DexType* type = obj->type;
	if (is_utype(type)) {
		if (uobj_get(obj, str).ref != NONE)
			return true;
	} else if (type == &dictType) {
		if (dict_get(obj, str).ref != NONE)
			return true;

	} else if (type == &moduleType) {
		if (mod_get(obj, str).ref != NONE)
			return true;
	} else if (type == &funcType) {
		if (str.ref == DISTR_CALL.ref || str.ref == DISTR_NAME.ref)
			return true;
	}
	if (has_method(type, str))
		return true;
	return false;
}
DexRef* dex_pget_member(DexRef & obj, const DexRef & str) {
	DexType* type = obj->type;
	DexRef ret;
	if (is_utype(type)) {
		return uobj_pget(obj, str);
	} else if (type == &dictType) {
		return dict_pget(obj, str);

	}/* else if (type == &moduleType) {
	 return mod_pget(obj, str);
	 }*/
	error(MEMB_ERROR, str_make("Member error updating '", str_make(type->tname,
			".", str), "'."));
	return NULL;
}

void dex_set_member(DexRef & obj, const DexRef & str, const DexRef & val) {
	DexType* type = obj->type;
	if (is_utype(type)) {
		uobj_pget(obj, str)[0] = val;
	} else if (type == &dictType) {
		dict_put(obj, str, val);
	} /*else if (type == &moduleType) {
	 DexModule* mod = get_dex_mod(obj.ref);
	 dict_put(mod->data, str, val);
	 }*/else {
		error(MEMB_ERROR, str_make("Member error setting '", str_make(
				type->tname, ".", str), "'."));
	}
}

void log_method(const DexRef& memb, const DexRef& obj) {
	DexRef& tname = obj->type->tname;
	stack_trace(str_make(tname, ".", memb), str_make("dex", tname, ".cpp"));
}
void dex_call_memb_equ(const DexRef& memb, DexRef& obj, DexRef* args, size_t n) {
	if (error()) {
		obj = NONE;
		return;
	}
	DexBnM* meth = lookup_method(obj->type, memb);
	if (!meth) {
		DexObj* method = lookup_user_method(obj->type, memb);
		if (method) {
			meth_eqcall(method, obj, args, n);
			return;
		} else {
			DexRef m = dex_get_member(obj, memb);
			if (m.ref != NONE) {
				if (m->type == &methType)
					meth_eqcall(m, obj, args, n);
				else
					obj = dex_call(m, args, n);
				return;
			}
			error(MEMB_ERROR, str_make("Member error retrieving '", str_make(
					obj->type->tname, ".", memb), "'."));
		}
	} else {
		(*meth)(obj, args, n);
		if (error()) {
			log_method(memb, obj);
		}
	}
}
DexRef dex_call_member(const DexRef& memb, const DexRef& obj, DexRef* args,
		size_t n) {
	if (error())
		return NONE;
	DexType* det = obj->type;
	DexBnM* meth = lookup_method(det, memb);
	if (!meth) {
		DexObj* method = lookup_user_method(det, memb);
		if (method) {
			return meth_call(method, obj, args, n);
		} else {
			DexRef m = dex_get_member(obj, memb);
			if (m.ref != NONE) {
				if (m->type == &methType)
					return meth_call(m, obj, args, n);
				else
					return dex_call(m, args, n);
			} else {
				error(MEMB_ERROR, str_make("Member error retrieving '",
						str_make(det->tname, ".", memb), "'."));
				return NONE;
			}
		}
	}

	DexRef ret = obj;
	(*meth)(ret, args, n);
	if (error()) {
		log_method(memb, obj);
		return NONE;
	}

	return ret;
}


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

#include "dexfunc.h"
#include "../dexdict.h"
#include "../dexstr.h"
#include "../../stdobjs/str.h"
#include "../util/strmake.h"
#include "../../interp/error.h"
#include <new>
#include <iostream>
using namespace std;

ASTParam::~ASTParam() {
	delete opt;
}

DexFunc::~DexFunc() {
	for (size_t i = 0; i < body.size(); i++) {
		delete body[i];
	}
	delete[] params;
}
DexObj* create_func() {
	DexObj* ret = alloc_dex_base(&funcType, sizeof(DexObj) + sizeof(DexFunc));
	DexFunc* df = get_dex_func(ret);
	new (df) DexFunc();
	return ret;
}
void DexFunc::print(int indent) {
	int i = indent;
	while (i--)
		cout << '\t';

	cout << "func ";
	cout << get_dex_str(name.ref)->data;
	cout << '(';
	for (size_t i = 0; i < maxargs; i++) {
		cout << get_dex_str(params[i].name.ref)->data;
		if (params[i].opt) {
			cout << " = ";
			params[i].opt->print(indent);
		}

		if (i != maxargs - 1)
			cout << ", ";
	}
	cout << "):\n";
	size_t s = body.size();
	for (size_t i = 0; i < s; i++) {
		body[i]->print(indent + 1);
		cout << '\n';
	}
}

static void arg_amnt_err(const DexRef& fn, size_t n, DexType* callt = NULL) {
	DexFunc* df = get_dex_func(fn.ref);
	arg_amnt_err(str_get_cstr(df->name), n, df->minargs, df->maxargs, callt);
}

DexRef meth_call(const DexRef& fn, const DexRef& obj, DexRef* args, size_t n) {
	DexFunc* df = get_dex_func(fn.ref);
	if (n < df->minargs || n > df->maxargs) {
		arg_amnt_err(fn, n, obj->type);
		return NONE;
	}

	int space_needed = df->varmap.size();
	DexRef self = obj;
	DexRef* varv = dex_realloca(args, n, space_needed);
	VarContext varc(df, varv, &self);
	for (int i = n; i < df->maxargs; i++) {
		varv[i] = df->params[i].opt->eval(varc);
	}

	//Store the old module
	DexRef lastmod = dex_state.module;
	dex_state.module = df->module;
	DexRef val = handle_statements(varc, df->body, false);
	//And restore it
	dex_state.module = lastmod;

	if (varv != args)
		dex_dealloca(varv, space_needed);
	else
		dex_dealloca(varv+n, space_needed-n);

	if (error()) {
		stack_trace(str_make(obj->type->tname, ".", df->name), df->fname);
		return NONE;
	}

	if (val.ref == NONE || val->type != &noneType)
		return val;
	return self;
}
void meth_eqcall(const DexRef& fn, DexRef& obj, DexRef* args, size_t n) {
	DexFunc* df = get_dex_func(fn.ref);
	if (n < df->minargs || n > df->maxargs) {
		arg_amnt_err(fn, n, obj->type);
		obj = NONE;
		return;
	}

	int space_needed = df->varmap.size();
	DexRef* varv = dex_realloca(args, n, space_needed);
	VarContext varc(df, varv, &obj);
	for (int i = n; i < df->maxargs; i++) {
		varv[i] = df->params[i].opt->eval(varc);
	}

	//Store the old module
	DexRef lastmod = dex_state.module;
	dex_state.module = df->module;
	DexRef val = handle_statements(varc, df->body, false);
	//And restore it
	dex_state.module = lastmod;

	if (varv != args)
		dex_dealloca(varv, space_needed);
	else
		dex_dealloca(varv+n, space_needed-n);

	if (error()) {
		stack_trace(str_make(obj->type->tname, ".", df->name), df->fname);
		return;
	}

	if (val.ref == NONE || val->type != &noneType)
		obj = val;

}
DexRef func_call(const DexRef& fn, DexRef* args, size_t n) {
	DexFunc* df = get_dex_func(fn.ref);
	if (n < df->minargs || n > df->maxargs) {
		arg_amnt_err(fn, n);
		return NONE;
	}

	int space_needed = df->varmap.size();
	DexRef* varv = dex_realloca(args, n, space_needed);
	VarContext varc(df, varv);
	for (int i = n; i < df->maxargs; i++) {
		varv[i] = df->params[i].opt->eval(varc);
	}

	//Store the old module
	DexRef lastmod = dex_state.module;
	dex_state.module = df->module;
	DexRef val = handle_statements(varc, df->body, false);
	//And restore it
	dex_state.module = lastmod;
	if (varv != args)
		dex_dealloca(varv, space_needed);
	else
		dex_dealloca(varv+n, space_needed-n);

	if (error()) {
		stack_trace(df->name, df->fname);
		return NONE;
	}

	return val;
}

static DexRef func_str(const DexRef& dr) {
	DexRef name = get_dex_func(dr.ref)->name;
	return str_make("<function ", name, ">");
}

static DexRef meth_str(const DexRef& dr) {
	DexRef name = get_dex_func(dr.ref)->name;
	return str_make("<method ", name, ">");
}
void destroy_func(DexObj* dobj) {
	get_dex_func(dobj)->~DexFunc();
}
DexType funcType = make_dex_type(&add_std_methods, &none_construct,
		&destroy_func, &func_str, "function");

DexType methType = make_dex_type(&add_std_methods, &none_construct,
		&destroy_func, &meth_str, "method");

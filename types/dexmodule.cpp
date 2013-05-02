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

#include "dexmodule.h"
#include "dexstr.h"
#include "dexdict.h"
#include "typetools.h"
#include <iostream>
#include "../interp/interp_state.h"
#include "util/strmake.h"
#include <new>

DexObj* create_module(const char* name){
	DexObj* ret = alloc_dex_base(&moduleType, sizeof(DexObj)
			+ sizeof(DexModule));
	DexModule* dd = get_dex_mod(ret);
	new (dd) DexModule();
	dd->fname = str_make("shell");

	return ret;
}

DexRef* mod_ind_pget(const DexRef& mod, int index){
	DexModule* m = get_dex_mod(mod.ref);
	return m->vars.get_var(index);
}
DexObj* create_module(const DexRef& path) {
	DexObj* ret = alloc_dex_base(&moduleType, sizeof(DexObj)
			+ sizeof(DexModule));
	DexModule* dd = get_dex_mod(ret);
	new (dd) DexModule();
	//Keep the module alive while we evaluate
	dex_ref(ret);

	DexRef last_mod = dex_state.module;
	dex_state.module = ret;
	dd->vars.merge_dict(dex_state.langconsts);
	dd->fname = path;

	compile_file(dd->ast, str_get_cstr(path));
	if (error()) {
		handle_error();
		dex_state.module = last_mod;
		destroy_dex_object(ret);
		return NONE;
	}
	{
		DexRef mod = ret;
		VarContext varc(mod);
		dd->ast.partial_eval(varc,false,true);
	}
	if (error()) {
		handle_error();
	}
	dex_state.module = last_mod;

	//Remove the ref count surplus
	dex_safe_unref(ret);

	return ret;
}

DexRef mod_get(const DexRef& mod, const DexRef& str){
	DexModule* m = get_dex_mod(mod.ref);
	return m->vars.get_var(str);
}

DexRef* mod_pget(const DexRef& mod, const DexRef& str){
	DexModule* m = get_dex_mod(mod.ref);
	return m->vars.pget_var(str);
}
void mod_put(const DexRef& mod, const DexRef& str, const DexRef& val){
	DexModule* m = get_dex_mod(mod.ref);
	*m->vars.pget_var(str) = val;
}
void mod_put_builtin(const DexRef& mod, const DexRef& built){
	DexBuiltin* db = get_dex_builtin(built.ref);
	DexRef n = dex_intern(db->name);
	mod_put(mod, n, built);
}

static void destroy_moduleobj(DexObj* d) {
	get_dex_mod(d)->~DexModule();
}
static DexRef modulestr(const DexRef& d) {
	DexModule* mod = get_dex_mod(d.ref);
	return str_make("<module ",mod->fname,">");
}
static DexRef module_constr(DexType* dext, DexRef* args, size_t n) {
	if (n != 1) {
		arg_err();
		return NONE;
	}
	return create_module(args[0]);
}

DexType moduleType = make_dex_type(&add_std_methods, module_constr,
		&destroy_moduleobj, &modulestr, "module");

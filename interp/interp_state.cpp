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

#include "interp_state.h"
#include "error.h"
#include "objstack.h"
#include "global.h"
#include "../types/dexdict.h"
#include "../types/dexlist.h"
#include "../types/dexset.h"
#include "../types/dexstr.h"
#include "../types/dexmodule.h"
#include "../stdobjs/consts.h"
#include "../stdobjs/struct.h"
#include "../types/init.h"
#include "../types/dexstdops.h"
#include "../parsing/keywords.h"
#include "path.h"
#include <new>
#include <iostream>
const char DEX_STACK_ERR[] = "DexScript stack ran out of space!";

//Do not initialize
InterpState::InterpState() :
	langconsts(DEX_DONT_INIT), module(DEX_DONT_INIT), contextList(DEX_DONT_INIT),
			loadedModules(DEX_DONT_INIT), internalModules(DEX_DONT_INIT), internObjs(
					DEX_DONT_INIT) {
	optlevel = 0;
}

//Relies on bypassing initialization later
void preinit_interp_state() {
	//Requisite for all further code
	preinit_none();

	dex_manual_init(&dex_state.internObjs, create_set(128));
	dex_manual_init(&dex_state.langconsts, create_dict(64));
	dex_manual_init(&dex_state.module, create_module());
	dex_manual_init(&dex_state.loadedModules, create_dict(64));
	dex_manual_init(&dex_state.internalModules, create_dict(64));
	dict_put(dex_state.loadedModules, create_string("__INTER"),
			dex_state.module);
	dex_manual_init(&dex_state.contextList, create_list(16));
	preinit_struct_consts();
}

InterpState::~InterpState() {
	size_t i = -1;
	DexRef ret;
	//Loop through set elements conveniently
	//Strip the intern ref
	while (set_incr_iter(ret, dex_state.internObjs, &i)) {
		ret->refc ^= INTERN_REF;
	}
	free(stack);
}

InterpState dex_state;
bool error(err_t type, const char* c) {
	if (!type || !error()) {
		error() = type;
		dex_state.err.data = c;
		dex_state.err.data2 = NONE;
		dex_state.err.stack.clear();
		return true;
	}
	return false;
}
bool handle_error() {
	if (error()) {
		dex_state.err.print_error();
		error(NO_ERROR, NULL);
		return true;
	}
	return false;
}

bool error(err_t type, const DexRef& err) {
	if (!type || !error()) {
		error() = type;
		dex_state.err.data = NULL;
		dex_state.err.data2 = err;
		dex_state.err.stack.clear();
		return true;
	}
	return false;
}

void stack_trace(const StackFrame& sf) {
	dex_state.err.stack.push_back(sf);
}

void stack_trace(const char* c, const DexRef& f) {
	stack_trace(StackFrame(create_string(c), f));
}

void stack_trace(const DexRef& c, const DexRef& f) {
	stack_trace(StackFrame(c, f));
}

void DexErrState::print_error() {
	if (error) {
		if (error == LEX_ERROR)
			cout << "Lex Error" << endl;
		else if (error == PARSE_ERROR)
			cout << "Parse Error" << endl;
		else if (error == OP_ERROR)
			cout << "Operator Error" << endl;
		else if (error == DOMAIN_ERROR)
			cout << "Domain Error" << endl;
		else if (error == ARG_ERROR)
			cout << "Argument Error" << endl;
		else if (error == MEMB_ERROR)
			cout << "Member Error" << endl;
		else if (error == OUT_OF_MEMORY)
			cout << "Out of Memory Error" << endl;
		else if (error == DEXOBJ_ERROR)
			cout << "Object as Error" << endl;
		else if (error == IO_ERROR)
			cout << "IO Error" << endl;
		if (data)
			cout << data << endl;
		else {
			dex_print(data2);
			cout << endl;
		}
		for (size_t i = 0; i < stack.size(); i++) {
			cout << "... from '";
			dex_print(stack[i].src_func);
			cout << "' in file \"";
			dex_print(stack[i].src_file);
			cout << "\"" << endl;
		}
	}
}
DexErrState::DexErrState() {
	error = NO_ERROR;
	data = NULL;
	stack.reserve(255);
}

void dex_init_interpreter() {
	init_string_consts();
	put_all_consts(dex_state.langconsts);
	get_dex_mod(dex_state.module.ref)->vars.merge_dict(dex_state.langconsts);
	init_std_types();
	init_keywords();
	init_path();
	dex_state.stack = make_obj_stack(STACKOBJ_LIM);
	dex_state.stackptr = dex_state.stack;
	dex_state.stackend = dex_state.stack + STACKOBJ_LIM;
}

DexRef* dex_alloca(int objn) {
	DexRef* ret = dex_state.stackptr;
	DexRef* end = ret + objn;

	if (end > dex_state.stackend) {
		error(STACK_OVERFLOW, DEX_STACK_ERR);
	}
	dex_manual_initv(ret,objn);
	dex_state.stackptr = end;

	return ret;
}
DexRef* dex_raw_alloca(int objn) {
	DexRef* ret = dex_state.stackptr;
	DexRef* end = ret + objn;

	if (end > dex_state.stackend) {
		error(STACK_OVERFLOW, DEX_STACK_ERR);
	}
	dex_state.stackptr = end;

	return ret;
}


//Attempts to add more space on the stack...
//- if 'objv' was allocated on the stack with 'objn' objects
//then 'objv' is returned and there is space is made for
//'newobjn' objects
//Otherwise the space is allocated on stack as normal, with
//the objects in 'objv' copied over
//Note this has initialization caveats as above
DexRef* dex_raw_realloca(DexRef* objv, int objn, int newobjn){
	DexRef* ret, *end;
	//Objv was part of the stack...
	if (dex_state.stackptr == objv + objn){
		ret = objv;
	} else {
		//Might be any arbitrary pointer
		//Copy over elements to stack
		ret = dex_state.stackptr;
		dex_manual_initv(ret,objv,objn);
	}
	end = ret + newobjn;
	if (end > dex_state.stackend) {
		error(STACK_OVERFLOW, DEX_STACK_ERR);
	}
	dex_state.stackptr = end;
	return ret;
}
//Attempts to add more space on the stack...
//- if 'objv' was allocated on the stack with 'objn' objects
//then 'objv' is returned and there is space is made for
//'newobjn' objects
//Otherwise the space is allocated on stack as normal, with
//the objects in 'objv' copied over
DexRef* dex_realloca(DexRef* objv, int objn, int newobjn){
	DexRef* ret, *end;
	//Objv was part of the stack...
	if (dex_state.stackptr == objv + objn){
		ret = objv;
	} else {
		//Might be any arbitrary pointer
		//Copy over elements to stack
		ret = dex_state.stackptr;
		dex_manual_initv(ret,objv,objn);

	}
	dex_manual_initv(ret + objn, newobjn - objn);
	end = ret + newobjn;
	if (end > dex_state.stackend) {
		error(STACK_OVERFLOW, DEX_STACK_ERR);
	}
	dex_state.stackptr = end;
	return ret;
}
void dex_dealloca(DexRef* objv, int objn) {
	DexRef* iter = objv;
	DexRef* end = iter + objn;
	while (iter < end){
		dex_unref(iter->ref);
		iter++;
	}
	dex_state.stackptr -= objn;
}

void set_module(const DexRef& mod) {
	dex_state.module = mod;
}

void add_internal_module(const DexRef& name, const DexRef& mod){
	dict_put(dex_state.internalModules, name, mod);
}
DexRef load_module(const DexRef& name, const DexRef& modulePath) {
	if (name.ref != NONE) {
		DexRef val = dict_get(dex_state.internalModules, name);
		if (val.ref != NONE)
			return val;
	}
	DexRef rel_path = path_from_abs(modulePath);
	DexRef* dr = dict_pget(dex_state.loadedModules, rel_path);
	if (dr->ref == NONE) {
		*dr = create_module(rel_path);
		//Dr will be NONE if an error has occured
		if (dr->ref != NONE){
			VarContext varc(*dr);
			//cout << "Path: " <<  rel_path << endl;
			get_dex_mod(dr->ref)->ast.eval(varc);
		}
	}
	return *dr;
}

int InitGuard::refc = 0;

InitGuard::InitGuard() {
	if (++refc <= 1) {
		preinit_interp_state();
	}
}

void print_globals() {
	VarToVect& vars = get_dex_mod(dex_state.module.ref)->vars;
	DexList* strl = get_dex_list(vars.strList.ref);
	DexList* varl = get_dex_list(vars.varList.ref);
	int s = strl->len;
	if (s) {
		cout << "{";
		dex_print(strl->data[0]);
		cout << " = ";
		dex_str_print(varl->data[0]);
		for (int i = 1; i < s; i++) {
			cout << ", \t";
			if (i % 4 == 0)
				cout << endl;
			dex_print(strl->data[i]);
			cout << " = ";
			dex_str_print(varl->data[i]);
		}
		cout << "}";

	} else
		cout << "{}";
}

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

#ifndef INTERP_STATE_H_
#define INTERP_STATE_H_

#include "../DexRef.h"
#include "../types/dexdict.h"
#include "../types/dexset.h"
#include "../types/dexstdops.h"
#include "../stdobjs/str.h"
#include <cstdio>
#include <vector>

using namespace std;

#ifdef NO_ERROR
#undef NO_ERROR
#endif
//Hardcored error types, DEXOBJ_ERROR handles custom errors
//See error.h
enum err_t {
	NO_ERROR,
	LEX_ERROR,
	PARSE_ERROR,
	OP_ERROR,
	ARG_ERROR,
	DOMAIN_ERROR,
	MEMB_ERROR,
	OUT_OF_MEMORY,
	DEXOBJ_ERROR,
	IO_ERROR,
	STACK_OVERFLOW,

};

struct StackFrame {
	DexRef src_func;
	DexRef src_file;
	StackFrame(const DexRef& func, const DexRef& file) {
		src_func = func;
		src_file = file;
	}
};

struct DexErrState {
	err_t error;
	const char* data;
	DexRef data2;
	vector<StackFrame> stack;
	void print_error();
	DexErrState();
};
static const int WORKPATH_LIM = 100;
static const int STACKOBJ_LIM = 10000;

struct InterpState {
	char workPath[WORKPATH_LIM];

	DexRef langconsts;//dict
	//
	DexRef module;//current module
	DexRef contextList;//list
	//Map of Module Name -> Module object
	//This is different from the module variable that contains a handle
	DexRef loadedModules;//dict
	//Internal modules start with dex.
	//eg import dex.draw
	DexRef internalModules;//dict
	DexRef internObjs;//dict
	int optlevel;
	DexErrState err;
	DexRef* stack, *stackend, *stackptr;

	InterpState();
	~InterpState();

};
void set_module(const DexRef& mod);
void add_internal_module(const DexRef& name, const DexRef& mod);
DexRef load_module(const DexRef& name, const DexRef& modulePath);
//Allocate 'objn' objects on the dexscript stack
//This initializes all objects with NONE
DexRef* dex_alloca(int objn);

//Allocate space for 'objn' objects on the dexscript stack
//This DOES NOT initialize any objects
//You must properly call the DexRef constructor (or equivalent)!
DexRef* dex_raw_alloca(int objn);

//Attempts to add more space on the stack...
//- if 'objv' was allocated on the stack with 'objn' objects
//then 'objv' is returned and there is space is made for
//'newobjn' objects
//Otherwise the space is allocated on stack as normal, with
//the objects in 'objv' copied over
//Note this has initialization caveats as above
DexRef* dex_raw_realloca(DexRef* objv, int objn, int newobjn);
DexRef* dex_realloca(DexRef* objv, int objn, int newobjn);

//Deallocate a stack frame, correcting object ref counts
void dex_dealloca(DexRef* objv, int objn);


extern void dex_init_interpreter();
//Lots of magic behind the initialization
extern InterpState dex_state;

#endif /* INTERP_STATE_H_ */

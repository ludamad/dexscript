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

#include "basic.h"
#include "../mainh.h"
#include <cstdio>
#include <iostream>
#include "../ast/nodes/nodes.h"
#include "objdefutil.h"
#include "../types/dexstr.h"
#include "../interp/error.h"
#include "../objdef/dex_conv.h"

using namespace std;
DexRef DSTR_DEXBUILTIN = create_string("dexbuiltin.cpp");

static DexRef builtin_print(DexRef* args, size_t n) {
	DexRef str;
	for (size_t i = 0; i < n; i++) {
		str = dex_to_string(args[i]);
		if (error()) {
			stack_trace("print", DSTR_DEXBUILTIN);
			break;
		}
		DexStr* ds = get_dex_str(str.ref);
		char* s = ds->data;
		size_t sn = ds->strlen;
		fwrite(s, 1, sn, stdout);
		if (i != n - 1)
			putchar(' ');
	}
	fflush(stdout);
	return NONE;
}
static DexRef builtin_println(DexRef* args, size_t n) {
	DexRef str;
	for (size_t i = 0; i < n; i++) {
		str = dex_to_string(args[i]);
		if (error()) {
			stack_trace("println", DSTR_DEXBUILTIN);
			break;
		}
		DexStr* ds = get_dex_str(str.ref);
		char* s = ds->data;
		size_t sn = ds->strlen;
		fwrite(s, 1, sn, stdout);
		if (i < n - 1)
			putchar(' ');
	}
	putchar('\n');
	fflush(stdout);
	return NONE;
}
static DexRef builtin_readnum(DexRef* args, size_t n) {
	if (n != 0) {
		arg_amnt_err("readnum", n, 0);
		return NONE;
	}
	double d;
	char toss;
	int ret = scanf("%lf%c", &d, &toss);
	if (!isspace(toss))
		ungetc(toss, stdin);
	if (ret == EOF || ret == 0) {
		error(IO_ERROR, "Error reading number from standard input.");
		return NONE;
	}
	return create_number(d);
}
static DexRef builtin_readln(DexRef* args, size_t n) {
	if (n != 0) {
		arg_amnt_err("readln", n, 0);
		return NONE;
	}
	char buff[500];
	if (fgets(buff, sizeof(buff), stdin)) {
		size_t strs = strlen(buff);
		if (buff[strs - 1] == '\n')
			strs--;
		return create_string(buff, strs);
	} else {
		error(IO_ERROR, "Error reading line from standard input.");
		stack_trace("readln", DSTR_DEXBUILTIN);
		return NONE;
	}
}
static DexRef builtin_read(DexRef* args, size_t n) {
	const char* NAME = "read";
	if (n != 1) {
		arg_amnt_err("read", n, 1);
		return NONE;
	}
	size_t number;
	char buff[500];
	if (!dex_econv(&number, args[0], NAME)) {
		return NONE;
	}
	if (fread(buff, 1, number,  stdin)) {
		return create_string(buff);
	} else {
		error(IO_ERROR, "Error reading characters from standard input.");
		stack_trace("read", DSTR_DEXBUILTIN);
		return NONE;
	}
}
static DexRef builtin_hash(DexRef* args, size_t n) {
	if (n != 1) {
		arg_amnt_err("hash", n, 1);
		stack_trace("hash", DSTR_DEXBUILTIN);
		return NONE;
	}
	return create_number(dex_hash(*args));
}
static DexRef builtin_pass(DexRef* args, size_t n) {
	DexRef temp;
	DexList* dl;
	if (n != 2)
		arg_amnt_err("pass", n, 2);
	else if (args[1]->type == &listType) {
		dl = get_dex_list(args[1].ref);
	} else {
		temp = list_from_seq(args[1]);
		dl = get_dex_list(temp.ref);
	}
	temp = dex_call(args[0], dl->data, dl->len);
	if (error()) {
		stack_trace("pass", DSTR_DEXBUILTIN);
		return NONE;
	}
	return temp;
}
static DexRef builtin_merge(DexRef* args, size_t n) {
	if (n != 2)
		arg_amnt_err("merge", n, 2);
	else if (args[0]->type != &dictType && args[1]->type != &dictType) {
		arg_type_err("merge", "dict");
	} else {
		dict_merge(args[0], args[1]);
		if (error()) {
			stack_trace("merge", DSTR_DEXBUILTIN);
			return NONE;
		}
		return args[0];
	}
	return NONE;
}
static DexRef builtin_ref(DexRef* args, size_t n) {
	if (n != 1) {
		arg_amnt_err("ref", n, 1);
		return NONE;
	}
	size_t st = args[0]->refc;
	st &= REST_MASK;
	return create_number(st);
}

static DexRef builtin_module(DexRef* args, size_t n) {
	if (n != 2 || args[0]->type != &strType || args[1]->type != &strType) {
		arg_amnt_err("module", n, 2);
		return NONE;
	}
	return load_module(args[0], args[1]);
}

static DexRef builtin_err(DexRef* args, size_t n) {
	if (n != 1) {
		arg_amnt_err("err", n, 1);
		return NONE;
	}
	error(DEXOBJ_ERROR, args[0]);
	return NONE;
}

static DexRef builtin_catch_call(DexRef* args, size_t n) {
	if (n < 1) {
		arg_amnt_err("catch_call", n, 1);
		return NONE;
	}
	DexRef ret = dex_call(args[0], args + 1, n - 1);
	err_t& err = error();
	if (err){
		err = NO_ERROR;
		return create_string("An error has occurred.");
	}
	return ret;
}

static DexRef builtin_to_charcode(DexRef* args, size_t n) {
	if (n != 1) {
		arg_amnt_err("to_charcode", n, 1);
		return NONE;
	} else if (args->ref->type != &strType) {
		arg_type_err("to_charcode", "str");
	}
	DexStr* s = get_dex_str(args->ref);
	if (s->strlen == 1)
		return create_number(s->data[0]);
	DexRef l = create_list(s->strlen);
	for (size_t i = 0; i < s->strlen; i++)
		list_push(l, create_number(s->data[i]));
	return l;
}
static DexRef builtin_from_charcode(DexRef* args, size_t n) {
	const char* NAME = "from_charcode";

	bool islist = false;
	if (n == 1 && args->ref->type != &numType) {
		islist = true;
		if (args->ref->type != &listType) {
			arg_type_err(NAME, "list");
			return NONE;
		}
	}
	DexRef str = create_string("", 0, 32);
	if (islist) {
		DexList* dl = get_dex_list(args->ref);
		args = dl->data;
		n = dl->len;
	}

	for (size_t i = 0; i < n; i++) {
		DexObj* num = args[i].ref;
		if (num->type != &numType) {
			arg_type_err(NAME, "numeric");
			stack_trace(NAME, DSTR_DEXBUILTIN);
			return NONE;
		}
		str_append(str, (char) get_dex_num(args[i].ref));
	}
	return str;
}

DexRef DFUNC_PRINT = create_builtin(&builtin_print, "print");
DexRef DFUNC_PRINTLN = create_builtin(&builtin_println, "println");
DexRef DFUNC_READNUM = create_builtin(&builtin_readnum, "readnum");
DexRef DFUNC_READLN = create_builtin(&builtin_readln, "readln");
DexRef DFUNC_READ = create_builtin(&builtin_read, "read");
DexRef DFUNC_HASH = create_builtin(&builtin_hash, "hash");
DexRef DFUNC_PASS = create_builtin(&builtin_pass, "pass");
DexRef DFUNC_REF = create_builtin(&builtin_ref, "ref");
DexRef DFUNC_ERR = create_builtin(&builtin_err, "err");
DexRef DFUNC_CATCH_CALL = create_builtin(&builtin_catch_call, "catch_call");
DexRef DFUNC_MERGE = create_builtin(&builtin_merge, "merge");
DexRef DFUNC_MODULE = create_builtin(&builtin_module, "module");
DexRef DFUNC_TO_CHARCODE = create_builtin(&builtin_to_charcode, "to_charcode");
DexRef DFUNC_FROM_CHARCODE = create_builtin(&builtin_from_charcode,
		"from_charcode");

void put_interned(DexRef& dict, const DexRef& str, const DexRef& dr) {
	dict_put(dict, dex_intern(str), DFUNC_PRINT);
}
void put_builtin_consts(DexRef& dict) {
	dex_put_builtin(dict, DFUNC_PRINT);
	dex_put_builtin(dict, DFUNC_PRINTLN);
	dex_put_builtin(dict, DFUNC_READNUM);
	dex_put_builtin(dict, DFUNC_READLN);
	dex_put_builtin(dict, DFUNC_READ);
	dex_put_builtin(dict, DFUNC_HASH);
	dex_put_builtin(dict, DFUNC_SORT);
	dex_put_builtin(dict, DFUNC_PASS);
	dex_put_builtin(dict, DFUNC_CATCH_CALL);
	dex_put_builtin(dict, DFUNC_REF);
	dex_put_builtin(dict, DFUNC_ERR);
	dex_put_builtin(dict, DFUNC_MERGE);
	dex_put_builtin(dict, DFUNC_MODULE);
	dex_put_builtin(dict, DFUNC_TO_CHARCODE);
	dex_put_builtin(dict, DFUNC_FROM_CHARCODE);
}


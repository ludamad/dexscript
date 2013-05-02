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

#include "iters.h"
#include "../mainh.h"
#include "objdefutil.h"
#include "../types/dexstr.h"
#include "math.h"
#include "../interp/error.h"
static DexRef DSTR_DEXITERS = create_string("dexiters.cpp");
static DexRef builtin_rev_cmp(DexRef* args, size_t n) {
	if (n != 2) {
		arg_amnt_err("rev_cmp", n, 1);
	}
	int cmp = dex_compare(args[0], args[1]);
	if (cmp < 0)
		return DNUM_ONE;
	if (!cmp)
		return DNUM_ZERO;
	return DNUM_NEG_ONE;
}
static DexRef builtin_cmp(DexRef* args, size_t n) {
	if (n != 2) {
		arg_amnt_err("cmp", n, 1);
	}
	int cmp = dex_compare(args[0], args[1]);
	if (cmp > 0)
		return DNUM_ONE;
	if (!cmp)
		return DNUM_ZERO;
	return DNUM_NEG_ONE;
}

DexRef DFUNC_REV_CMP = create_builtin(&builtin_rev_cmp, "rev_cmp");
DexRef DFUNC_CMP = create_builtin(&builtin_cmp, "cmp");

//Takes either A, function of A
//Or, A, other sequence to map to
//A, scriptable object (returns list of keys sorted by their value)
static DexRef builtin_key_sort(DexRef* args, size_t n) {
	/*DexRef*
	if (n == 2){
		arg_amnt_err("key_sort", n, 2);
		return NONE;
	}
	if (args->ref->type != &listType) {
		args[0] = list_from_seq(args[0]);
		if (error()) {
			stack_log("key_sort", DSTR_DEXITERS);
			return NONE;
		}
	}
	if (n == 1 || args[1].ref == DFUNC_CMP.ref)
		list_sort(args[0]);
	else if (args[1].ref = DFUNC_REV_CMP.ref ){
		list_rev_sort(args[0]);
	} else {
		list_sort_cmpf(args[0], args[1]);
	}

	if (error()) {
		stack_log("key_sort", DSTR_DEXBUILTIN);
		return NONE;
	}*/
	return args[0];
}
static DexRef builtin_sort(DexRef* args, size_t n) {
	if (n < 1 || n > 2){
		arg_amnt_err("sort", n, 1, 2);
		return NONE;
	}
	if (args->ref->type != &listType) {
		args[0] = list_from_seq(args[0]);
		if (error()) {
			stack_trace("sort", DSTR_DEXITERS);
			return NONE;
		}
	}
	if (n == 1 || args[1].ref == DFUNC_CMP.ref)
		list_sort(args[0]);
	else if (args[1].ref == DFUNC_REV_CMP.ref ){
		list_rev_sort(args[0]);
	} else {
		list_sort_cmpf(args[0], args[1]);
	}

	if (error()) {
		stack_trace("sort", DSTR_DEXITERS);
		return NONE;
	}
	return args[0];
}

typedef size_t ITER[4];
static DexRef builtin_map(DexRef* args, size_t n) {
	if (n < 2) {
		arg_amnt_err("map", n, 2);
	}

	DexRef* mapped_args = new DexRef[n];
	ITER* iters = new ITER[n];

	DexRef out = create_list(16);

	bool more = false;
	for (int i = 1; i < n; i++) {
		if (dex_init_iter(mapped_args[i], args[i], iters + i))
			more = true;
	}
	while (more) {
		list_push(out, dex_call(args[0], mapped_args + 1, n - 1));
		if (error())
			break;
		more = false;
		for (int i = 1; i < n; i++) {
			if (dex_incr_iter(mapped_args[i], args[i], iters + i))
				more = true;
			else
				mapped_args[i] = NONE;
		}
	}
	return out;

	delete[] mapped_args;
	delete[] iters;

	return NONE;
}

//sum of number objects only
static DexRef builtin_sum(DexRef* args, size_t n) {
	if (n != 1) {
		arg_amnt_err("sum", n, 1);
	}
	double sum = 0.0;

	bool notanum = false;
	if (args->ref->type == &listType) {
		DexList* dl = get_dex_list(args->ref);
		for (int i = 0; i < dl->len; i++) {
			DexObj* n = dl->data[i].ref;
			if (n->type != &numType) {
				notanum = true;
				break;
			}
			sum += get_dex_num(n);
		}
	} else {
		int iterspace[4];
		DexRef it;
		bool more = dex_init_iter(it, args[0], iterspace);
		while (more) {
			if (it->type != &numType) {
				notanum = true;
				break;
			}
			sum += get_dex_num(it.ref);
			more = dex_incr_iter(it, args[0], iterspace);
		}
	}
	if (notanum) {
		arg_type_err("sum", "numeric sequence");
		return NONE;
	}
	return create_number(sum);
}

static DexRef builtin_filter(DexRef* args, size_t n) {
	if (n != 2) {
		arg_amnt_err("filter", n, 1);
	}
	double sum = 0.0;

	bool notanum = false;
	if (args->ref->type == &listType) {
		DexList* dl = get_dex_list(args->ref);
		for (int i = 0; i < dl->len; i++) {
			DexObj* n = dl->data[i].ref;
			if (n->type != &numType) {
				notanum = true;
				break;
			}
			sum += get_dex_num(n);
		}
	} else {
		int iterspace[4];
		DexRef it;
		bool more = dex_init_iter(it, args[0], iterspace);
		while (more) {
			if (it->type != &numType) {
				notanum = true;
				break;
			}
			sum += get_dex_num(it.ref);
			more = dex_incr_iter(it, args[0], iterspace);
		}
	}
	if (notanum) {
		arg_type_err("sum", "numeric sequence");
		return NONE;
	}
	return create_number(sum);
}

DexRef DFUNC_MAP = create_builtin(&builtin_map, "map");
DexRef DFUNC_SORT = create_builtin(&builtin_sort, "sort");
DexRef DFUNC_SUM = create_builtin(&builtin_sum, "sum");
void put_iter_consts(DexRef& dict) {
	dex_put_builtin(dict, DFUNC_SUM);
	dex_put_builtin(dict, DFUNC_SORT);
	dex_put_builtin(dict, DFUNC_MAP);
	dex_put_builtin(dict, DFUNC_REV_CMP);
	dex_put_builtin(dict, DFUNC_CMP);
}


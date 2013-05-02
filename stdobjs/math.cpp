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

#include "../types/funcs/dexbuiltin.h"
#include "math.h"
#include "../types/dexnum.h"
#include "../types/dexstr.h"
#include "objdefutil.h"
#include "../libs/rand/mtwist.h"
#include "../objdef/dex_conv.h"

static DexRef DSTR_MATHCPP = create_string("math.cpp");

DexRef DNUM_PI = create_number(PI), DNUM_E = create_number(E);

DexRef DNUM_NEG_ONE = create_number(-1), DNUM_ONE = create_number(1), DNUM_ZERO = create_number(0);

//0 arg -> return number 0 to 1 (float)
//1 arg -> return number 0 to n (int)
//2 arg -> return number a to b (int)
static DexRef builtin_rand(DexRef* args, size_t n) {
	const char* NAME = "rand";

	if (n > 2) {
		arg_amnt_err(NAME, n, 0, 2);
		return NONE;
	}
	if (n == 0) {
		return create_number(twist_state.genrand_res53());
	} else if (n == 1) {
		int max;
		if (!dex_econv(&max, args[0], NAME)) {
			return NONE;
		} else {
			if (max <= 0)
				arg_type_err(NAME,"int (greater than 0)");
			size_t st = twist_state.genrand_int32();
			return create_number(st % max);
		}
	} else {
		int min, max;
		if (!dex_econv(&min, args[0], NAME) || !dex_econv(&max, args[1], NAME)) {
			return NONE;
		} else {
			if (min > max) {
				int t = min;
				min = max;
				max = t;
			} else if (min == max) {
				return create_number(min);
			}
			size_t st = twist_state.genrand_int32();
			return create_number(min + int(st % (max - min)));
		}
	}
	return NONE;
}

DexRef DFUNC_RAND = create_builtin(&builtin_rand, "rand");

static DexRef builtin_max(DexRef* args, size_t n) {
	if (n == 0)
		return NONE;
	DexRef best = args[0];
	for (int i = 1; i < n; i++) {
		if (best < args[i])
			best = args[i];
	}
	if (error()) {
		stack_trace("max", DSTR_MATHCPP);
		return NONE;
	} else
		return best;
}

static DexRef builtin_min(DexRef* args, size_t n) {
	if (n == 0)
		return NONE;
	DexRef best = args[0];
	for (int i = 1; i < n; i++) {
		if (args[i] < best)
			best = args[i];
	}
	if (error()) {
		stack_trace("min", DSTR_MATHCPP);
		return NONE;
	} else
	return best;
}

DexRef DFUNC_MAX = create_builtin(&builtin_max, "max");
DexRef DFUNC_MIN = create_builtin(&builtin_min, "min");

struct F_SIN {
	static DexRef file() {
		return DSTR_MATHCPP;
	}
	static const char* name() {
		return "sin";
	}
	static double func(double d) {
		return sin(d);
	}
};
struct F_COS {
	static DexRef file() {
		return DSTR_MATHCPP;
	}
	static const char* name() {
		return "cos";
	}
	static double func(double d) {
		return cos(d);
	}
};
struct F_CEIL {
	static DexRef file() {
		return DSTR_MATHCPP;
	}
	static const char* name() {
		return "ceil";
	}
	static double func(double d) {
		return ceil(d);
	}
};
struct F_FLOOR {
	static DexRef file() {
		return DSTR_MATHCPP;
	}
	static const char* name() {
		return "floor";
	}
	static double func(double d) {
		return floor(d);
	}
};
struct F_ROUND {
	static DexRef file() {
		return DSTR_MATHCPP;
	}
	static const char* name() {
		return "round";
	}
	static double func(double d) {
#ifdef _MSC_VER
		return floor(d+.5);
#else
		return round(d);
#endif
	}
};
struct F_ATAN {
	static DexRef file() {
		return DSTR_MATHCPP;
	}
	static const char* name() {
		return "atan";
	}
	static double func(double d) {
		return atan(d);
	}
};
struct F_ATAN2 {
	static DexRef file() {
		return DSTR_MATHCPP;
	}
	static const char* name() {
		return "atan2";
	}
	static double func(double d, double d2) {
		return atan2(d, d2);
	}
};
void put_math_consts(DexRef& dict) {

	dex_add_num1f<F_SIN> (dict);
	dex_add_num1f<F_COS> (dict);
	dex_add_num1f<F_ROUND> (dict);
	dex_add_num1f<F_FLOOR> (dict);
	dex_add_num1f<F_CEIL> (dict);
	dex_add_num1f<F_ATAN> (dict);
	dex_add_num2f<F_ATAN2> (dict);

	dex_put_intern(dict, "pi", 2, DNUM_PI);
	dex_put_intern(dict, "e", 1, DNUM_E);
	dex_put_builtin(dict, DFUNC_RAND);
	dex_put_builtin(dict, DFUNC_MAX);
	dex_put_builtin(dict, DFUNC_MIN);
}

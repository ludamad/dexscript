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

#include "../mainh.h"
#include "objdefutil.h"
#include "types.h"
#include "basic.h"
#include "math.h"
#include "str.h"
#include "iters.h"
#include <iostream>
void put_basic_consts(DexRef& dict) {
	dict_put(dict, DISTR_TRUE, dex_true);
	dict_put(dict, DISTR_FALSE, dex_false);
	dict_put(dict, DISTR_NONE, NONE);
}

void put_all_consts(DexRef& dict){
	put_builtin_consts(dict);
	put_math_consts(dict);
	put_basic_consts(dict);
	put_type_consts(dict);
	put_iter_consts(dict);
}

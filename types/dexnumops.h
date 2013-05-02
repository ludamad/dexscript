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

#ifndef DEXNUMOPS_H_
#define DEXNUMOPS_H_
#include "dexnum.h"
#include "../objdef/dex_conv.h"
#include <cmath>
void num_add(DexRef& dst, const DexRef& o){
	if (dst->refc > 1){
		dst = create_number(get_dex_num(dst.ref));
	}
	get_dex_num(dst.ref) += get_dex_num(o.ref);
}

void num_mod(DexRef& dst, const DexRef& o){
	double val = fmod(get_dex_num(dst.ref),get_dex_num(o.ref));
	if (dst->refc > 1){
		dst = create_number(val);
	}
	else
		get_dex_num(dst.ref) = val;
}
void num_pow(DexRef& dst, const DexRef& o){
	double val = std::pow(
			get_dex_num(dst.ref), get_dex_num(o.ref));
	if (dst->refc > 1){
		dst = create_number(val);
	}
	else
		get_dex_num(dst.ref) = val;

}
void num_mul(DexRef& dst, const DexRef& o){
	if (dst->refc > 1){
		dst = create_number(get_dex_num(dst.ref));
	}
	get_dex_num(dst.ref) *= get_dex_num(o.ref);
}
void num_div(DexRef& dst, const DexRef& o){
	if (dst->refc > 1){
		dst = create_number(get_dex_num(dst.ref));
	}
	if (get_dex_num(o.ref) == 0)
		error(ARG_ERROR, "Division by zero isn't allowed you idiot.");
	else
	get_dex_num(dst.ref) /= get_dex_num(o.ref);
}
void num_bitor(DexRef& dst, const DexRef& o){
	int i1, i2;
	if ( !is_int(get_dex_num(dst.ref), i1) || !is_int(get_dex_num(o.ref), i2) )
		error(ARG_ERROR, "Bit operations require integers.");
	dst = double(i1 | i2);
}
void num_bitxor(DexRef& dst, const DexRef& o){
	int i1, i2;
	if ( !is_int(get_dex_num(dst.ref), i1) || !is_int(get_dex_num(o.ref), i2) )
		error(ARG_ERROR, "Bit operations require integers.");
	dst = double(i1 ^ i2);
}
void num_bitand(DexRef& dst, const DexRef& o){
	int i1, i2;
	if ( !is_int(get_dex_num(dst.ref), i1) || !is_int(get_dex_num(o.ref), i2) )
		error(ARG_ERROR, "Bit operations require integers.");
	dst = double(i1 & i2);
}
void num_bitsup(DexRef& dst, const DexRef& o){
	int i1, i2;
	if ( !is_int(get_dex_num(dst.ref), i1) || !is_int(get_dex_num(o.ref), i2) )
		error(ARG_ERROR, "Bit operations require integers.");
	dst = double(i1 << i2);
}
void num_bitsdown(DexRef& dst, const DexRef& o){
	int i1, i2;
	if ( !is_int(get_dex_num(dst.ref), i1) || !is_int(get_dex_num(o.ref), i2) )
		error(ARG_ERROR, "Bit operations require integers.");
	dst = double(i1 >> i2);
}
void num_sub(DexRef& dst, const DexRef& o){
	if (dst->refc > 1){
		dst = create_number(get_dex_num(dst.ref));
	}
	get_dex_num(dst.ref) -= get_dex_num(o.ref);
}

#endif /* DEXNUMOPS_H_ */

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

#ifndef DEXGEN_H_
#define DEXGEN_H_
#include "../../DexRef.h"
#include "../typetools.h"

typedef	bool (DexIterF)(DexRef& dr, const DexRef& cont, void* iter);
struct DexGen {
	const char* name;
	DexIterF* initf, *iterf;
	size_t* memb_ptr(){
		return (size_t*)(sizeof(DexGen)+(size_t)this);
	}
};

inline DexGen* get_dex_gen(DexObj* dobj){
	return (DexGen*)get_dex_memb(dobj,0);
}
inline bool gen_init_iter(DexRef& dr, const DexRef& c, void* iter){
	return (*(get_dex_gen(c.ref)->initf))(dr, c, iter);
}

inline bool gen_incr_iter(DexRef& dr, const DexRef& c, void* iter){
	return (*(get_dex_gen(c.ref)->iterf))(dr, c, iter);
}
DexObj* create_range(int min, int max);
DexObj* create_gen(DexIterF* is, DexIterF* in, const char* name, size_t mem);
extern DexType genType;
extern DexRef DFUNC_RANGE, DSTR_RANGE;

#endif /* DEXGEN_H_ */

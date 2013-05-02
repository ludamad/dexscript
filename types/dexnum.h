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

#ifndef DEXNUM_H_
#define DEXNUM_H_
#include "../mainh.h"
#include "typetools.h"//Guarantee its loaded first

inline double& get_dex_num(DexObj* dobj){
	return *(double*)get_dex_memb(dobj,0);
}

extern DexType numType;
DexObj* create_number(double d);
size_t num_hash(const DexRef& d1);
size_t num_hash(const double& val);
size_t num_to_cstr(const double& val, char* buff);
size_t int_to_cstr(int val, char* buff);
#endif /* DEXNUM_H_ */

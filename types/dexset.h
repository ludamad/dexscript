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

#ifndef DEXSET_H_
#define DEXSET_H_
#include "../DexRef.h"

extern DexType setType;
DexObj* create_set(size_t buckets = 16);
DexObj* create_set(DexRef* entries, size_t len);
DexRef* set_add(DexRef& s, const DexRef& val);
DexRef* set_add_cstr(DexRef& s, const char* c, size_t n);

bool set_remove(DexRef& s, const DexRef& val);

void set_clear(DexRef& s);

DexRef* set_add_num(DexRef& s, const double& n);

void set_merge(DexRef& s, const DexRef& s2);
size_t set_hash(const DexRef& s);

bool set_incr_iter(DexRef& val, const DexRef& s, size_t* i);
bool set_equal(const DexRef& s1, const DexRef& s2);
DexRef* set_find(const DexRef& d1, const DexRef& v);

DexRef set_to_list(const DexRef& s);

size_t set_size(const DexRef& s);

#endif /* DEXSET_H_ */

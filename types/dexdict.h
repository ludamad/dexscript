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
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *  Created on: Jan 22, 2011
 */

#ifndef DEXDICT_H_
#define DEXDICT_H_
#include "../DexRef.h"
#include <algorithm>

extern DexType dictType;
DexObj* create_dict(size_t buckets = 16);
DexObj* create_dict(std::pair<DexRef,DexRef>* entries, size_t len);
void dict_put(DexRef& d1, const DexRef& k, const DexRef& v);
DexRef dict_get(const DexRef& d1, const DexRef& k);
DexRef* dict_pget(DexRef& d1, const DexRef& k);
void dict_merge(DexRef& d1, const DexRef& d2);
size_t dict_hash(const DexRef& d1);
size_t dict_hash_entries(const DexRef& d1);
bool dict_remove(const DexRef& d1, const DexRef& k);
void dict_intern_keys(DexRef& d1);
void dict_clear(DexRef& d1);

bool dict_incr_iter(DexRef& key, const DexRef& d1, size_t* i);

//Returns NULL if done
DexRef* dict_next_entry(const DexRef& d1, DexRef* iter);

bool dict_equal(const DexRef& d1, const DexRef& d2);
bool dict_has_key(const DexRef& d1, const DexRef& k);
size_t dict_size(const DexRef& d);
DexRef* dict_raw_array(const DexRef& d);
#endif /* DEXDICT_H_ */

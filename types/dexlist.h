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

#ifndef DEXLIST_H_
#define DEXLIST_H_
#include <algorithm>
#include "../DexRef.h"
#include "typetools.h"

struct DexList {
	size_t len;
	DexRef data[0];
};

inline DexList* get_dex_list(DexObj* d){
	return (DexList*)get_dex_memb(d,0);
}
extern DexType listType;
DexObj* create_list(size_t space = 16);
DexObj* create_list(DexRef* entries, size_t len, size_t buffspace = 0);
DexRef list_from_seq(const DexRef& dexseq);
void list_push(DexRef& l, const DexRef& val);
void list_pop(DexRef& l);
void list_concat(DexRef& l, const DexRef& o);

DexRef list_get(const DexRef& l, size_t ind);
DexRef* list_pget(DexRef& l, size_t ind);
void list_set(DexRef& l, size_t ind, const DexRef& d);
void list_intern_all(DexRef& l);

void list_clear(DexRef& l);
bool valid_list_key(const DexRef& o, int& index);
void list_sort(DexRef& l);
void list_sort_keyf(DexRef& l, DexRef keyf);
void list_rev_sort(DexRef& l);
void list_sort_cmpf(DexRef& l, DexRef cmpf);
void list_reverse(DexRef& l);
size_t list_hash(const DexRef& l);
inline size_t list_size(const DexRef& l){
	return get_dex_list(l.ref)->len;
}
bool list_equal(const DexRef& l, const DexRef& l2);
int list_index_of(const DexRef& l, const DexRef& val);

#endif /* DEXLIST_H_ */

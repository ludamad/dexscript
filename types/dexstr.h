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

#ifndef DEXSTR_H_
#define DEXSTR_H_
#include "../mainh.h"
#include "typetools.h"//Guarantee its loaded first

struct DexStr {
	size_t strlen;
	size_t cached_hash;
	char data[0];
};
inline DexStr* get_dex_str(DexObj* dobj){
	return (DexStr*)get_dex_memb(dobj,0);
}

extern DexType strType;
DexObj* create_string(const char* c);
DexObj* create_string(const char* c, size_t strlen);
DexObj* create_string(const char* c, size_t strlen, size_t alen);
void str_append(DexRef& d1, char c);
void str_concat(DexRef& d1, const DexRef& d2);
void str_substr(DexRef& d1, int start, int end);
//Takes arr of strings
void str_concat_all(DexRef& d1, DexRef* strs, size_t strn);


size_t str_len(const DexRef& dr);
void strc_concat(DexRef& d1, const char* c, size_t strlen);
void strc_concat(DexRef& d1, const char* c);
void str_set_len(DexRef& d1, size_t news);
size_t str_hash(const DexRef& d1);
size_t strc_hash(const char* c, size_t n);
void str_tocase(DexRef& str, bool up);
size_t str_capacity(const DexRef& d1);
bool str_equal(const DexRef& s1, const DexRef& s2);
bool strc_equal(const DexRef& s1, const char* c, size_t strlen);

inline char* str_get_cstr(const DexRef& s){
	return get_dex_str(s.ref)->data;
}

#endif /* DEXSTR_H_ */

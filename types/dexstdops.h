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

#ifndef DEXSTDOPS_H_
#define DEXSTDOPS_H_
#include "../DexRef.h"

bool dex_equal(const DexRef& d1, const DexRef& d2);
size_t dex_hash(const DexRef& d1);
void dex_print(const DexRef& d1);
void dex_str_print(const DexRef& d1);

DexRef dex_to_string(const DexRef& d1);
DexRef dex_call(const DexRef& d1, DexRef* args, size_t n);//For function objects

DexRef dex_get_subscr(const DexRef& d1, const DexRef& k);
DexRef* dex_pget(DexRef& d1, const DexRef& k);
void dex_set_subscr(DexRef& d1, const DexRef& k, const DexRef& v);
void dex_operation(char opcode, DexRef& dst, const DexRef& o);

bool dex_init_iter(DexRef& it, const DexRef& cont, void* iter);
bool dex_incr_iter(DexRef& it, const DexRef& cont, void* iter);

int dex_compare(const DexRef& a, const DexRef& b);

bool dex_identity_equal(const DexRef& d1, const DexRef& d2);//Returns if addresses equal
size_t dex_identity_hash(const DexRef& d1);//Returns address
bool dex_is_true(const DexRef& d1);

bool dex_is_seq(const DexRef& v);

DexRef dex_identity_func(const DexRef& d1);

//All the following assume that str is a valid interned string
DexRef dex_get_member(const DexRef& obj, const DexRef& str);
bool dex_has_member(const DexRef& obj, const DexRef& str);
DexRef* dex_pget_member(DexRef& obj, const DexRef& str);

DexRef dex_call_member(const DexRef& memb, const DexRef& obj, DexRef* args,
		size_t n);
void dex_call_memb_equ(const DexRef& memb, DexRef& obj, DexRef* args, size_t n);

void dex_set_member(DexRef& obj, const DexRef& str, const DexRef& val);

void dex_log(DexRef& dr);

#endif /* DEXSTDOPS_H_ */

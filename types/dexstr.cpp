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

#include "dexstr.h"
#include "dexstrops.h"
#include "typetools.h"
#include <cstring>
#include "dexstdops.h"
#include <iostream>
#include "../stdobjs/methods.h"
#include "../stdobjs/str.h"
#include "../stdobjs/struct.h"
#include "../objdef/dex_conv.h"
#include <algorithm>
#include <cctype>
using namespace std;
DexRef DEXSTRCPP = create_string("dexstr.cpp");

static DexRef create_strobj(DexType* dext, DexRef* dobjs, size_t len) {
	DexRef ret = EMPTY_STR;
	for (size_t i = 0; i < len; i++)
		str_concat(ret, dex_to_string(dobjs[i]));
	return ret;
}

inline size_t str_alloc_len(size_t strlen) {
	return sizeof(DexObj) + sizeof(DexStr) + strlen + 1;
}
inline size_t str_buff_len(size_t strlen) {
	return sizeof(DexObj) + sizeof(DexStr) + roundUpPower2(strlen) + 1;
}
size_t str_len(const DexRef& dr) {
	return get_dex_str(dr.ref)->strlen;
}

void str_tocase(DexRef& str, bool up){
	DexStr* dstr = get_dex_str(str.ref);
	if (str->refc > 1) {
		str = create_string(dstr->data, dstr->strlen);
		dstr = get_dex_str(str.ref);
	}
	int len = dstr->strlen;
	char* data= dstr->data;
	if (up){
		for (int i = 0; i < len; i++){
			data[i] = toupper(data[i]);
		}
	} else {
		for (int i = 0; i < len; i++){
			data[i] = tolower(data[i]);
		}

	}
}
static void init_str_base(DexStr* dstr, size_t chars, const char* src) {
	dstr->cached_hash = 0;
	memcpy(dstr->data, src, chars);
	dstr->data[chars] = '\0';
	dstr->strlen = chars;
}
void str_set_len(DexRef& d1, size_t news) {
	DexStr* dstr = get_dex_str(d1.ref);
	if (d1->refc <= 1) {
		dstr->cached_hash = 0;
		dstr->strlen = news;
	} else {
		d1 = create_string(dstr->data, news);
	}
}
size_t strc_hash(const char* c, size_t n) {
	const char *endc = c + n;
	size_t x = (*c) << 7;
	while (c < endc) {
		x = (1000003 * x) ^ *c;
		c++;
	}
	x ^= n;
	if (x == 0)
		x = 0x55101241;
	return x;
}
static size_t _str_hash(DexStr* dstr) {
	return strc_hash(dstr->data, dstr->strlen);
}
/*
 static void destroy_string(DexObj* dobj) {
 if (isalnum(get_dex_str(dobj)->data[0])) {
 std::cout << "DESTROYING ~~~~ " << std::endl;
 std::cout << get_dex_str(dobj)->data << std::endl;
 }
 }*/

DexObj* create_string(const char* c) {
	return create_string(c, strlen(c));
}
DexObj* create_string(const char* c, size_t strlen) {
	DexObj* ret = alloc_dex_base(&strType, str_alloc_len(strlen));
	DexStr* dstr = get_dex_str(ret);
	init_str_base(dstr, strlen, c);
	return ret;
}

DexObj* create_string(const char* c, size_t strlen, size_t alen) {
	DexObj* ret = alloc_dex_base(&strType, str_alloc_len(alen));
	DexStr* dstr = get_dex_str(ret);
	init_str_base(dstr, strlen, c);
	return ret;
}

void str_append(DexRef& d1, char c) {
	DexStr* str = get_dex_str(d1.ref);
	size_t sl = str->strlen;
	size_t al = str_alloc_len(sl);
	if (d1->refc > 1 || al + 1 > d1->len) {
		size_t nl = str_buff_len(sl);
		d1 = alloc_dex_copy_base(d1.ref, al, nl);
		str = get_dex_str(d1.ref);
	}
	str->data[sl] = c;
	str->data[sl + 1] = '\0';
	str->cached_hash = 0;
	str->strlen++;
}
void str_concat(DexRef& d1, const DexRef& d2) {
	DexStr* str = get_dex_str(d1.ref), *str2 = get_dex_str(d2.ref);
	size_t sl = str->strlen, sl2 = str2->strlen, news = sl + sl2;
	size_t al = str_alloc_len(sl);
	if (d1->refc > 1 || al + sl2 > d1->len) {
		size_t nl = str_buff_len(news);
		d1 = alloc_dex_copy_base(d1.ref, al, nl);
		str = get_dex_str(d1.ref);
	}
	memcpy(str->data + sl, str2->data, sl2);
	str->strlen = news;
	str->cached_hash = 0;
	str->data[news] = '\0';
}

void str_concat_all(DexRef& d1, DexRef* strs, size_t strn) {
	//Prework
	char* iter;
	DexStr* str1 = get_dex_str(d1.ref), *ostr;
	size_t slen = str1->strlen;
	size_t al = str_alloc_len(slen), tl;
	size_t addlen = 0, minlen, news;

	for (size_t i = 0; i < strn; i++) {
		ostr = get_dex_str(strs[i].ref);
		addlen += ostr->strlen;
	}
	minlen = al + addlen;
	news = slen + addlen;

	if (d1->refc > 1 || minlen > d1->len) {
		tl = str_buff_len(news);
		d1 = alloc_dex_copy_base(d1.ref, al, tl);
		str1 = get_dex_str(d1.ref);
	}
	//Append the strings
	iter = str1->data + slen;

	for (size_t i = 0; i < strn; i++) {
		ostr = get_dex_str(strs[i].ref);
		tl = ostr->strlen;
		memcpy(iter, ostr->data, tl);
		iter += tl;
	}
	str1->strlen = news;
	str1->cached_hash = 0;
	str1->data[news] = '\0';
}

size_t str_capacity(const DexRef& d1) {
	return d1->len - sizeof(DexObj) - sizeof(DexStr);
}
void strc_concat(DexRef& d1, const char* c) {
	strc_concat(d1, c, strlen(c));
}
void strc_concat(DexRef& d1, const char* c, size_t n) {
	DexStr* str = get_dex_str(d1.ref);
	size_t sl = str->strlen, news = sl + n;
	size_t al = str_alloc_len(sl);
	if (d1->refc > 1 || al + n > d1->len) {
		size_t nl = str_buff_len(news);
		d1 = alloc_dex_copy_base(d1.ref, al, nl);
		str = get_dex_str(d1.ref);
	}
	memcpy(str->data + sl, c, n);
	str->strlen = news;
	str->cached_hash = 0;
	str->data[news] = '\0';
}
size_t str_hash(const DexRef& d1) {
	DexStr* ds = get_dex_str(d1.ref);
	if (ds->cached_hash)
		return ds->cached_hash;
	return ds->cached_hash = _str_hash(ds);
}
bool str_equal(const DexRef& s1, const DexRef& s2) {
	if (s1.ref == s2.ref)
		return true;
	DexStr* ds = get_dex_str(s1.ref), *ds2 = get_dex_str(s2.ref);
	if (ds->strlen != ds2->strlen)
		return false;
	if (ds->cached_hash != ds2->cached_hash && ds->cached_hash != 0
			&& ds2->cached_hash != 0)
		return false;
	for (size_t i = 0; i < ds->strlen; i++) {
		if (ds->data[i] != ds2->data[i])
			return false;
	}
	return true;
}
bool strc_equal(const DexRef& s1, const char* c, size_t n) {
	DexStr* ds = get_dex_str(s1.ref);
	if (ds->strlen != n)
		return false;
	for (size_t i = 0; i < n; i++) {
		if (ds->data[i] != c[i])
			return false;
	}
	return true;
}

void str_substr(DexRef& d1, int start, int end) {
	DexStr* str = get_dex_str(d1.ref);
	size_t sl = str->strlen;
	bool err = false;

	if (start < 0) {
		start += sl;
		if (start < 0)
			err = true;
	} else if (start >= sl) {
		err = true;
	}

	if (end < 0) {
		end += sl;
		if (end < 0)
			err = true;
	} else if (end >= sl) {
		err = true;
	}
	size_t nl = end - start;

	/*printf("Substring of '%s', s=%d, e=%d\n",
			str->data, start, end);
	fflush(stdout);*/

	if (d1->refc > 1) {
		d1 = create_string(str->data + start, nl);
	} else {
		str->strlen = nl;
		if (start != 0)
			memcpy(str->data, str->data + start, nl+1);
	}
}
static void dm_strconcat(DexRef& obj, DexRef* args, size_t n) {
	for (size_t i = 0; i < n; i++) {
		if (args[i]->type != &strType) {
			error(ARG_ERROR, "Concat method takes all strings.");
			return;
		}
	}

	str_concat_all(obj, args, n);
}
static void dm_strreplace(DexRef& obj, DexRef* args, size_t n) {
	const char* NAME = "replace";
	if (n != 2) {
		arg_amnt_err(NAME, n, 2);
		return;
	}
	dex_validate(args[0], &strType, NAME);
	dex_validate(args[1], &strType, NAME);
	if (error()) return;
	str_replace(obj, args[0], args[1]);
}
static void dm_strjoin(DexRef& obj, DexRef* args, size_t n) {
	if (n != 1)
		error(ARG_ERROR, ONEARGS);
	const char* cptr = str_get_cstr(obj);
	size_t clen = str_len(obj);

	DexRef col = args[0], val;
	size_t iter = 0;
	DexRef ret = create_string("", 0, 32);
	if (dex_init_iter(val, col, &iter)) {
		str_concat(ret, dex_to_string(val));
		while (dex_incr_iter(val, col, &iter)) {
			strc_concat(ret, cptr, clen);
			str_concat(ret, dex_to_string(val));
		}
	}
	obj = ret;
}

static void dm_strreverse(DexRef& obj, DexRef* args, size_t n) {
	if (n != 0)
		error(ARG_ERROR, NOARGS);
	DexStr* s = get_dex_str(obj.ref);
	if (obj->refc > 1){
		DexRef ret = create_string(NULL,0, s->strlen);
		DexStr* rets = get_dex_str(ret.ref);
		std::reverse_copy(s->data, s->data + s->strlen, rets->data);
		obj = ret;
	} else {
		std::reverse(s->data, s->data + s->strlen);
	}
}
static void dm_strlen(DexRef& obj, DexRef* args, size_t n) {
	if (n != 0)
		error(ARG_ERROR, NOARGS);
	obj = str_len(obj);
}
static void dm_struppercase(DexRef& obj, DexRef* args, size_t n) {
	if (n != 0)
		error(ARG_ERROR, NOARGS);
	str_tocase(obj, true);
}
static void dm_strlowercase(DexRef& obj, DexRef* args, size_t n) {
	if (n != 0)
		error(ARG_ERROR, NOARGS);
	str_tocase(obj, false);
}
static void dm_strsubstr(DexRef& obj, DexRef* args, size_t n) {
	if (n == 0) return;
	static const char* NAME = "substr";
	int mn, mx;

	if (n == 1) {
		mn = 0;
		dex_econv(&mx,args[0], NAME);
	} else if (n == 2) {
		dex_econv(&mn, args[0], NAME);
		dex_econv(&mx, args[1], NAME);
	} else
		error(ARG_ERROR, "Expected 1-2 arguments.");
	if (!error())
		str_substr(obj,mn, mx);
}

static void add_str_methods(DexType* det) {
	add_std_methods(det);
	add_method(det, DISTR_LEN, &dm_strlen);
	add_method(det, DISTR_JOIN, &dm_strjoin);
	add_method(det, dex_intern("reverse"), &dm_strreverse);
	add_method(det, dex_intern("replace"), &dm_strreplace);
	add_method(det, dex_intern("uppercase"), &dm_struppercase);
	add_method(det, dex_intern("substr"), &dm_strsubstr);
	add_method(det, dex_intern("lowercase"), &dm_strlowercase);
	add_method(det, DISTR_CONCAT, &dm_strconcat);
}
DexType strType = make_dex_type(&add_str_methods, &create_strobj,
		&default_destroy, &dex_identity_func, "str", &str_equal, &str_hash);

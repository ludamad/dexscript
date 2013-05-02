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


#include "dexlist.h"
#include "util/hashutil.h"
#include <new>
#include "dexstr.h"
#include <cstring>
#include <algorithm>
#include "../stdobjs/math.h"
#include "../stdobjs/methods.h"
#include "../stdobjs/struct.h"
#include "../objdef/dex_conv.h"
#include "dexnum.h"


using namespace std;
static const DexRef FILE_S = create_string("dexlist.cpp");
static DexRef create_listobj(DexType* dext, DexRef* dobjs, size_t len) {
	if (len == 0)
		return EMPTY_LIST;
	if (len != 1)
		return NONE;
	if (dobjs->ref->type == &setType){
		return set_to_list(dobjs[0]);
	} else if (dobjs->ref->type == &listType){
		return dobjs[0];
	}
	//For other iterables
	DexRef ret =  list_from_seq(dobjs[0]);
	if (error()){
		stack_trace("list", FILE_S);
		return NONE;
	}
	return ret;
}

DexRef list_from_seq(const DexRef& dexseq){
	//For other iterables
	DexRef dl = create_list();
	int iterspace[4];
	DexRef it;
	bool more = dex_init_iter(it, dexseq, iterspace);
	while (more) {
		list_push(dl, it);
		more = dex_incr_iter(it, dexseq, iterspace);
	}
	return dl;
}

void list_sort(DexRef& l) {
	DexList* dl = get_dex_list(l.ref);
	if (l->refc > 1) {
		l = create_list(dl->data, dl->len);
		dl = get_dex_list(l.ref);
	}
	stable_sort(dl->data, dl->data + dl->len);
}

struct RevCompare {
	bool operator()(const DexRef& a, const DexRef& b) {
		return !(a < b);
	}
};
void list_rev_sort(DexRef& l) {
	DexList* dl = get_dex_list(l.ref);
	if (l->refc > 1) {
		l = create_list(dl->data, dl->len);
		dl = get_dex_list(l.ref);
	}
	stable_sort(dl->data, dl->data + dl->len, RevCompare());
}
struct KeyFCompare {
	DexRef keyf;
	inline bool operator()(DexRef a, DexRef b) {
		DexRef aa = dex_call(keyf, &a, 1);
		DexRef bb = dex_call(keyf, &b, 1);
		return aa < bb;
	}
};
//Uses keyf(a) < keyf(b) for comparisons
void list_sort_keyf(DexRef& l, DexRef keyf) {

	KeyFCompare c;
	c.keyf = keyf;
	DexList* dl = get_dex_list(l.ref);
	if (l->refc > 1) {
		l = create_list(dl->data, dl->len);
		dl = get_dex_list(l.ref);
	}
	stable_sort(dl->data, dl->data + dl->len, c);
}
struct CmpFCompare {
	DexRef cmpf;
	inline bool operator()(const DexRef& a, const DexRef& b) {
		DexRef args[] = { a, b };
		DexRef ret = dex_call(cmpf, args, 2);
		if (ret->type != &numType) {
			error(ARG_ERROR, "Expected function returning numeric values.");
			return false;
		}

		return get_dex_num(ret.ref) < 0;
	}
};
//Uses keyf(a) < keyf(b) for comparisons
void list_sort_cmpf(DexRef& l, DexRef cmpf) {
	CmpFCompare c;
	c.cmpf = cmpf;
	DexList* dl = get_dex_list(l.ref);
	if (l->refc > 1) {
		l = create_list(dl->data, dl->len);
		dl = get_dex_list(l.ref);
	}
	stable_sort(dl->data, dl->data + dl->len, c);
}
inline size_t list_alloc_len(size_t space) {
	return sizeof(DexObj) + sizeof(DexList) + space * sizeof(DexRef);
}
inline size_t list_maxlen(DexObj* d) {
	return (d->len - (sizeof(DexObj) + sizeof(DexList))) / sizeof(DexRef);
}
DexObj* create_list(size_t space) {
	DexObj* ret = alloc_dex_base(&listType, list_alloc_len(space));
	DexList* dl = get_dex_list(ret);
	dl->len = 0;
	return ret;
}
DexObj* create_list(DexRef* entries, size_t len, size_t buffspace) {
	size_t alen = list_alloc_len(len + buffspace);
	DexObj* l = create_list(alen);
	DexList* dl = get_dex_list(l);
	dl->len = len;
	memcpy(dl->data, entries, len * sizeof(DexRef));
	while (len--)
		dex_ref(entries++->ref);
	return l;
}
void list_push(DexRef& l, const DexRef& val) {
	DexList* dl = get_dex_list(l.ref);
	size_t n = dl->len;
	if (l->refc > 1 || n == list_maxlen(l.ref)) {
		l = create_list(dl->data, n, n);
		dl = get_dex_list(l.ref);
	}

	new (dl->data + n) DexRef(val.ref);
	dl->len++;
}
void list_pop(DexRef& l) {
	DexList* dl = get_dex_list(l.ref);
	if (dl->len < 1)
		return;
	if (l->refc > 1) {
		l = create_list(dl->data, dl->len - 1);
		return;
	}
	dl->len--;
	dl->data[dl->len].~DexRef();
}
void list_concat(DexRef& l, const DexRef& o) {
	DexList* l1 = get_dex_list(l.ref), *l2 = get_dex_list(o.ref);
	size_t s1 = l1->len, s2 = l2->len;
	size_t ns = s1 + s2;
	if (l->refc > 1 || l->len < list_alloc_len(ns)) {
		l = create_list(l1->data, s1, s1 + ns);
		l1 = get_dex_list(l.ref);
	}
	l1->len = ns;
	memcpy(l1->data + s1, l2->data, s2 * sizeof(DexRef));
	DexRef* p = l2->data;
	while (s2--)
		dex_ref(p++->ref);
}

DexRef list_get(const DexRef& l, size_t ind) {
	DexList* dl = get_dex_list(l.ref);
	if (dl->len <= ind) {
		error(DOMAIN_ERROR, "Not within list bounds.");
		return NONE;
	}
	return dl->data[ind];
}
DexRef* list_pget(DexRef& l, size_t ind) {
	DexList* dl = get_dex_list(l.ref);
	if (dl->len <= ind) {
		error(DOMAIN_ERROR, "Not within list bounds.");
		return NULL;
	}
	if (l->refc > 1) {
		l = create_list(dl->data, dl->len);
		dl = get_dex_list(l.ref);
	}
	return dl->data + ind;
}
void list_set(DexRef& l, size_t ind, const DexRef& d) {
	DexList* dl = get_dex_list(l.ref);
	if (l->refc > 1) {
		l = create_list(dl->data, dl->len);
		dl = get_dex_list(l.ref);
	}
	if (dl->len <= ind) {
		error(DOMAIN_ERROR, "Not within list bounds.");
		return;
	}
	dl->data[ind] = d;
}

void list_intern_all(DexRef& l) {
	DexList* dl = get_dex_list(l.ref);
	size_t n = dl->len;
	if (l->refc > 1) {
		l = create_list(dl->data, dl->len);
		dl = get_dex_list(l.ref);
	}
	DexRef* arr = dl->data;
	while (n > 0) {
		*arr = dex_intern(*arr);
		arr++;
		n--;
	}
}

void list_reverse(DexRef& l){

}
size_t list_hash(const DexRef& d) {
	DexList* l = get_dex_list(d.ref);
	return ordered_hash(l->data, l->len);
}

bool list_equal(const DexRef& l, const DexRef& l2) {
	DexList* d = get_dex_list(l.ref);
	DexList* d2 = get_dex_list(l2.ref);

	if (d->len != d2->len)
		return false;

	DexRef* s = d->data, *s2 = d2->data;
	DexRef* end = s + d->len;
	while (s < end) {
		if (!dex_equal(*s, *s2))
			return false;
		s++;
		s2++;
	}
	return true;
}
int list_index_of(const DexRef& d, const DexRef& val) {
	DexList* l = get_dex_list(d.ref);
	DexRef* s = l->data;
	DexRef* start = s;
	DexRef* end = start + l->len;
	while (s < end) {
		if (dex_equal(val, *s))
			return s - start;
		s++;
	}
	return -1;
}

void destroy_list(DexObj* d) {
	DexList* l = get_dex_list(d);
	DexRef* s = l->data;
	DexRef* end = s + l->len;
	while (s < end) {
		dex_unref(s->ref);
		s++;
	}
}
static DexRef list_str(const DexRef& d) {
	DexRef str = create_string("[", 1, 32);
	DexList* l = get_dex_list(d.ref);
	DexRef* s = l->data;
	DexRef* end = s + l->len;
	while (s < end) {
		DexRef ss = dex_to_string(*s);
		if (ss.ref == s->ref)
			str_append(str, '"');
		str_concat(str, ss);
		if (ss.ref == s->ref)
			str_append(str, '"');
		s++;
		if (s != end)
			strc_concat(str, ", ", 2);
	}
	str_append(str, ']');
	return str;
}
void list_clear(DexRef& l) {
	if (l->refc <= 1) {
		dex_clr(l.ref, sizeof(DexList));
	} else {
		l = create_list(list_maxlen(l.ref));
	}
}

bool valid_list_key(const DexRef& o, int& index) {
	return (o->type == &numType && is_int(get_dex_num(o.ref), index) && index
			>= 0);
}

static void dm_listhas(DexRef& obj, DexRef* args, size_t n) {
	if (n != 1)
		error(ARG_ERROR, ONEARGS);
	obj = (list_index_of(obj, args[0]) > -1) ? dex_true : dex_false;
}
static void dm_listfind(DexRef& obj, DexRef* args, size_t n) {
	if (n != 1)
		error(ARG_ERROR, ONEARGS);
	obj = list_index_of(obj, args[0]);
}
static void dm_listlen(DexRef& obj, DexRef* args, size_t n) {
	if (n != 0)
		error(ARG_ERROR, NOARGS);
	obj = list_size(obj);
}
static void dm_listpush(DexRef& obj, DexRef* args, size_t n) {
	if (n != 1)
		error(ARG_ERROR, ONEARGS);
	list_push(obj, args[0]);
}
static void dm_listsort(DexRef& obj, DexRef* args, size_t n) {
	if (n != 0)
		error(ARG_ERROR, NOARGS);
	list_sort(obj);
}
static void dm_listreverse(DexRef& obj, DexRef* args, size_t n) {
	if (n != 0)
		error(ARG_ERROR, NOARGS);
	list_reverse(obj);
}
static void dm_listpop(DexRef& obj, DexRef* args, size_t n) {
	if (n != 0)
		error(ARG_ERROR, NOARGS);
	list_pop(obj);
}

static void dm_listtop(DexRef& obj, DexRef* args, size_t n) {
	if (n != 0)
		error(ARG_ERROR, NOARGS);
	DexList* dl = get_dex_list(obj.ref);
	size_t len = dl->len;
	if (len == 0) {
		error(DOMAIN_ERROR, "Cannot take top of empty list.");
		return;
	}
	obj = dl->data[dl->len - 1];
}

static void add_list_methods(DexType* det) {
	add_std_methods(det);
	add_method(det, DISTR_LEN, &dm_listlen);
	add_method(det, DISTR_HAS, &dm_listhas);
	add_method(det, DISTR_FIND, &dm_listfind);
	add_method(det, DISTR_PUSH, &dm_listpush);
	add_method(det, DISTR_POP, &dm_listpop);
	add_method(det, DISTR_TOP, &dm_listtop);
	add_method(det, DISTR_SORT, &dm_listsort);
	add_method(det, DISTR_REVERSE, &dm_listreverse);
}
DexType listType = make_dex_type(&add_list_methods, &create_listobj,
		&destroy_list, &list_str, "list", &list_equal, &list_hash);

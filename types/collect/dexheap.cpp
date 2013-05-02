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

/*
#include "dexheap.h"
#include "util/hashutil.h"
#include <new>
#include "dexstr.h"
#include <cstring>
#include <algorithm>
#include "../stdobjs/math.h"
#include "../stdobjs/methods.h"
#include "../stdobjs/struct.h"
#include "dexnum.h"

using namespace std;
static const DexRef FILE_S = create_string("dexheap.cpp");
static DexRef create_heapobj(DexType* dext, DexRef* dobjs, size_t len) {
	if (len == 0)
		return EMPTY_LIST;
	if (len != 1)
		return NONE;
	if (dobjs->ref->type == &setType){
		return set_to_heap(dobjs[0]);
	} else if (dobjs->ref->type == &heapType){
		return dobjs[0];
	}
	//For other iterables
	DexRef ret =  heap_from_seq(dobjs[0]);
	if (error()){
		stack_log("heap", FILE_S);
		return NONE;
	}
	return ret;
}

DexRef heap_from_seq(const DexRef& dexseq){
	//For other iterables
	DexRef dl = create_heap();
	int iterspace[4];
	DexRef it;
	bool more = dex_init_iter(it, dexseq, iterspace);
	while (more) {
		heap_push(dl, it);
		more = dex_incr_iter(it, dexseq, iterspace);
	}
	return dl;
}

void heap_sort(DexRef& l) {
	DexList* dl = get_dex_heap(l.ref);
	if (l->refc > 1) {
		l = create_heap(dl->data, dl->len);
		dl = get_dex_heap(l.ref);
	}
	sort(dl->data, dl->data + dl->len);
}
struct KeyFCompare {
	DexRef keyf;
	bool operator()(DexRef a, DexRef b) {
		DexRef aa = dex_call(keyf, &a, 1);
		DexRef bb = dex_call(keyf, &b, 1);
		return aa < bb;
	}
};
//Uses keyf(a) < keyf(b) for comparisons
void heap_sort_keyf(DexRef& l, DexRef keyf) {

	KeyFCompare c;
	c.keyf = keyf;
	DexList* dl = get_dex_heap(l.ref);
	if (l->refc > 1) {
		l = create_heap(dl->data, dl->len);
		dl = get_dex_heap(l.ref);
	}
	sort(dl->data, dl->data + dl->len, c);
}
struct CmpFCompare {
	DexRef cmpf;
	bool operator()(const DexRef& a, const DexRef& b) {
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
void heap_sort_cmpf(DexRef& l, DexRef cmpf) {

	CmpFCompare c;
	c.cmpf = cmpf;
	DexList* dl = get_dex_heap(l.ref);
	if (l->refc > 1) {
		l = create_heap(dl->data, dl->len);
		dl = get_dex_heap(l.ref);
	}
	sort(dl->data, dl->data + dl->len, c);
}
inline size_t heap_alloc_len(size_t space) {
	return sizeof(DexObj) + sizeof(DexList) + space * sizeof(DexRef);
}
inline size_t heap_maxlen(DexObj* d) {
	return (d->len - (sizeof(DexObj) + sizeof(DexList))) / sizeof(DexRef);
}
DexObj* create_heap(size_t space) {
	DexObj* ret = alloc_dex_base(&heapType, heap_alloc_len(space));
	DexList* dl = get_dex_heap(ret);
	dl->len = 0;
	return ret;
}
DexObj* create_heap(DexRef* entries, size_t len, size_t buffspace) {
	size_t alen = heap_alloc_len(len + buffspace);
	DexObj* l = create_heap(alen);
	DexList* dl = get_dex_heap(l);
	dl->len = len;
	memcpy(dl->data, entries, len * sizeof(DexRef));
	while (len--)
		dex_ref(entries++->ref);
	return l;
}
void heap_push(DexRef& l, const DexRef& val) {
	DexList* dl = get_dex_heap(l.ref);
	size_t n = dl->len;
	if (l->refc > 1 || n == heap_maxlen(l.ref)) {
		l = create_heap(dl->data, n, n);
		dl = get_dex_heap(l.ref);
	}

	new (dl->data + n) DexRef(val.ref);
	dl->len++;
}
void heap_pop(DexRef& l) {
	DexList* dl = get_dex_heap(l.ref);
	if (dl->len < 1)
		return;
	if (l->refc > 1) {
		l = create_heap(dl->data, dl->len - 1);
		return;
	}
	dl->len--;
	dl->data[dl->len].~DexRef();
}
void heap_concat(DexRef& l, const DexRef& o) {
	DexList* l1 = get_dex_heap(l.ref), *l2 = get_dex_heap(o.ref);
	size_t s1 = l1->len, s2 = l2->len;
	size_t ns = s1 + s2;
	if (l->refc > 1 || l->len < heap_alloc_len(ns)) {
		l = create_heap(l1->data, s1, s1 + ns);
		l1 = get_dex_heap(l.ref);
	}
	l1->len = ns;
	memcpy(l1->data + s1, l2->data, s2 * sizeof(DexRef));
	DexRef* p = l2->data;
	while (s2--)
		dex_ref(p++->ref);
}

DexRef heap_get(const DexRef& l, size_t ind) {
	DexList* dl = get_dex_heap(l.ref);
	if (dl->len <= ind) {
		error(DOMAIN_ERROR, "Not within heap bounds.");
		return NONE;
	}
	return dl->data[ind];
}
DexRef* heap_pget(DexRef& l, size_t ind) {
	DexList* dl = get_dex_heap(l.ref);
	if (dl->len <= ind) {
		error(DOMAIN_ERROR, "Not within heap bounds.");
		return NULL;
	}
	if (l->refc > 1) {
		l = create_heap(dl->data, dl->len);
		dl = get_dex_heap(l.ref);
	}
	return dl->data + ind;
}
void heap_set(DexRef& l, size_t ind, const DexRef& d) {
	DexList* dl = get_dex_heap(l.ref);
	if (l->refc > 1) {
		l = create_heap(dl->data, dl->len);
		dl = get_dex_heap(l.ref);
	}
	if (dl->len <= ind) {
		error(DOMAIN_ERROR, "Not within heap bounds.");
		return;
	}
	dl->data[ind] = d;
}

void heap_intern_all(DexRef& l) {
	DexList* dl = get_dex_heap(l.ref);
	size_t n = dl->len;
	if (l->refc > 1) {
		l = create_heap(dl->data, dl->len);
		dl = get_dex_heap(l.ref);
	}
	DexRef* arr = dl->data;
	while (n > 0) {
		*arr = dex_intern(*arr);
		arr++;
		n--;
	}
}

void heap_reverse(DexRef& l){

}
size_t heap_hash(const DexRef& d) {
	DexList* l = get_dex_heap(d.ref);
	return ordered_hash(l->data, l->len);
}

bool heap_equal(const DexRef& l, const DexRef& l2) {
	DexList* d = get_dex_heap(l.ref);
	DexList* d2 = get_dex_heap(l2.ref);

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
int heap_index_of(const DexRef& d, const DexRef& val) {
	DexList* l = get_dex_heap(d.ref);
	DexRef* s = l->data, *start = s;
	DexRef* end = start + l->len;
	while (s < end) {
		if (dex_equal(val, *s))
			return s - start;
		s++;
	}
	return -1;
}

void destroy_heap(DexObj* d) {
	DexList* l = get_dex_heap(d);
	DexRef* s = l->data;
	DexRef* end = s + l->len;
	while (s < end) {
		s->~DexRef();
		s++;
	}
}
static DexRef heap_str(const DexRef& d) {
	DexRef str = create_string("[", 1, 32);
	DexList* l = get_dex_heap(d.ref);
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
void heap_clear(DexRef& l) {
	if (l->refc <= 1) {
		dex_clr(l.ref, sizeof(DexList));
	} else {
		l = create_heap(heap_maxlen(l.ref));
	}
}

bool valid_heap_key(const DexRef& o, int& index) {
	return (o->type == &numType && is_int(get_dex_num(o.ref), index) && index
			>= 0);
}

static void dm_heaphas(DexRef& obj, DexRef* args, size_t n) {
	if (n != 1)
		error(ARG_ERROR, ONEARGS);
	obj = (heap_index_of(obj, args[0]) == -1) ? dex_true : dex_false;
}
static void dm_heapfind(DexRef& obj, DexRef* args, size_t n) {
	if (n != 1)
		error(ARG_ERROR, ONEARGS);
	obj = heap_index_of(obj, args[0]);
}
static void dm_heaplen(DexRef& obj, DexRef* args, size_t n) {
	if (n != 0)
		error(ARG_ERROR, NOARGS);
	obj = heap_size(obj);
}
static void dm_heappush(DexRef& obj, DexRef* args, size_t n) {
	if (n != 1)
		error(ARG_ERROR, ONEARGS);
	heap_push(obj, args[0]);
}
static void dm_heapsort(DexRef& obj, DexRef* args, size_t n) {
	if (n != 0)
		error(ARG_ERROR, NOARGS);
	heap_sort(obj);
}
static void dm_heapreverse(DexRef& obj, DexRef* args, size_t n) {
	if (n != 0)
		error(ARG_ERROR, NOARGS);
	heap_reverse(obj);
}
static void dm_heappop(DexRef& obj, DexRef* args, size_t n) {
	if (n != 0)
		error(ARG_ERROR, NOARGS);
	heap_pop(obj);
}

static void dm_heaptop(DexRef& obj, DexRef* args, size_t n) {
	if (n != 0)
		error(ARG_ERROR, NOARGS);
	DexList* dl = get_dex_heap(obj.ref);
	size_t len = dl->len;
	if (len == 0) {
		error(DOMAIN_ERROR, "Cannot take top of empty heap.");
		return;
	}
	obj = dl->data[dl->len - 1];
}

static void add_heap_methods(DexType* det) {
	add_std_methods(det);
	add_method(det, DISTR_LEN, &dm_heaplen);
	add_method(det, DISTR_HAS, &dm_heaphas);
	add_method(det, DISTR_FIND, &dm_heapfind);
	add_method(det, DISTR_PUSH, &dm_heappush);
	add_method(det, DISTR_POP, &dm_heappop);
	add_method(det, DISTR_TOP, &dm_heaptop);
	add_method(det, DISTR_SORT, &dm_heapsort);
	add_method(det, DISTR_REVERSE, &dm_heapreverse);
}
DexType heapType = make_dex_type(&add_heap_methods, &create_heapobj,
		&destroy_heap, &heap_str, "heap", &heap_equal, &heap_hash);
*/

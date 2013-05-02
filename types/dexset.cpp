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

#include "dexset.h"
#include "../datastruct/settools.h"
#include "../mainh.h"
#include "../interp/error.h"
#include "util/hashutil.h"
#include "../stdobjs/methods.h"
#include "../stdobjs/struct.h"
#include "typetools.h"//Guarantee its loaded first
#include <algorithm>
#include "dexnum.h"
#include <iostream>
#include "../objdef/dex_conv.h"
using namespace std;

struct DexSet {
	size_t amount;
	DexRef data[0];
};
inline DexSet* get_dex_set(DexObj* dobj) {
	return (DexSet*) get_dex_memb(dobj, 0);
}
struct _SetT {//Helper class

	static bool isNull(const DexRef& e) {
		return e.ref == NONE;
	}

	static bool isRemoved(const DexRef& e) {
		return e.ref == TOMBSTONE;
	}

	static void remove(DexRef& e) {
		e = TOMBSTONE;
	}

	static bool equal(const DexRef& v1, const DexRef& v2) {
		//Num/String shortcut
		DexType* det = v1->type;
		if (det != v2->type)
			return false;
		if (det == &numType)
			return get_dex_num(v1.ref) == get_dex_num(v2.ref);
		if (det == &strType)
			return str_equal(v1, v2);

		return dex_equal(v1, v2);
	}

	static bool equal(const DexRef& v1, const DexCStr& v2) {
		if (v1->type != &strType)
			return false;
		return strc_equal(v1, v2.str, v2.len);
	}

	static bool equal(const DexRef& v1, const double& v2) {
		if (v1->type != &numType)
			return false;
		return get_dex_num(v1.ref) == v2;
	}

	static size_t hash(const DexRef& v) {
		//String/num shortcut
		if (v->type == &numType)
			return num_hash(v);
		if (v->type == &strType)
			return str_hash(v);
		return dex_hash(v);
	}

	static size_t hash(const DexCStr& v) {
		return strc_hash(v.str, v.len);
	}

	static size_t hash(const double& d) {
		return num_hash(d);
	}
};

static size_t inline set_alloc_len(size_t entries) {
	return sizeof(DexObj) + sizeof(DexSet) + entries * sizeof(DexRef);
}
static inline size_t get_set_buckets(DexObj* d) {
	return (d->len - set_alloc_len(0)) / sizeof(DexRef);
}
DexObj* create_set(DexRef* objs, size_t len) {
	DexObj* set = create_set(len * 4 / 3);
	DexSet* dd = get_dex_set(set);
	size_t bs = get_set_buckets(set);
	dd->amount = tset_add_allv<_SetT> (objs, len, dd->data, bs);
	return set;
}
static DexRef create_setobj(DexType* dext, DexRef* dobjs, size_t len) {
	if (len > 1) {
		error(ARG_ERROR, "'set' type takes 0-1 arguments.");
		return NONE;
	}
	DexRef ret = EMPTY_SET, key;
	size_t i[2];

	if (len == 0)
		return ret;

	if (dex_init_iter(key, dobjs[0], i)) {
		do {
			set_add(ret, key);
		} while (dex_incr_iter(key, dobjs[0], i));
	}
	return ret;
}
DexRef set_to_list(const DexRef& s){
	DexSet* dd = get_dex_set(s.ref);
	size_t iter = -1, i = 0;
	DexRef ret = create_list (dd->amount);
	DexRef key;
	DexList* dl = get_dex_list(ret.ref);
	while (set_incr_iter(key, s, &iter)){
		dl->data[i].ref = key.ref;
		dex_ref(key.ref);
		i++;
	}
	dl->len = dd->amount;
	return ret;
}

static void destroy_setobj(DexObj* d) {
	DexSet* dd = get_dex_set(d);
	size_t bb = get_set_buckets(d);
	DexRef* e = dd->data;
	for (size_t i = 0; i < bb; i++) {
		dex_unref(e[i].ref);
	}
}
size_t set_size(const DexRef& s) {
	return get_dex_set(s.ref)->amount;
}
static DexObj* set_resize(DexObj* d, size_t buckets) {
	DexObj* newset = create_set(buckets);
	DexSet* nd = get_dex_set(newset), *dd = get_dex_set(d);
	DexRef* e1 = dd->data, *e2 = nd->data;
	nd->amount += tset_add_all<_SetT> (e1, get_set_buckets(d), e2, buckets);
	return newset;
}
DexObj* create_set(size_t buckets) {
	
	if (buckets < 8)
		buckets = 8;
	else
		buckets = roundUpPower2(buckets - 1);
	DexObj* ret = alloc_dex_base(&setType, set_alloc_len(buckets));
	dex_init_clr(ret, sizeof(DexSet));
	return ret;
}
static DexObj* clone_set(DexObj* d) {
	DexObj* nd = alloc_dex_copy_base(d, d->len, d->len);
	DexSet* dd = get_dex_set(d);
	DexRef* s = dd->data, *e = (DexRef*) (d->len + (size_t) d);
	while (s < e) {
		dex_ref(s->ref);
		s++;
	}
	return nd;
}
std::ostream &operator<<(std::ostream &o, const DexCStr& val) {
	o << val.str;
	return o;
}
template<class V>
static DexRef* set_add_t(DexRef& s, const V& val) {
	DexSet* ds = get_dex_set(s.ref);
	size_t buckets = get_set_buckets(s.ref);
	if (tset_should_resize(ds->amount, buckets)) {
		buckets *= 2;
		s = set_resize(s.ref, buckets);
		ds = get_dex_set(s.ref);
	} else if (s->refc > 1) {
		s = clone_set(s.ref);
		ds = get_dex_set(s.ref);
	}
	DexRef* es = ds->data;
	bool added = tset_add<_SetT> (val, es, buckets);
	if (added)
		ds->amount++;
	return es;
}
void set_clear(DexRef& s) {
	if (s->refc <= 1) {
		dex_clr(s.ref, sizeof(DexSet));
	} else {
		s = create_set(get_set_buckets(s.ref));
	}
}
DexRef* set_add(DexRef& s, const DexRef& val) {
	return set_add_t(s, val);
}
DexRef* set_add_num(DexRef& s, const double& val) {
	return set_add_t(s, val);
}
DexRef* set_add_cstr(DexRef& s, const char* str, size_t len) {
	return set_add_t(s, DexCStr(str, len));
}
DexRef* set_find(const DexRef& s, const DexRef& v) {
	DexRef* dat = get_dex_set(s.ref)->data;
	return tset_find<_SetT> (v, dat, get_set_buckets(s.ref));
}
bool set_remove(DexRef& s, const DexRef& k) {
	DexSet* ds = get_dex_set(s.ref);
	size_t buckets = get_set_buckets(s.ref);
	if (s->refc > 1) {
		s = clone_set(s.ref);
		ds = get_dex_set(s.ref);
	}
	bool removed = tset_remove<_SetT> (k, ds->data, buckets);
	if (removed)
		ds->amount--;
	return removed;
}
void set_merge(DexRef& r1, const DexRef& r2) {
	DexSet* s = get_dex_set(r1.ref), *s2 = get_dex_set(r2.ref);
	size_t b1 = get_set_buckets(r1.ref), b2 = get_set_buckets(r2.ref);
	if (tset_should_resize(s->amount + s2-> amount, b1)) {
		b1 = roundUpPower2((b1 + b2) * 4 / 3);
		r1 = set_resize(r1.ref, b1);
		s = get_dex_set(r1.ref);
	}
	s->amount += tset_add_all<_SetT> (s2->data, b2, s->data, b1);
}
size_t set_hash(const DexRef& d) {
	DexSet* dd = get_dex_set(d.ref);
	size_t h = 1927868237L * (dd->amount + 1);
	DexRef* es = dd->data, *entry;
	size_t en = get_set_buckets(d.ref);
	for (size_t i = 0; i < en; i++) {
		entry = es + i;
		if (_SetT::isNull(*entry) || _SetT::isRemoved(*entry))
			continue;
		h ^= unordered_correction(_SetT::hash(*entry));
	}
	h = postprocess_unordered(h);
	return h;
}
bool set_equal(const DexRef& s, const DexRef& s2) {
	if (s.ref == s2.ref)
		return true;
	size_t n = get_set_buckets(s.ref);
	DexSet* ds = get_dex_set(s.ref), *ds2 = get_dex_set(s2.ref);

	if (ds->amount != ds2->amount)
		return false;

	DexRef* es = ds->data, *entry;
	for (size_t i = 0; i < n; i++) {
		entry = es + i;
		if (_SetT::isNull(*entry) || _SetT::isRemoved(*entry))
			continue;
		if (set_find(s2, *entry) == NULL)
			return false;
	}
	return true;
}
bool set_incr_iter(DexRef& key, const DexRef& s, size_t* i) {
	DexSet* dd = get_dex_set(s.ref);
	size_t b = get_set_buckets(s.ref);
	do {
		++*i;
		if (*i >= b)
			return false;
	} while (_SetT::isNull(dd->data[*i]) || _SetT::isRemoved(dd->data[*i]));
	key = dd->data[*i];
	return true;
}
static DexRef setstr(const DexRef& d) {
	DexSet* dd = get_dex_set(d.ref);
	size_t sizeguess = dd->amount * 4;
	DexRef rstr = create_string("{", 1, sizeguess);
	DexRef* es = dd->data, *entry;
	size_t en = get_set_buckets(d.ref);
	size_t visited = 0;
	for (size_t i = 0; i < en; i++) {
		entry = es + i;
		if (_SetT::isNull(*entry) || _SetT::isRemoved(*entry))
			continue;
		visited++;
		DexRef ks = dex_to_string(*entry);
		if (ks.ref == entry->ref)
			str_append(rstr, '"');
		str_concat(rstr, ks);
		if (ks.ref == entry->ref)
			str_append(rstr, '"');
		if (visited != dd->amount)
			strc_concat(rstr, ", ", 2);
	}
	str_append(rstr, '}');
	return rstr;
}

static void dm_setlen(DexRef& obj, DexRef* args, size_t n) {
	if (n != 0)
		error(ARG_ERROR, NOARGS);
	obj = set_size(obj);
}

static void dm_setremove(DexRef& obj, DexRef* args, size_t n) {
	if (n != 1)
		error(ARG_ERROR, ONEARGS);
	set_remove(obj, args[0]);
}
static void dm_setmerge(DexRef& obj, DexRef* args, size_t n) {
	if (n != 1)
		error(ARG_ERROR, ONEARGS);
	if (args[0]->type == &setType)
		set_merge(obj, args[0]);
	else {
		int iterspace[4];
		DexRef it;
		bool more = dex_init_iter(it, args[0], iterspace);
		while (more) {
			set_add(obj, it);
			more = dex_incr_iter(it, args[0], iterspace);
		}
	}
}

static void dm_setput(DexRef& obj, DexRef* args, size_t n) {
	if (n != 1)
		error(ARG_ERROR, ONEARGS);
	set_add(obj, args[0]);
}

static void dm_sethas(DexRef& obj, DexRef* args, size_t n) {
	if (n != 1)
		error(ARG_ERROR, ONEARGS);
	obj = set_find(obj, args[0]) ? dex_true : dex_false;
}
static void dm_sethasall(DexRef& obj, DexRef* args, size_t n) {
	if (n != 1)
		error(ARG_ERROR, ONEARGS);
	if (args[0]->type == &listType){
		DexList* dl = get_dex_list(args[0].ref);
		for (int i = 0; i < dl->len; i++)
			if (!set_find(obj, dl->data[i])){
				obj = dex_false;
				return;
			}
	} else {
		int iterspace[4];
		DexRef it;
		bool more = dex_init_iter(it, args[0], iterspace);
		while (more) {
			if (!set_find(obj, it)){
				obj = dex_false;
				return;
			}
			more = dex_incr_iter(it, args[0], iterspace);
		}
	}
	if (error())
		obj = NONE;
	else
		obj = dex_true;
}
static void dm_setremoveall(DexRef& obj, DexRef* args, size_t n) {
	if (n != 1)
		error(ARG_ERROR, ONEARGS);
	if (args[0]->type == &listType){
		DexList* dl = get_dex_list(args[0].ref);
		for (int i = 0; i < dl->len; i++)
			set_remove(obj, dl->data[i]);
	} else {
		int iterspace[4];
		DexRef it;
		bool more = dex_init_iter(it, args[0], iterspace);
		while (more) {
			set_remove(obj, it);
			more = dex_incr_iter(it, args[0], iterspace);
		}
	}
	if (error())
		obj = NONE;
}

void add_set_methods(DexType* det) {
	add_std_methods(det);
	add_method(det, DISTR_LEN, &dm_setlen);
	add_method(det, DISTR_REMOVE, &dm_setremove);
	add_method(det, dex_intern("remove_all"), &dm_setremoveall);
	add_method(det, DISTR_HAS, &dm_sethas);
	add_method(det, dex_intern("has_all"), &dm_sethasall);
	add_method(det, DISTR_PUT, &dm_setput);
	add_method(det, dex_intern("put_all"), &dm_setmerge);
}
DexType setType = make_dex_type(&add_set_methods, &create_setobj,
		&destroy_setobj, &setstr, "set", &set_equal, &set_hash);

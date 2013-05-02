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

#include "dexdict.h"
#include <algorithm>
#include "../datastruct/dicttools.h"
#include "../mainh.h"
#include "typetools.h"//Guarantee its loaded first
#include <algorithm>
#include <iostream>
#include "../interp/error.h"
#include "dexstdops.h"
#include "../stdobjs/methods.h"
#include "../stdobjs/struct.h"
#include "util/hashutil.h"

static const DexRef FILE_S = create_string("dexdict.cpp");

struct DexDict {
	size_t amount;
	DexRef data[0];
};
inline DexDict* get_dex_dict(DexObj* dobj) {
	return (DexDict*) get_dex_memb(dobj, 0);
}

struct _DictT {//Helper class
	typedef DexRef K;
	typedef DexRef V;
	typedef pair<K, V> E;
	static bool isNull(E* e) {
		return e->first.ref == NONE;
	}
	static bool isRemoved(E* e) {
		return e->first.ref == TOMBSTONE;
	}
	static void remove(E* e) {
		e->first = TOMBSTONE;
		e->second = NONE;
	}
	static bool equal(const K& k1, const K& k2) {
		//String shortcut
		if (k1->type == &strType && k2->type == &strType) {
			return str_equal(k1, k2);
		}
		return dex_equal(k1, k2);
	}
	static size_t hash(const K& k1) {
		return hash_key(k1);
	}
};

typedef _DictT::E Dpair;
static size_t inline dict_alloc_len(size_t entries) {
	return sizeof(DexObj) + sizeof(DexDict) + entries * sizeof(_DictT::E);
}
static inline size_t get_dict_buckets(DexObj* d) {
	return (d->len - dict_alloc_len(0)) / sizeof(_DictT::E);
}
DexObj* create_dict(Dpair* objs, size_t len) {
	DexObj* dict = create_dict(len * 4 / 3);
	DexDict* dd = get_dex_dict(dict);
	size_t bs = get_dict_buckets(dict);
	dd->amount = tdict_put_allv<_DictT> (objs, len, (Dpair*) dd->data, bs);
	return dict;
}
static DexRef create_dictobj(DexType* dext, DexRef* dobjs, size_t len) {
	if (len > 2) {
		error(ARG_ERROR, "'dict' type takes 0-2 arguments.");
		return NONE;
	}

	DexRef ret = EMPTY_DICT;
	DexRef key, val;
	size_t i[2];
	bool baditer = false;

	if (len == 1) {
		DexRef col = dobjs[0];
		bool is = dex_init_iter(key, col, i);
		if (is) {
			is = dex_incr_iter(val, col, i);
			if (is)
				for (;;) {
					dict_put(ret, key, val);
					is = dex_incr_iter(key, col, i);
					if (!is)
						break;
					is = dex_incr_iter(val, col, i);
					if (!is) {
						baditer = true;
						break;
					}
				}
		} else
			baditer = true;
	} else if (len == 2) {
		DexRef col1 = dobjs[0], col2 = dobjs[1];
		bool is1 = dex_init_iter(key, col1, i);
		bool is2 = dex_init_iter(val, col2, i + 1);
		if (is1 != is2)
			baditer = true;
		else {
			if (is1)
				for (;;) {
					dict_put(ret, key, val);
					is1 = dex_incr_iter(key, col1, i);
					is2 = dex_incr_iter(val, col2, i + 1);
					if (is1 != is2) {
						baditer = true;
						break;
					}
					if (!is1)
						break;
				}

		}
	}
	if (baditer)
		error(DOMAIN_ERROR, "Uneven number of keys and values.");
	if (error()) {
		stack_trace("dict", FILE_S);
		return NONE;
	}
	return ret;
}

static void destroy_dictobj(DexObj* d) {
	DexDict* dd = get_dex_dict(d);
	size_t bb = get_dict_buckets(d) * 2;
	DexRef* e = dd->data;
	for (size_t i = 0; i < bb; i++) {
		dex_unref(e[i].ref);
	}
}

static DexObj* dict_resize(DexObj* d, size_t buckets) {
	DexObj* newdict = create_dict(buckets);
	DexDict* nd = get_dex_dict(newdict), *dd = get_dex_dict(d);
	Dpair* e1 = (Dpair*) dd->data, *e2 = (Dpair*) nd->data;
	nd->amount += tdict_put_all<_DictT> (e1, get_dict_buckets(d), e2, buckets);
	return newdict;
}
DexObj* create_dict(size_t buckets) {
	if (buckets < 8)
		buckets = 8;
	buckets = roundUpPower2(buckets - 1);
	DexObj* ret = alloc_dex_base(&dictType, dict_alloc_len(buckets));
	dex_init_clr(ret, sizeof(DexDict));
	return ret;
}
static DexObj* clone_dict(DexObj* d) {
	DexObj* nd = alloc_dex_copy_base(d, d->len, d->len);
	DexDict* dd = get_dex_dict(d);
	DexRef* s = dd->data, *e = (DexRef*) (d->len + (size_t) d);
	while (s < e) {
		dex_ref(s->ref);
		s++;
	}
	return nd;
}
void dict_put(DexRef& d1, const DexRef& k, const DexRef& v) {
	DexDict* dd = get_dex_dict(d1.ref);
	size_t buckets = get_dict_buckets(d1.ref);
	if (tdict_should_resize(dd->amount, buckets)) {
		buckets *= 2;
		d1 = dict_resize(d1.ref, buckets);
		dd = get_dex_dict(d1.ref);
	} else if (d1->refc > 1) {
		d1 = clone_dict(d1.ref);
		dd = get_dex_dict(d1.ref);
	}
	Dpair* es = (Dpair*) dd->data;
	if (tdict_put<_DictT> (k, v, es, buckets))
		dd->amount++;
}
void dict_intern_keys(DexRef& d1) {
	DexDict* dd = get_dex_dict(d1.ref);
	size_t buckets = get_dict_buckets(d1.ref);
	if (d1->refc > 1) {
		//Theoretically we could get away with just mutating
		//Remove this later maybe
		d1 = clone_dict(d1.ref);
		dd = get_dex_dict(d1.ref);
	}
	Dpair* es = (Dpair*) dd->data;
	while (buckets > 0) {
		if (!_DictT::isNull(es) && !_DictT::isRemoved(es)) {
			es->first = dex_intern(es->first);
		}
		buckets--;
		es++;
	}
}
bool dict_remove(DexRef& d1, const DexRef& k) {
	DexDict* dd = get_dex_dict(d1.ref);
	size_t buckets = get_dict_buckets(d1.ref);
	if (d1->refc > 1) {
		d1 = clone_dict(d1.ref);
		dd = get_dex_dict(d1.ref);
	}
	Dpair* es = (Dpair*) dd->data;
	Dpair* loc = tdict_get<_DictT> (k, es, buckets);
	if (loc) {
		_DictT::remove(loc);
		dd->amount--;
		return true;
	}
	return false;
}

bool dict_has_key(const DexRef& d1, const DexRef& k) {
	Dpair* entries = (Dpair*) get_dex_dict(d1.ref)->data;
	return tdict_get<_DictT> (k, entries, get_dict_buckets(d1.ref)) != NULL;
}
DexRef dict_get(const DexRef& d1, const DexRef& k) {
	Dpair* entries = (Dpair*) get_dex_dict(d1.ref)->data;
	Dpair* ret = tdict_get<_DictT> (k, entries, get_dict_buckets(d1.ref));
	if (ret == NULL)
		return NONE;
	return ret->second;
}
DexRef* dict_pget(DexRef& d1, const DexRef& k) {
	DexDict* dd = get_dex_dict(d1.ref);
	size_t buckets = get_dict_buckets(d1.ref);
	if (tdict_should_resize(dd->amount, buckets)) {
		buckets *= 2;
		d1 = dict_resize(d1.ref, buckets);
		dd = get_dex_dict(d1.ref);
	} else if (d1->refc > 1) {
		d1 = clone_dict(d1.ref);
		dd = get_dex_dict(d1.ref);
	}

	Dpair * entry = (Dpair*) dd->data;
	if (tdict_pget<_DictT> (k, entry, get_dict_buckets(d1.ref)))
		dd->amount++;
	if (error()) {
		stack_trace("dict.put", FILE_S);
		return NULL;
	}
	return &entry->second;
}
void dict_merge(DexRef& r1, const DexRef& r2) {
	DexDict* d1 = get_dex_dict(r1.ref), *d2 = get_dex_dict(r2.ref);
	size_t b1 = get_dict_buckets(r1.ref), b2 = get_dict_buckets(r2.ref);
	if (tdict_should_resize(d1->amount + d2-> amount, b1)) {
		b1 = roundUpPower2((b1 + b2) * 4 / 3);
		r1 = dict_resize(r1.ref, b1);
		d1 = get_dex_dict(r1.ref);
	}
	Dpair* e1 = (Dpair*) d1->data, *e2 = (Dpair*) d2->data;
	d1->amount += tdict_put_all<_DictT> (e2, b2, e1, b1);
}

size_t dict_hash_entries(const DexRef& d) {
	size_t h = 0;
	DexDict* dd = get_dex_dict(d.ref);
	Dpair* es = (Dpair*) dd->data, *entry;
	size_t en = get_dict_buckets(d.ref);
	for (size_t i = 0; i < en; i++) {
		entry = es + i;
		if (_DictT::isNull(entry) || _DictT::isRemoved(entry))
			continue;
		h ^= hash_keyval(entry->first, entry->second);
	}
	return h;
}
size_t dict_hash(const DexRef& d) {
	DexDict* dd = get_dex_dict(d.ref);
	size_t h = 1927868237L * (dd->amount + 1);
	Dpair* es = (Dpair*) dd->data, *entry;
	size_t en = get_dict_buckets(d.ref);
	for (size_t i = 0; i < en; i++) {
		entry = es + i;
		if (_DictT::isNull(entry) || _DictT::isRemoved(entry))
			continue;
		h ^= hash_keyval(entry->first, entry->second);
	}
	h = postprocess_unordered(h);
	return h;
}
bool dict_equal(const DexRef& d1, const DexRef& d2) {
	if (d1.ref == d2.ref)
		return true;
	size_t s = get_dict_buckets(d1.ref);
	DexDict* dd = get_dex_dict(d1.ref), *dd2 = get_dex_dict(d2.ref);

	if (dd->amount != dd2->amount)
		return false;

	Dpair* es = (Dpair*) dd->data, *entry;
	for (size_t i = 0; i < s; i++) {
		entry = es + i;
		if (_DictT::isNull(entry) || _DictT::isRemoved(entry))
			continue;
		if (!dex_equal(entry->second, dict_get(d2, entry->first))) {
			return false;
		}
	}
	return true;
}
bool dict_incr_iter(DexRef& key, const DexRef& d1, size_t* i) {
	DexDict* dd = get_dex_dict(d1.ref);
	size_t b = get_dict_buckets(d1.ref);
	do {
		++*i;
		if (*i >= b)
			return false;
	} while (_DictT::isNull(*i + (Dpair*) dd->data) || _DictT::isRemoved(*i
			+ (Dpair*) dd->data));
	key = *((*i) * 2 + dd->data);
	return true;
}
static DexRef dictstr(const DexRef& d) {
	DexDict* dd = get_dex_dict(d.ref);
	size_t sizeguess = dd->amount * 6 + 4;
	DexRef rstr = create_string("{", 1, sizeguess);
	Dpair* es = (Dpair*) dd->data, *entry;
	size_t en = get_dict_buckets(d.ref);
	size_t visited = 0, amnt = dd->amount;
	for (size_t i = 0; i < en; i++) {
		entry = es + i;
		if (_DictT::isNull(entry) || _DictT::isRemoved(entry))
			continue;
		visited++;
		DexRef ks = dex_to_string(entry->first), vs = dex_to_string(
				entry->second);
		if (ks.ref == entry->first.ref)
			str_append(rstr, '"');
		str_concat(rstr, ks);
		if (ks.ref == entry->first.ref)
			str_append(rstr, '"');
		strc_concat(rstr, " => ", 4);
		if (vs.ref == entry->second.ref)
			str_append(rstr, '"');
		str_concat(rstr, vs);
		if (vs.ref == entry->second.ref)
			str_append(rstr, '"');
		if (visited != amnt)
			strc_concat(rstr, ", ", 2);
	}
	if (amnt == 0)
		strc_concat(rstr, "=>", 2);
	str_append(rstr, '}');
	return rstr;
}
void dict_clear(DexRef & d1) {
	if (d1->refc <= 1) {
		dex_clr(d1.ref, sizeof(DexDict));
	} else {
		d1 = create_dict(get_dict_buckets(d1.ref));
	}
}

size_t dict_size(const DexRef& d) {
	return get_dex_dict(d.ref)->amount;
}

DexRef* dict_raw_array(const DexRef& d) {
	return get_dex_dict(d.ref)->data;
}
DexRef* dict_next_entry(const DexRef& d1, DexRef* iter) {
	DexRef* dend = (DexRef*) (d1->len + (char*) d1.ref);
	do {
		iter += 2;
		if (iter >= dend)
			return NULL;
	} while (_DictT::isNull((Dpair*) iter) || _DictT::isRemoved((Dpair*) iter));
	return iter;
}
static void dm_dictlen(DexRef& obj, DexRef* args, size_t n) {
	if (n != 0)
		error(ARG_ERROR, NOARGS);
	obj = dict_size(obj);
}

static void dm_dictremove(DexRef& obj, DexRef* args, size_t n) {
	if (n != 1)
		error(ARG_ERROR, ONEARGS);
	dict_remove(obj, args[0]);
}

static void dm_dicthas(DexRef& obj, DexRef* args, size_t n) {
	if (n != 1)
		error(ARG_ERROR, ONEARGS);
	obj = dict_has_key(obj, args[0]) ? dex_true : dex_false;
}

void add_dict_methods(DexType* det) {
	add_std_methods(det);
	add_method(det, DISTR_LEN, &dm_dictlen);
	add_method(det, DISTR_REMOVE, &dm_dictremove);
	add_method(det, DISTR_HAS, &dm_dicthas);
}

DexType dictType = make_dex_type(&add_dict_methods, &create_dictobj,
		&destroy_dictobj, &dictstr, "dict", &dict_equal, &dict_hash);

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

#include "VoidDict.h"
#include <cstdlib>
#include <cstring>
#include "../../datastruct/dicttools.h"
using namespace std;

struct _VoidDictT {//Helper class
	typedef void* K;
	typedef void* V;
	typedef pair<K, V> E;
	static inline bool isNull(E* e) {
		return e->first == NULL;
	}
	//Removal not included
	static inline bool isRemoved(E* e) {
		return false;
	}

	static inline bool equal(const K& k1, const K& k2) {
		return k1 == k2;
	}

	static inline size_t hash(const K& k1) {
		return size_t(k1);
	}
};

using namespace std;
typedef _VoidDictT::E Vpair;
void VoidDict::__init(int capacity) {
	size_t bytes = sizeof(Vpair) * capacity;
	data = (T*) malloc(bytes);
	memset(data, 0, bytes);
	len = capacity;
	usage = 0;

}
VoidDict::VoidDict() {
	__init(32);
}
VoidDict::VoidDict(int capacity) {
	__init(capacity);
}

VoidDict::VoidDict(const VoidDict& o){
	len = o.len;
	size_t bytes = sizeof(Vpair) * len;
	usage = o.usage;
	data = (T*) malloc(bytes);
	memcpy(data, o.data, bytes);
}

void VoidDict::resize() {
	size_t bytes = sizeof(Vpair) * len * 2;
	T* newd = (T*) malloc(bytes);

	memset(newd, 0, bytes);

	tdict_put_all<_VoidDictT> ((Vpair*) data, len, (Vpair*) newd, len * 2);
	free(data);
	data = newd;
	len *= 2;
}

VoidDict::~VoidDict() {
	free(data);
}

void* VoidDict::get(void* k) {
	Vpair* ret = tdict_get<_VoidDictT> (k, (Vpair*) data, len);
	if (ret == NULL)
		return NULL;
	return ret->second;
}
void** VoidDict::pget(void* k) {
	if (tdict_should_resize(usage, len))
		resize();
	Vpair* entry = (Vpair*) data;
	if (tdict_pget<_VoidDictT> (k, entry, len))
		usage++;
	return &entry->second;
}
bool VoidDict::has_key(void* k) {
	return tdict_get<_VoidDictT> (k, (Vpair*) data, len) != NULL;
}

void VoidDict::put(void* k, void* v) {
	if (tdict_should_resize(usage, len))
		resize();
	if (tdict_put<_VoidDictT> (k, v, (Vpair*) data, len))
		usage++;
}

void VoidDict::merge(const VoidDict & vd) {
	while (tdict_should_resize(usage + vd.usage, len))
		resize();
	usage += tdict_put_all<_VoidDictT> ((Vpair*) vd.data, vd.len,
			(Vpair*) data, len);
}


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

#ifndef DICTTOOLS_H_
#define DICTTOOLS_H_
#include <algorithm>
#include <cstdlib>

using namespace std;

inline size_t tdictNextIndex(size_t s) {
	return s + 1;
}

//Returns the entry where we would find an object
//or where we would place it
template<class D, class K, class V>
bool tdict_pget(const K& key, std::pair<K, V>*& data, size_t entryn) {
	std::pair<K, V>* start = data;
	//Don't handle resizing, assume we have space
	//Find the index corresponding to the hash code
	size_t index = D::hash(key) & (entryn - 1);
	pair<K, V>* tomb;
	for (;;) {
		pair<K, V>* entry = start + index;
		if (D::isRemoved(entry)) {
			tomb = entry;
			goto TombLoop;
		}
		if (D::isNull(entry)) {
			data = entry;
			data->first = key;
			return true;
		}
		if (D::equal(key, entry->first)) {
			data = entry;
			return false;
		}
		//Wrap if past end
		index = tdictNextIndex(index) & (entryn - 1);
	}
	//Tomb has been found...
	//Enter another loop
	TombLoop: for (;;) {
		index = tdictNextIndex(index) & (entryn - 1);
		pair<K, V>* entry = data + index;
		if (D::isNull(entry)) {
			data = tomb;
			data->first = key;
			return true;
		} else if (D::equal(key, entry->first)) {
			data = entry;
			return false;
		}
	}
}

//Return if we added a new entry
template<class D, class K, class V>
bool tdict_put(const K& key, const V& val, std::pair<K, V>* data, size_t entryn) {
	//Don't handle resizing, assume we have space
	//Find the index corresponding to the hash code
	size_t index = D::hash(key) & (entryn - 1);
	pair<K, V>* tomb;
	for (;;) {
		pair<K, V>* entry = data + index;
		if (D::isNull(entry)) {
			*entry = std::pair<K, V>(key, val);
			return true;
		} else if (D::isRemoved(entry)) {
			tomb = entry;
			goto TombLoop;
		} else if (D::equal(key, entry->first)) {
			entry->second = val;
			return false;
		}
		//Wrap if past end
		index = tdictNextIndex(index) & (entryn - 1);
	}
	//Tomb has been found...
	//Enter another loop
	TombLoop: for (;;) {
		index = tdictNextIndex(index) & (entryn - 1);
		pair<K, V>* entry = data + index;
		if (D::isNull(entry)) {
			*tomb = std::pair<K, V>(key, val);
			return true;
		} else if (D::equal(key, entry->first)) {
			entry->second = val;
			return false;
		}
	}
}

template<class D, class K, class V>
std::pair<K, V>* tdict_get(const K& key, std::pair<K, V>* data, size_t entryn) {
	//Find the index corresponding to the hash code
	size_t start = D::hash(key) & (entryn - 1);
	size_t index = start;
	do {
		pair<K, V>* entry = data + index;
		if (D::isNull(entry))
			return NULL;
		if (D::equal(key, entry->first))
			return entry;
		//Wrap if past end
		index = tdictNextIndex(index) & (entryn - 1);
	} while (index != start);
	return NULL;
}
inline bool tdict_should_resize(size_t amnt, size_t entryn) {
	return amnt * 4 >= entryn * 3;
}
template<class D, class K, class V>
bool tdict_remove(const K& key, std::pair<K, V>* data, size_t entryn) {
	std::pair<K, V>* rptr = tdict_get<D> (key, data, entryn);
	if (rptr) {
		D::remove(rptr);

		return true;
	} else
		return false;
}

//Return amount of new entries in dst
template<class D, class K, class V>
size_t tdict_put_all(const std::pair<K, V>* src, size_t srcn,
		std::pair<K, V>* dst, size_t dstn) {
	const std::pair<K, V>* end = src + srcn;
	size_t newn = 0;
	for (; src < end; src++) {
		if (!D::isNull((std::pair<K, V>*) src) && !D::isRemoved(
				(std::pair<K, V>*) src)) {
			if (tdict_put<D> (src->first, src->second, dst, dstn))
				newn++;
		}
	}
	return newn;
}

//Return amount of new entries in dst
template<class D, class K, class V>
size_t tdict_put_allv(const std::pair<K, V>* src, size_t srcn,
		std::pair<K, V>* dst, size_t dstn) {
	const std::pair<K, V>* end = src + srcn;
	size_t newn = 0;
	for (; src < end; src++) {
		if (tdict_put<D> (src->first, src->second, dst, dstn))
			newn++;
	}
	return newn;
}
#endif /* DICTTOOLS_H_ */

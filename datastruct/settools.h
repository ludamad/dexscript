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

#ifndef SETTOOLS_H_
#define SETTOOLS_H_

inline size_t tsetNextIndex(size_t s) {
	return s + 1;
}

template<class D, class V, class K>
bool tset_add(const K& val, V*& data, size_t entryn) {
	size_t start = D::hash(val) & (entryn - 1);
	size_t index = start;
	V* tomb;
	for (;;) {
		V* entry = data + index;
		if (D::isNull(*entry)) {
			*entry = val;
			data = entry;
			return true;
		}
		if (D::isRemoved(*entry)) {
			tomb = entry;
			goto TombLoop;
		}
		if (D::equal(*entry, val)) {
			data = entry;
			return false;
		}
		//Wrap if past end
		index = tsetNextIndex(index) & (entryn - 1);
	}
	TombLoop: for (;;) {
		//Wrap if past end
		index = tsetNextIndex(index) & (entryn - 1);
		V* entry = data + index;
		if (D::isNull(*entry)) {
			*tomb = val;
			data = tomb;
			return true;
		}
		if (D::equal(*entry, val)) {
			data = entry;
			return false;
		}
	}
}
template<class D, class V, class K>
int tset_add_all(K* src, size_t srcn, V* dst, size_t dstn) {
	V* end = src + srcn;
	size_t newn = 0;
	for (; src < end; src++) {
		if (!D::isNull(*src) && !D::isRemoved(*src)) {
			V* tdst = dst;
			if (tset_add<D> (*src, tdst, dstn))
				newn++;
		}
	}
	return newn;
}

template<class D, class V, class K>
int tset_add_allv(K* src, size_t srcn, V* dst, size_t dstn) {
	V* end = src + srcn;
	size_t newn = 0;
	for (; src < end; src++) {
		V* tdst = dst;
		if (tset_add<D> (*src, tdst, dstn))
			newn++;
	}
	return newn;
}

template<class D, class V, class K>
V* tset_find(const K& key, V* data, size_t entryn) {
	size_t start = D::hash(key) & (entryn - 1);
	size_t index = start;
	for (;;) {
		V* entry = data + index;
		if (D::isNull(*entry)) {
			return NULL;
		} else if (D::equal(*entry, key)) {
			return entry;
		}
		//Wrap if past end
		index = tsetNextIndex(index) & (entryn - 1);
	}
}
inline bool tset_should_resize(size_t amnt, size_t entryn) {
	return amnt * 4 >= entryn * 3;
}
template<class D, class V, class K>
bool tset_remove(const K& key, V* data, size_t entryn) {
	V* rptr = tset_find<D> (key, data, entryn);
	if (rptr) {
		D::remove(*rptr);
		return true;
	} else
		return false;
}

#endif /* SETTOOLS_H_ */

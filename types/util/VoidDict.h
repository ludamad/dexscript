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

#ifndef VOIDDICT_H_
#define VOIDDICT_H_
#include <cstdlib>

//Meant for fast retrieval of ptr/int objects only
//No tombstone elements, no ref counting
//Can use interned/unique objects as key
struct VoidDict {
	typedef void* T;
	size_t len, usage;
	T* data;
	void __init(int capacity);

	VoidDict(int capacity);
	VoidDict(const VoidDict& o);
	VoidDict();
	~VoidDict();

	void put(T k, T v);
	void resize();
	T get(T k);
	T* pget(T k);
	bool has_key(T k);
	void merge(const VoidDict& vd);

};

#endif /* VOIDDICT_H_ */

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

#ifndef DICTTEST_H_
#define DICTTEST_H_
#include "dicttools.h"
#include "../mainh.h"
struct DTest {
	typedef size_t K;
	typedef size_t V;
	typedef pair<K,V> E;
	static bool isNull(E* e){
		return e->first == 0;
	}
	static bool isRemoved(E* e){
		return e->first == 1;
	}
	static void remove(E* e){
		e->first = 1;
	}
	static bool equal(const K& k1, const K& k2){
		return k1 == k2;
	}
	static size_t hash(const K& k1){
		return k1;
	}
};

#endif /* DICTTEST_H_ */

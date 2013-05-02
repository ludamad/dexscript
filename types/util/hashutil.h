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

#ifndef HASHUTIL_H_
#define HASHUTIL_H_
#include "../dexstdops.h"
#include "../dexstr.h"

//Many small details borrowed from python 3.2 source...
inline size_t ordered_hash(DexRef* args, size_t n) {

    size_t x = 0x345678L;
    size_t mult = 1000003L;
    while (n > 0) {
        x = (x ^ dex_hash(*args)) * mult;

        mult += (size_t)(82520L + n + n);
        args++;
        n--;
    }
    x += 97531L;
    return x;
}
inline size_t ordered_hash(size_t* s, size_t* e) {

    size_t x = 0x345678L;
    size_t mult = 1000003L;
    while (s < e) {
        x = (x ^ *s) * mult;

        mult += (size_t)(82520L + e - s);
        s++;
    }
    return x;
}

/*From python 3.2 setobject.c:
  Work to increase the bit dispersion for closely spaced hash
   values.  The is important because some use cases have many
   combinations of a small number of elements with nearby
   hashes so that many distinct combinations collapse to only
   a handful of distinct hash values. */
inline size_t unordered_correction(size_t h){
	return (h ^ (h << 16) ^ 89869747L)  * 3644798167u;
}
inline size_t postprocess_unordered(size_t h){
	return h * 69069L + 907133923L;
}
inline size_t hash_key(const DexRef& k1){
	//String shortcut
	if (k1->type == &strType)
		return str_hash(k1);
	return dex_hash(k1);
}
inline size_t hash_keyval(const DexRef& key, const DexRef& val){
	size_t x = (dex_hash(key)) * 1000003L;
    x = (x ^ dex_hash(val)) * 1825204L;
    return unordered_correction(x);
}
/*
inline size_t cset_hash(DexRef* args, size_t n){
    PySetObject *so = (PySetObject *)self;
    Py_hash_t h, hash = 1927868237L;
    setentry *entry;
    Py_ssize_t pos = 0;

    if (so->hash != -1)
        return so->hash;

    hash *= PySet_GET_SIZE(self) + 1;
    while (set_next(so, &pos, &entry)) {
        h = entry->hash;
        hash ^= (h ^ (h << 16) ^ 89869747L)  * 3644798167u;
    }
    hash = hash * 69069L + 907133923L;
    if (hash == -1)
        hash = 590923713L;
    so->hash = hash;
    return hash;
}
*/
#endif /* HASHUTIL_H_ */

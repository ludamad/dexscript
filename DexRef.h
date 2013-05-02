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

#ifndef DEXREF_H_
#define DEXREF_H_
#include "types/dexobj.h"

#define DEX_DONT_INIT ((void*)NULL)

/* The main interface to any object defined for DexScript.
 * This class keeps track of the ref count of objects and
 * is an improvement compared to the raw DexObj* type for
 * object storage.
 * Various workarounds are provided to work with custom
 * allocated memory (eg using DexRef with malloc/free)
 */
struct DexRef {
	//The internal object pointer
	DexObj* ref;

	/* Initialize an empty DexRef, holding the NONE object.
	 */
	inline DexRef(){
		ref = NONE;
		dex_ref(NONE);
	}

	/*Keeps 'ref' parameter uninitialized.
	 *NB: The object must be initialized
	 *before the destructor call!
	 */
	inline explicit DexRef(void*){

	}

	/* Initialize reference to a given
	 * DexObj, updating the ref count.
	 */
	inline DexRef(DexObj* dobj){
		ref = dobj;
		dex_ref(ref);
	}


	/* Initialize a copy of another
	 * DexRef, updating the ref count.
	 */
	DexRef(const DexRef& dr){
		ref = dr.ref;
		dex_ref(ref);
	}
	/*
	 * Copy over the value of another
	 * DexRef, updating both ref counts.
	 */
	void operator=(const DexRef& dr){
		dex_ref(dr.ref);
		dex_unref(ref);
		ref = dr.ref;
	}

	/* Make a reference to a DexObj,
	 * updating the previous contents ref count
	 * as well as the new reference.
	 */
	void operator=(DexObj* dobj){
		dex_ref(dobj);
		dex_unref(ref);
		ref = dobj;
	}

	bool operator<(const DexRef& o) const;
	DexObj& operator*() const {
		return *ref;
	}
	DexObj* operator->() const {
		return ref;
	}
	~DexRef(){
		dex_unref(ref);
	}
	void swap(DexRef& dr){
		DexObj* tref = ref;
		ref = dr.ref;
		dr.ref = tref;

	}
	void operator=(const double& d);
	void set(const char* s, size_t n);
	void operator=(const DexCStr& dcs){
		set(dcs.str, dcs.len);
	}
};
//These functions emulate the constructor/destructor
//And can operate on arrays of DexRef
//You must be sure automatic [const|dest]ructors are NOT being called
//Eg, with memory allocated with malloc and deallocated with free
inline void dex_manual_init(DexRef* dr){
	dr->ref = NONE;
	dex_ref(NONE);
}
inline void dex_manual_init(DexRef* dr, const DexRef& val){
	dr->ref = val.ref;
	dex_ref(dr->ref);
}
inline void dex_manual_initv(DexRef* drv, int drn){
	DexRef* iter = drv, *end = drv + drn;
	while (iter < end){
		iter->ref = NONE;
		iter++;
	}
	NONE->refc += drn;
}
inline void dex_manual_initv(DexRef* drv, const DexRef& val, int drn){
	DexRef* iter = drv, *end = drv + drn;
	while (iter < end){
		iter->ref = val.ref;
		iter++;
	}
	val->refc += drn;
}
inline void dex_manual_initv(DexRef* drv, const DexRef* odrv, int drn){
	DexRef* iter = drv, *end = drv + drn;
	const DexRef* oiter = odrv;
	while (iter < end){
		dex_manual_init(iter, *oiter);
		iter++;
		oiter++;
	}
}


#endif /* DEXREF_H_ */

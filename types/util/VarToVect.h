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

#ifndef VARTOVECT_H_
#define VARTOVECT_H_
#include "VoidDict.h"
#include "../dexlist.h"
#include "../dexdict.h"

class VarToVect {
public:
	//String list keeps ref count, along with being useful generally
	DexRef strList;
	DexRef varList;
	VoidDict varDict;
	VarToVect(){
		varList = create_list(64);
		strList = create_list(64);
	}
	VarToVect(const VarToVect& o) : varDict(o.varDict){
		strList = o.strList;
		varList = o.varList;
	}
	inline void merge(const VarToVect& o){
		varDict.merge(o.varDict);
		list_concat(strList,o.strList);
		list_concat(varList,o.varList);
	}
	inline int add_var(const DexRef& str){
		int ind = (int)(long)varDict.get(str.ref);
		if (ind)
			return ind-1;
		list_push(varList, NONE);
		list_push(strList, str);
		int size = list_size(varList);
		varDict.put((void*)str.ref, (void*)size);
		return size-1;
	}
	inline int pget_var_ind(const DexRef& str){
		int v = (int)(long)varDict.get((void*)str.ref);
		if (v)
			return v-1;
		return add_var(str);
	}
	inline int size() const {
		return get_dex_list(strList.ref)->len;
	}

	inline DexRef* get_var(int i){
		return get_dex_list(varList.ref)->data + i;
	}
	inline DexRef get_var(const DexRef& str){
		int v = (int)(long)varDict.get((void*)str.ref);
		if (v)
			return *get_var(v-1);
		return NONE;
	}
	inline int get_var_ind(const DexRef& str){
		int v = (int)(long)varDict.get((void*)str.ref);
		return v - 1;
	}

	inline DexRef* pget_var(const DexRef& str){
		return get_var(pget_var_ind(str));
	}
	inline void merge_dict(const DexRef& dict){
		DexRef* entry = dict_raw_array(dict) - 2;
		while ( (entry = dict_next_entry(dict, entry)) ){
			*pget_var(entry[0]) = entry[1];
		}
	}
};

#endif /* VARTOVECT_H_ */

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
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * dex_conv.h
 * Provides functionality for checking
 * the type of a dex object, and
 * extracting data held in the object.
 * 'dex_econv' and 'dex_validate' may set the
 * global error state.
 *  Created on: May 7, 2011
 *      Author: 100397561
 */

#ifndef DEX_CONV_H_
#define DEX_CONV_H_
#include "../interp/error.h"
#include "../mainh.h"

/*
#ifdef _MSC_VER
//Cant really do this well in visual studio, unless we want to break 64bit compatibility
long int lrint (double flt){	
	int intgr;
	_asm
	{	fld flt
	fistp intgr
	} ;
	return intgr ;
} 
#endif*/
inline bool is_int(double d, int& intval){
	#ifdef _MSC_VER
	intval = (int)d;
	#else
	intval = lrint(d);
	#endif
	return (intval == d);
}

inline bool dex_validate(const DexRef& dr, DexType* det, const char* func){
	if (dr->type == det)
		return true;
	arg_type_err(func, str_get_cstr(det->tname));
	return false;
}
inline bool dex_econv(DexList** optr, const DexRef& dr, const char* func){
	if (dex_validate(dr,&listType,func)){
		*optr = get_dex_list(dr.ref);
		return true;
	}
	return false;
}
inline bool dex_econv(double* optr, const DexRef& dr, const char* func){
	if (dex_validate(dr,&numType,func)){
		*optr = get_dex_num(dr.ref);
		return true;
	}
	return false;
}
inline bool dex_econv(int* optr, const DexRef& dr, const char* func){
	if (dex_validate(dr,&numType,func)){
		return is_int(get_dex_num(dr.ref), *optr);
	}
	return false;
}
inline bool dex_econv(size_t* optr, const DexRef& dr, const char* func){
	if (dex_validate(dr,&numType,func)){
		int asInt;
		if (!is_int(get_dex_num(dr.ref), asInt) || asInt < 0)
			return false;
		*optr = asInt;
		return true;
	}
	return false;
}
inline bool dex_econv(DexStr** optr, const DexRef& dr, const char* func){
	if (dex_validate(dr,&strType,func)){
		*optr = get_dex_str(dr.ref);
		return true;
	}
	return false;
}
inline bool dex_econv(DexType** optr, const DexRef& dr, const char* func){
	if (dex_validate(dr,&typeType,func)){
		*optr = (DexType*)dr.ref;
		return true;
	}
	return false;
}
//Converts to user type
inline bool dex_econv(DexUObj** optr, const DexRef& dr, const char* func){
	if (!is_utype(dr->type)){
		arg_type_err(func, "user defined object");
		return false;
	}
	*optr = get_dex_uobj(dr.ref);
	return false;
}

#endif /* DEX_CONV_H_ */

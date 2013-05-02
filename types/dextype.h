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

#ifndef DEXTYPE_H_
#define DEXTYPE_H_
#include "../DexRef.h"
#include "util/VoidDict.h"
#include "dexstdops.h"

typedef void ( DexDestr)(DexObj*);
//This is the constructor used from the scripting language
//Essentially this is the call operator for the type object
typedef DexRef ( DexConstr)(DexType* dext, DexRef* args, size_t n);

typedef bool ( DexEquF)(const DexRef& d1, const DexRef& d2);
typedef size_t ( DexHashF)(const DexRef& d1);
typedef DexRef ( DexBnF)(DexRef* dr, size_t n);
typedef void ( DexBnM)(DexRef& self, DexRef* dr, size_t n);
typedef DexRef ( DexStrF)(const DexRef& d1);
typedef void ( DexInitF)(DexType* dext);

struct DexType {
	DexObj __base;
	DexConstr* constr;
	DexDestr* destr;
	DexStrF* tostr;
	DexEquF* equality;
	DexHashF* hashfunc;
	DexInitF* typeinit;
	DexRef tname;
	//Built-in method
	VoidDict cmethMap, cmembMap, dmethMap, dmembMap;
	DexRef mstrList, gettersList, mdexList;
	DexRef uconstr;
	int membn;
	DexRef base_type;
};

void add_std_methods(DexType* type);
DexType make_dex_type(DexInitF* dif, DexConstr* constr, DexDestr* destr,
		DexStrF* dsf, const DexRef& name, DexEquF* def, DexHashF* dhf);
DexType make_dex_type(DexInitF* dif, DexConstr* constr, DexDestr* destr,
		DexStrF* dsf, const char* name, DexEquF* def, DexHashF* dhf);

inline DexType make_dex_type(DexInitF* dif, DexConstr* constr, DexDestr* destr,
		DexStrF* tostr, const char* name) {
	return make_dex_type(dif, constr, destr, tostr, name, &dex_identity_equal,
			&dex_identity_hash);
}
inline DexType make_dex_type(DexInitF* dif, DexConstr* constr, DexStrF* tostr,
		const char* name) {
	return make_dex_type(dif, constr, &default_destroy, tostr, name);
}
inline DexType make_dex_type(DexStrF* tostr, const char* name) {
	return make_dex_type(&add_std_methods, &none_construct, &default_destroy,
			tostr, name);
}

inline DexObj* lookup_user_method(DexType* dt, const DexRef& str) {
	return (DexObj*) dt->dmethMap.get((void*) str.ref);
}

inline bool has_method(DexType* dt, const DexRef& str) {
	return dt->cmethMap.get((void*) str.ref);
}

inline bool has_user_method(DexType* dt, const DexRef& str) {
	return dt->dmethMap.get((void*) str.ref);
}

inline DexBnM* lookup_method(DexType* dt, const DexRef& str) {
	return (DexBnM*) dt->cmethMap.get((void*) str.ref);
}
inline void init_type(DexType* dt) {
	dt->typeinit(dt);
}
void log_const(DexType* dt);

extern DexType typeType;
extern DexType* typeList[];


#endif /* DEXTYPE_H_ */

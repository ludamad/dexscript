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

#include "dextype.h"
#include "typetools.h"
#include "dexstr.h"
#include "../stdobjs/str.h"
#include "../stdobjs/methods.h"
#include "../stdobjs/struct.h"
#include "util/strmake.h"
#include "dexstdops.h"

void add_std_methods(DexType* type){
	add_method(type, DISTR_STR, &dm_objstr);
	add_method(type, DISTR_HASH, &dm_objhash);
	add_method(type, DISTR_REINIT, &dm_objreinit);
}

static DexRef type_str(const DexRef& d1) {
	return str_make("<type ", ((DexType*) d1.ref)->tname, ">");
}
DexType make_dex_type(DexInitF* dif, DexConstr* constr, DexDestr* destr,
		DexStrF* dsf, const char* name, DexEquF* def, DexHashF* dhf){
	return make_dex_type(dif, constr, destr, dsf, create_string(name), def, dhf);
}
DexType make_dex_type(DexInitF* dif, DexConstr* constr,
		DexDestr* destr, DexStrF* dsf, const DexRef& name, DexEquF* def,
		DexHashF* dhf) {
	DexType dt;
	dt.tname = name;
	dt.constr = constr;
	dt.destr = destr;
	dt.tostr = dsf;
	dt.equality = def;
	dt.hashfunc = dhf;
	dt.typeinit = dif;
	dt.uconstr = NONE;
	dt.membn = 0;
	dt.mdexList = EMPTY_LIST;
	dt.mstrList = EMPTY_LIST;
	dt.gettersList = EMPTY_LIST;
	init_dex_base(&dt.__base, &typeType, sizeof(DexType));
	dex_ref(&dt.__base);
	return dt;
}
DexType typeType = make_dex_type(&type_str, "type");

void log_const(DexType* dt){
	const DexRef& n = dt->tname;
	stack_trace(str_make(n,".",DISTR_INIT), str_make("dex",n,".cpp"));
}

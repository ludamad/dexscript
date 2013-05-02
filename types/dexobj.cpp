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

#include "dexobj.h"
#include "dextype.h"
#include "../interp/error.h"
#include "util/strmake.h"
#include "../stdobjs/str.h"
#include "dexset.h"
#include "dexstdops.h"
#include "dexnum.h"
using namespace std;

//Use create_string etc to pass C arguments
DexRef create_dex_object(DexType* dext, DexRef* args, size_t n){
	DexRef ret = (*dext->constr)(dext,args,n);
	return ret;
}

void destroy_dex_object(DexObj* dobj){
	/*int i = dobj->refc;
	dobj->refc = 100;
	static bool nested = false;
	if (!nested){
		cout << "Refc: " << (i+1) << endl;
		nested = true;
		DexRef r = dobj;
		cout << "Destroying ... ";
		if (r->type == &strType)
			dex_str_print(r);
		else
			dex_print(r);
		cout << endl;
		nested = false;
	}
	dobj->refc = 0;*/
	(*dobj->type->destr)(dobj);
	free(dobj);
}

void default_destroy(DexObj* d){
}

DexRef none_construct(DexType* dext, DexRef* args, size_t argn){
	error(OP_ERROR, str_make("Type '", dext->tname, "' is not callable."));
	return NONE;
}

void remove_interned(DexObj* d){
	//cout << "IREF: " << d->refc << endl;
	set_remove(dex_state.internObjs, d);
}

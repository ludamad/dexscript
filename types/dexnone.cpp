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

#include "../mainh.h"
#include "../DexRef.h"
#include "../stdobjs/consts.h"

static DexRef nonestr(const DexRef& dr) {
	return DISTR_NONE;
}

DexType noneType = make_dex_type(&add_std_methods, NULL, &nonestr, "Undefined");

void makeNone(DexObj* dobj) {
	init_dex_base(dobj, &noneType, sizeof(DexObj));
	dex_ref(dobj);
}

DexObj noneobj, tombobj;
DexObj nbreak, ncont, nact;

DexObj *DNONE_BREAK = &nbreak, *DNONE_CONTINUE = &ncont, *DNONE_NOACTION =
		&nact;

//First thing that must be done in the program!
void preinit_none() {
	makeNone(&noneobj);
	makeNone(&tombobj);
	makeNone(&nbreak);
	makeNone(&ncont);
	makeNone(&nact);
}

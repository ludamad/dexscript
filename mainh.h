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

#ifndef MAINH_H_
#define MAINH_H_
#include <cstdlib>
#include <cstring>
using namespace std;
typedef unsigned int uint;
typedef long long int64;
typedef unsigned long long uint64;

#include "types/dexobj.h"
#include "types/funcs/dexbuiltin.h"
#include "types/funcs/dexfunc.h"
#include "types/funcs/dexbound.h"
#include "types/funcs/dexgen.h"
#include "types/io/dexfile.h"
#include "types/dextype.h"
#include "types/typetools.h"
#include "types/dexnum.h"
#include "types/dexlist.h"
#include "types/dexbool.h"
#include "types/dexstr.h"
#include "types/dexuobj.h"
#include "types/dexdict.h"
#include "types/dexset.h"
#include "types/dexmodule.h"
#include "types/dexstdops.h"
#include "stdobjs/math.h"
#include "DexRef.h"
#include "interp/error.h"
#include "interp/global.h"
#include "interp/intern.h"
#include "interp/VarContext.h"

void dex_simple_interactive(DexRef& mod);
void dex_file_interactive(const char* filename, int argc, const char** argv);


#endif /* MAINH_H_ */

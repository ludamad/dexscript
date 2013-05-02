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
#include <new>
DexRef EMPTY_DICT(DEX_DONT_INIT), EMPTY_SET(DEX_DONT_INIT), EMPTY_LIST(DEX_DONT_INIT), EMPTY_STR(DEX_DONT_INIT);

void preinit_struct_consts(){
	new (&EMPTY_DICT) DexRef(create_dict(16));
	new (&EMPTY_SET) DexRef(create_set(16));
	new (&EMPTY_LIST) DexRef(create_list(4));
	new (&EMPTY_STR) DexRef(create_string("",0));
}

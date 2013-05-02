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

#include "VarContext.h"
#include "../types/dexmodule.h"
DexRef VarContext::get_global(const DexRef& str) {
	return mod_get(mod, str);
}
DexRef VarContext::get_global(int i) {
	return *get_dex_mod(mod.ref)->vars.get_var(i);
}
int VarContext::get_global_ind(const DexRef& str) {
	return get_dex_mod(mod.ref)->vars.get_var_ind(str);
}
int VarContext::get_local_ind(const DexRef& str) {
	return (int)(long)vars->get((void*) str.ref) - 1;
}
DexRef* VarContext::pget_global(const DexRef& str) {
	return mod_pget(mod, str);
}
void VarContext::set_global(const DexRef& str, const DexRef& val) {
	mod_put(mod, str, val);
}

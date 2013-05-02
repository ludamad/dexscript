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

#ifndef DEXMODULE_H_
#define DEXMODULE_H_
#include "../DexRef.h"
#include "../ast/ast.h"
#include "typetools.h"
#include "util/VarToVect.h"

struct DexModule {
	DexRef fname;//Path to load from
	VarToVect vars;
	AST ast;
};
inline DexModule* get_dex_mod(DexObj* dobj){
	return (DexModule*)get_dex_memb(dobj,0);
}
inline VarToVect& get_mod_vars(const DexRef& dobj){
	return get_dex_mod(dobj.ref)->vars;
}
DexRef* mod_ind_pget(const DexRef& mod, int index);
DexRef mod_get(const DexRef& mod, const DexRef& str);
DexRef* mod_pget(const DexRef& mod, const DexRef& str);
void mod_put(const DexRef& mod, const DexRef& str, const DexRef& val);
void mod_put_builtin(const DexRef& mod, const DexRef& built);

DexObj* create_module(const DexRef& path);
DexObj* create_module(const char* name = "shell");
extern DexType moduleType;
#endif /* DEXMODULE_H_ */

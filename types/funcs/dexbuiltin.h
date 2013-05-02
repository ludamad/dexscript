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

#ifndef DEXBUILTIN_H_
#define DEXBUILTIN_H_
#include "../../DexRef.h"
#include "../dextype.h"
#include "../typetools.h"
struct BaseNode;
struct DexBuiltin {
	DexBnF* func;
	const char* name;
};

inline DexBuiltin* get_dex_builtin(DexObj* dobj) {
	return (DexBuiltin*) get_dex_memb(dobj, 0);
}
DexObj* create_builtin(DexBnF* func, const char* name);
DexRef builtin_call(const DexRef& bn, DexRef* args, size_t n);
BaseNode* create_load_module(BaseNode* targ, const DexRef& mname,
		const DexRef& fname);

extern DexType builtinType;
#endif /* DEXBUILTIN_H_ */

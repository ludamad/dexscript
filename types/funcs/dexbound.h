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

#ifndef DEXBOUND_H_
#define DEXBOUND_H_
#include "../../DexRef.h"
#include "../typetools.h"

//Base for AST function
struct DexBound {
	DexRef self, memb;
	DexBound(DexRef s, DexRef m) :
		self(s), memb(m) {
	}
};

DexObj* create_bound(const DexRef& self, const DexRef& memb);
inline DexBound* get_dex_bound(DexObj* dobj) {
	return (DexBound*) get_dex_memb(dobj, 0);
}
DexRef bound_call(const DexRef& fn, DexRef* args, size_t argn);
extern DexType boundType;
#endif /* DEXBOUND_H_ */

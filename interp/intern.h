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

#ifndef INTERN_H_
#define INTERN_H_
#include "interp_state.h"
inline DexRef dex_intern(const DexRef& val) {
	if (val->refc & INTERN_REF)
		return val;
	DexRef* dr = set_add(dex_state.internObjs, val);
	dr->ref->refc |= INTERN_REF;
	return *dr;
}
inline DexRef dex_intern(const char* str, size_t len) {
	DexRef* dr = set_add_cstr(dex_state.internObjs, str, len);
	dr->ref->refc |= INTERN_REF;
	return *dr;
}
inline DexRef dex_intern(const char* str) {
	return dex_intern(str, strlen(str));
}
inline DexRef dex_intern(const double& d) {
	DexRef* dr = set_add_num(dex_state.internObjs, d);
	//Required for correct workings...
	DexRef ret = *dr;
	ret->refc |= INTERN_REF;
	return ret;
}

#endif /* INTERN_H_ */

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

#ifndef GLOBAL_H_
#define GLOBAL_H_
#include "interp_state.h"

inline DexRef dex_get_global(const DexRef& dr) {
	return mod_get(dex_state.module, dr);
}
inline void dex_set_global(const DexRef& dr, const DexRef& val) {
	if (dict_has_key(dex_state.langconsts, dr)) {
		std::printf("Error, attempting to redefine language constant.");
	}
	mod_put(dex_state.module, dr, val);
}

void print_globals();

#endif /* GLOBAL_H_ */

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

#ifndef OBJSTACK_H_
#define OBJSTACK_H_
#include <cstdlib>
#include "../DexRef.h"
inline DexRef* make_obj_stack(size_t space){
	return (DexRef*)std::malloc(sizeof(DexRef)*space);
}
inline void push_obj_stack(DexRef*& ptr, const DexRef& val){
	ptr->ref = val.ref;
	dex_ref(val.ref);
	ptr++;
}
inline void free_obj_stack(DexRef* first_ptr, DexRef* last_ptr){
	DexRef* itr = first_ptr;
	while (itr != last_ptr){
		dex_unref(itr->ref);
		itr++;
	}
	free(first_ptr);
}

#endif /* OBJSTACK_H_ */


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

#ifndef DEXFILE_H_
#define DEXFILE_H_
#include <cstdio>
#include "../../DexRef.h"
#include "../typetools.h"

struct DexFile {
	FILE* file;
	DexRef strbuff;
};

inline DexFile* get_dex_file(DexObj* d){
	return (DexFile*)get_dex_memb(d,0);
}
extern DexType fileType;
DexObj* create_file(const char* name, const char* mode);
DexRef read_line(const DexRef& fobj);


#endif /* DEXFILE_H_ */

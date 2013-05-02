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
#include "init.h"

DexType* typeList[] = { &typeType, &boundType, &boolType, &builtinType,
		&genType, &dictType, &listType, &moduleType, &noneType, &numType,
		&setType, &strType, &fileType };

void init_std_types() {
	int size = sizeof(typeList) / sizeof(DexType*);
	for (int i = 0; i < size; i++) {
		init_type(typeList[i]);
	}
}

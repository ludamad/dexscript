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

#include "bytes.h"
#include "../types/dexstr.h"

enum ByteCode {
	//0 arg instructions
	bJUMP,
	//1 arg instructions
};

bytes::bytes() {
}



DexRef call_bytes(const bytes& b, DexRef* args, size_t n){
	char* iter = str_get_cstr(b.byte_str);
	//Main byte loop
	return NONE;//TODO: implement
}


bool validate_bytes(bytes& b){
	return true;//TODO: Implement
}

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

#ifndef BYTES_H_
#define BYTES_H_
#include "../DexRef.h"
#include "../types/util/VarToVect.h"
#include <vector>
#include "../DexRef.h"

const int GLOBAL_START = 65000;

//0bit:	Equ
//1bit: 2byte arg (global names start at )
//2bit: 1 arg or 2 args (of said byte-ness)

struct FunctionContext {
	DexRef* self;
	VoidDict* strToInd;
	DexRef vars[0];
};

struct OptArg {
	int goto_loc;
};

struct bytes {
	std::vector<OptArg> opt_args;
	DexRef byte_str;
	VarToVect vars;
	bytes();
};

DexRef call_bytes(const bytes& b, DexRef* args, size_t n);
bool validate_bytes(bytes& b);
#endif /* BYTES_H_ */

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

#ifndef STR_H_
#define STR_H_

#include "../DexRef.h"

//Interned constants

//Std object methods
extern DexRef DISTR_STR, DISTR_HASH, DISTR_APPEND, DISTR_CONCAT;
extern DexRef DISTR_NAME, DISTR_INCR, DISTR_SORT, DISTR_REVERSE;
extern DexRef DISTR_GET, DISTR_PUT, DISTR_CMP;
extern DexRef DISTR_MAIN;

//Operator overloading methods
extern DexRef DISTR_ADD, DISTR_SUBTRACT, DISTR_MULTIPLY, DISTR_DIVIDE;
extern DexRef DISTR_MODULOS, DISTR_POWER, DISTR_NEGATIVE;
//Std objects
extern DexRef DISTR_PI, DISTR_E, DISTR_NONE, DISTR_TRUE, DISTR_FALSE;
//Control flow
extern DexRef DISTR_DERIVE;
extern DexRef DISTR_BREAK, DISTR_CONTINUE, DISTR_RETURN, DISTR_IN;
extern DexRef DISTR_AND, DISTR_WHILE, DISTR_IMPORT, DISTR_AS, DISTR_FOR;
extern DexRef DISTR_BITAND, DISTR_BITOR, DISTR_BITXOR, DISTR_BITSUP, DISTR_BITSDOWN;
extern DexRef DISTR_FUNC, DISTR_LEN, DISTR_REMOVE, DISTR_HAS;
//Blocks
extern DexRef DISTR_TYPE, DISTR_OBJ;
extern DexRef DISTR_SWITCH, DISTR_CASE, DISTR_FROM, DISTR_INITBASE, DISTR_BIND, DISTR_MBIND;
extern DexRef DISTR_OR, DISTR_IF, DISTR_ELSE, DISTR_ELSEIF;
extern DexRef DISTR_CLOSE, DISTR_READLN, DISTR_READ, DISTR_PRINT, DISTR_PRINTLN;
extern DexRef DISTR_EOF, DISTR_ATSIGN, DISTR_FIND, DISTR_JOIN;
extern DexRef DISTR_PUSH, DISTR_POP, DISTR_TOP;

//UObj constants
extern DexRef DISTR_INIT, DISTR_REINIT;
extern DexRef DISTR_CALL;


void init_string_consts();

#endif /* STR_H_ */

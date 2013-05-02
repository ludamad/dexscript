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

#include "str.h"
#include "../mainh.h"
#include <new>

//Interned constants

//Std object methods
DexRef DISTR_STR, DISTR_HASH, DISTR_APPEND, DISTR_CONCAT;
DexRef DISTR_NAME, DISTR_INCR, DISTR_SORT, DISTR_REVERSE;
DexRef DISTR_GET, DISTR_PUT, DISTR_CMP;
DexRef DISTR_MAIN;
//Operator overloading:
DexRef DISTR_ADD, DISTR_SUBTRACT, DISTR_MULTIPLY, DISTR_DIVIDE;
DexRef DISTR_MODULOS, DISTR_POWER, DISTR_NEGATIVE;

//Std objects
DexRef DISTR_PI, DISTR_E, DISTR_NONE, DISTR_TRUE, DISTR_FALSE;
DexRef DISTR_BITAND, DISTR_BITOR, DISTR_BITXOR, DISTR_BITSUP, DISTR_BITSDOWN;

//Control flow
DexRef DISTR_DERIVE;
DexRef DISTR_BREAK, DISTR_CONTINUE, DISTR_RETURN, DISTR_IN;
DexRef DISTR_AND, DISTR_WHILE, DISTR_IMPORT, DISTR_AS, DISTR_FOR;
DexRef DISTR_FUNC, DISTR_LEN, DISTR_REMOVE, DISTR_HAS;

//Blocks
DexRef DISTR_TYPE, DISTR_OBJ;
DexRef DISTR_SWITCH, DISTR_CASE, DISTR_FROM, DISTR_INITBASE, DISTR_BIND, DISTR_MBIND;
DexRef DISTR_OR, DISTR_IF, DISTR_ELSE, DISTR_ELSEIF;
DexRef DISTR_CLOSE, DISTR_READLN, DISTR_READ, DISTR_PRINT, DISTR_PRINTLN;
DexRef DISTR_EOF, DISTR_ATSIGN, DISTR_FIND, DISTR_JOIN;
DexRef DISTR_PUSH, DISTR_POP, DISTR_TOP;


//UObj constants
DexRef DISTR_INIT, DISTR_REINIT;
DexRef DISTR_CALL;

void init_string_consts() {
	DISTR_STR = dex_intern("str", 3);
	DISTR_HASH = dex_intern("hash", 4);
	DISTR_APPEND = dex_intern("append", 6);
	DISTR_CONCAT = dex_intern("concat", 6);


	DISTR_ADD = dex_intern("add", 3);
	DISTR_SUBTRACT = dex_intern("subtract", 8);
	DISTR_MULTIPLY = dex_intern("multiply", 8);
	DISTR_DIVIDE = dex_intern("divide", 6);
	DISTR_MODULOS = dex_intern("modulos", 7);
	DISTR_POWER = dex_intern("power", 5);
	DISTR_NEGATIVE = dex_intern("negative", 8);

	DISTR_PI = dex_intern("pi", 2);
	DISTR_E = dex_intern("e", 1);
	DISTR_NONE = dex_intern("none", 4);
	DISTR_TRUE = dex_intern("true", 4);
	DISTR_FALSE = dex_intern("false", 5);

	DISTR_DERIVE = dex_intern("derive",6);
	DISTR_BREAK = dex_intern("break", 5);
	DISTR_CONTINUE = dex_intern("continue", 8);
	DISTR_RETURN = dex_intern("return", 6);
	DISTR_IN = dex_intern("in", 2);
	DISTR_AND = dex_intern("and", 3);
	DISTR_WHILE = dex_intern("while", 5);
	DISTR_IMPORT = dex_intern("import", 6);
	DISTR_FOR = dex_intern("for", 3);
	DISTR_FUNC = dex_intern("func", 4);
	DISTR_LEN = dex_intern("len", 3);
	DISTR_REMOVE = dex_intern("remove", 6);
	DISTR_HAS = dex_intern("has", 3);
	DISTR_TYPE = dex_intern("type", 4);
	DISTR_OR = dex_intern("or", 2);
	DISTR_INCR = dex_intern("incr", 4);
	DISTR_IF = dex_intern("if", 2);
	DISTR_ELSE = dex_intern("else", 4);
	DISTR_ELSEIF = dex_intern("elseif", 6);
	DISTR_CLOSE = dex_intern("close", 5);
	DISTR_READLN = dex_intern("readln", 6);
	DISTR_READ = dex_intern("read", 4);
	DISTR_PRINTLN = dex_intern("println", 7);
	DISTR_PRINT = dex_intern("print", 5);
	DISTR_EOF = dex_intern("eof", 3);
	DISTR_ATSIGN = dex_intern("@", 1);
	DISTR_FIND = dex_intern("find", 4);
	DISTR_JOIN = dex_intern("join", 4);
	DISTR_PUSH = dex_intern("push", 4);
	DISTR_POP = dex_intern("pop", 3);
	DISTR_TOP = dex_intern("top", 3);
	DISTR_INIT = dex_intern("init", 4);
	DISTR_REINIT = dex_intern("reinit", 6);
	DISTR_CALL = dex_intern("call", 4);
	DISTR_OBJ = dex_intern("obj", 3);
	DISTR_NAME = dex_intern("name", 4);
	DISTR_SORT = dex_intern("sort", 4);
	DISTR_REVERSE = dex_intern("reverse", 7);
	DISTR_MAIN = dex_intern("main",4);
	DISTR_AS = dex_intern("as",2);
	DISTR_GET = dex_intern("get",3);
	DISTR_PUT = dex_intern("put",3);
	DISTR_CMP = dex_intern("cmp",3);
	DISTR_BITAND = dex_intern("bitand", 6);
	DISTR_BITOR = dex_intern("bitor", 5);
	DISTR_BITXOR = dex_intern("bitxor", 6);
	DISTR_BITSUP = dex_intern("bitsup", 6);
	DISTR_BITSDOWN = dex_intern("bitsdown", 8);
	DISTR_FROM = dex_intern("from", 4);
	DISTR_CASE = dex_intern("case", 4);
	DISTR_BIND = dex_intern("bind", 4);
	DISTR_MBIND = dex_intern("mbind", 5);
	DISTR_SWITCH = dex_intern("switch", 6);
	DISTR_INITBASE = dex_intern("initbase", 8);
}

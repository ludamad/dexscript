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

#include "op.h"

static const Operator ops[] =
	{Operator(opNOP,0,0,false), Operator(opADD, 16, 2, true),
	 Operator(opSUB, 16, 2, true), Operator(opMUL, 17, 2, true),
	 Operator(opDIV, 17, 2, true), Operator(opMOD, 17, 2, true),
	 Operator(opPOW, 18, 2, false),
	 Operator(opEQU, 10, 2, true), Operator(opNEQ, 10, 2, true),
	 Operator(opLT, 11, 2, true), Operator(opGT, 11, 2, true),
	Operator(opEQL, 11, 2, true), Operator(opEQG, 11, 2, true),
	Operator(opCAT, 16, 2, true), Operator(opAND, 3, 2, true),
	Operator(opOR, 2, 2, true), Operator(opNOT, 4, 1, false),
	Operator(opBITAND, 19, 2, true), Operator(opBITOR,19,2, true),
	Operator(opBITXOR,19,2, true), Operator(opBITSDOWN,19,2, true),
	Operator(opBITSUP,19,2, true)};
static const char opStrs[][12] = { "", "+", "-", "*", "/", "%",
								  "^","==", "!=", "<",">",
								  "<=",">=", "~","and", "or",
								  "!", "bitand", "bitor",
								  "bitxor", "bitsup", "bitsdown",};
Operator getOpInfo(char c){
	switch (c){
	case '+':
		return ops[int(opADD)];
	case '-':
		return ops[int(opSUB)];
	case '*':
		return ops[int(opMUL)];
	case '/':
		return ops[int(opDIV)];
	case '%':
		return ops[int(opMOD)];
	case '>':
		return ops[int(opGT)];
	case '<':
		return ops[int(opLT)];
	case '~':
		return ops[int(opCAT)];
	case '^':
		return ops[int(opPOW)];
	case '!':
		return ops[int(opNOT)];
	default:
		return ops[int(opNOP)];
	}
}
Operator getOpInfo(char c1, char c2){
	switch (c1){
	case '=':{
		if (c2 == '='){
			return ops[int(opEQU)];
		}
		break;
	}
	case '>':{
		if (c2 == '=')
			return ops[int(opEQG)];
		break;
	}
	case '<':{
		if (c2 == '=')
			return ops[int(opEQL)];
		break;
	}
	case '!':{
		if (c2 == '=')
			return ops[int(opNEQ)];
		break;
	}
	}
	return *ops;
}
Operator getOpInfo(OpType op){
	return ops[int(op)];
}
const char* getOpStr(OpType op){
	return opStrs[int(op)];
}

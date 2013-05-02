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

#ifndef OP_H_
#define OP_H_

//if a:
//   b()
//else if c():
//   d()
//
//Goes to bytecode ...
//
//JIF a PAST0
//CALL b
//JMP PAST1
//JNIF c PAST1
//CALL d
//PAST1

enum OpType {
	opNOP = 0, opADD = 1, opSUB = 2, opMUL = 3, opDIV = 4, opMOD = 5, opPOW = 6,
	opEQU = 7, opNEQ = 8, opLT = 9, opGT = 10, opEQL = 11, opEQG = 12,
	opCAT = 13, opAND = 14, opOR = 15, opNOT = 16,
	//Here for completion, not for any efficiency reasons (numbers are floats):
	opBITAND = 17, opBITOR = 18, opBITXOR = 19, opBITSDOWN = 20, opBITSUP = 21,
	opMEMB = 22,
	//Used for bytecode flow
	bytec_JMP, bytec_JIF, bytec_JNIF
};
struct Operator {
	unsigned char opcode, prec, args;
	bool left_assoc;
	Operator(OpType o, int p, int a, bool left){
		opcode = (unsigned char)o;
		prec = (unsigned char)p;
		args = (unsigned char)a;
		left_assoc = left;

	}
	Operator(){
		opcode = 0;
		prec = 0;
		args = 0;
		left_assoc = 0;
	}
	inline OpType getOpType(){
		return OpType(opcode);
	}
};
Operator getOpInfo(char c);
Operator getOpInfo(char c1, char c2);
Operator getOpInfo(OpType op);
const char* getOpStr(OpType op);

#endif /* OP_H_ */

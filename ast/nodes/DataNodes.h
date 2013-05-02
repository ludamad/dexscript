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

#ifndef DATANODES_H_
#define DATANODES_H_

#include "BaseNode.h"
#include <vector>

struct LiteralNode: public BaseNode {
	DexRef dr;
	LiteralNode(const DexRef& lit);
	virtual DexRef eval(VarContext& vars);
	virtual void print(int indent);
	virtual ~LiteralNode();
	virtual BaseNode* optimize(int optlevel);
	virtual bool isconst(DexRef& con);
};

struct DictNode: public BaseNode {
	std::vector< std::pair<BaseNode*,BaseNode*> > entries;
	DictNode(){
	}
	virtual DexRef eval(VarContext& vars);
	virtual void print(int indent);
	virtual ~DictNode();
	virtual bool isconst(DexRef& con);
	virtual BaseNode* optimize(int optlevel);
};

struct ListNode: public BaseNode {
	std::vector<BaseNode*> entries;
	ListNode(){

	}
	virtual DexRef eval(VarContext& vars);
	virtual void print(int indent);
	virtual bool isconst(DexRef& con);
	virtual ~ListNode();
	virtual BaseNode* optimize(int optlevel);
};

struct SetNode: public BaseNode {
	std::vector< BaseNode* > entries;
	SetNode(){
	}
	virtual DexRef eval(VarContext& vars);
	virtual void print(int indent);
	virtual ~SetNode();
	virtual bool isconst(DexRef& con);
	virtual BaseNode* optimize(int optlevel);
};

#endif

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

#ifndef CONTROLNODES_H_
#define CONTROLNODES_H_

#include "BaseNode.h"
#include <vector>

struct IfNode: public SBaseNode {
	BaseNode* condition;
	std::vector<BaseNode*> trueb, falseb;
	IfNode(){
		condition = NULL;
	}
	virtual DexRef eval(VarContext& vars);
	virtual void print(int indent);
	virtual ~IfNode();
	virtual BaseNode* optimize(int optlevel);
};

struct ForNode: public SBaseNode {
	BaseNode* value;
	BaseNode* collection;
	BaseNode* ifcondition;
	std::vector<BaseNode*> statements;
	ForNode(){
		value = NULL;
		collection = NULL;
		ifcondition = NULL;
	}
	virtual ~ForNode();

	virtual DexRef eval(VarContext& vars);
	virtual void print(int indent);
	virtual BaseNode* optimize(int optlevel);
};

struct WhileNode: public SBaseNode {
	BaseNode* condition;
	std::vector<BaseNode*> statements;
	WhileNode(){
		condition = NULL;
	}
	virtual DexRef eval(VarContext& vars);
	virtual void print(int indent);
	virtual BaseNode* optimize(int optlevel);
	virtual ~WhileNode();
};

struct JumpNode : SBaseNode {
	BaseNode* optional;
	enum JumpType {
		BREAK, CONTINUE, RETURN
	};
	JumpType type;
	JumpNode(){
		optional = NULL;
	}
	virtual DexRef eval(VarContext& vars);
	virtual ~JumpNode();
	virtual void print(int indent);
};
#endif /* CONTROLNODES_H_ */

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

#ifndef OPERNODES_H_
#define OPERNODES_H_


#include "BaseNode.h"
#include <vector>
#include "../../parsing/op.h"

struct CallNode: public BaseNode {
	BaseNode* func;
	int in;
	std::vector<BaseNode*> args;
	CallNode(){
		in = -1;
		func = NULL;
	}
	virtual void print(int indent);
	virtual DexRef eval(VarContext& vars);
	virtual ~CallNode();
	virtual BaseNode* optimize(int optlevel);
};

struct OpNode: public BaseNode {

	static OpNode* make_def(const DexRef& name, const DexRef& lit, bool glob = true);
	char type;
	bool assign;
	BaseNode* val1, *val2;
	OpNode(char o){
		type = o;
		assign = false;
		val1 = NULL;
		val2 = NULL;
	}

	inline bool assigns(){
		return assign;
	}
	inline void set_assign(){
		assign = true;
	}
	bool is_funcdef();
	bool is_selfdef(DexRef& name);
	virtual DexRef eval(VarContext& vars);
	virtual void print(int indent);
	virtual ~OpNode();
	virtual BaseNode* optimize(int optlevel);
	virtual bool isconst(DexRef& con);
};


struct SubscrNode: public BaseNode {
	BaseNode* val, *index;
	SubscrNode(){
		val = NULL;
		index = NULL;

	}
	virtual void print(int indent);
	virtual DexRef eval(VarContext& vars);
	virtual DexRef* set_eval(VarContext& vars);
	virtual ~SubscrNode();
	virtual BaseNode* optimize(int optlevel);
	virtual bool isconst(DexRef& con);
	virtual bool settable();
};

struct MembCallNode : public BaseNode {
	bool base_class_call;
	int in;
	//If left NULL we assume self
	BaseNode* self;
	DexRef istr;
	std::vector<BaseNode*> args;

	MembCallNode(){
		self = NULL;
		in = -1;
		base_class_call = false;
	}

	inline bool assigns(){
		return in == -2;
	}
	inline void set_assign(){
		in = -2;
	}
	virtual void print(int indent);
	virtual DexRef eval(VarContext& vars);
	virtual ~MembCallNode();
	virtual BaseNode* optimize(int optlevel);
};

struct NotNode: public BaseNode {
	BaseNode* val;
	NotNode(){
		val = NULL;
	}
	virtual DexRef eval(VarContext& vars);
	virtual void print(int indent);
	virtual bool isconst(DexRef& con);
	virtual ~NotNode();
	virtual BaseNode* optimize(int optlevel);
};

#endif /* OPERNODES_H_ */

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

#ifndef LOOKUPNODES_H_
#define LOOKUPNODES_H_


#include "BaseNode.h"
#include "../../interp/intern.h"

struct SelfNode: public BaseNode {
	virtual void print(int indent);
	virtual DexRef eval(VarContext& vars);
	virtual DexRef* set_eval(VarContext& vars);
	virtual ~SelfNode();
	virtual bool settable();
};
struct VarNode: public BaseNode {
	bool glob;
	DexRef name;
	VarNode(const DexRef& n){
		glob = false;
		name = dex_intern(n);
	}
	virtual void print(int indent);
	virtual DexRef eval(VarContext& vars);
	virtual DexRef* set_eval(VarContext& vars);
	virtual ~VarNode();
	virtual bool settable();
};

struct MemberNode: public BaseNode {
	BaseNode* val;
	DexRef str;
	bool assign;
	MemberNode() {
		//If left NULL we assume self
		val = NULL;
		assign = false;
	}
	inline bool assigns() {
		return assign;
	}
	inline void set_assign() {
		assign = true;
	}
	virtual void print(int indent);
	virtual DexRef eval(VarContext& vars);
	virtual DexRef* set_eval(VarContext& vars);
	virtual ~MemberNode();
	virtual BaseNode* optimize(int optlevel);
	virtual bool isconst(DexRef& con);
	virtual bool settable();
};

#endif /* LOOKUPNODES_H_ */

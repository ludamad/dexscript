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

#ifndef BASENODE_H_
#define BASENODE_H_
#include "../../bytecode/bytes.h"
#include "../../interp/VarContext.h"
#include <vector>

struct BaseNode {
	inline BaseNode() {
	}
	virtual DexRef eval(VarContext& vars) = 0;
	virtual void print(int indent) = 0;
	virtual ~BaseNode();
	virtual DexRef* set_eval(VarContext& vars);
	virtual bool settable();
	virtual bool isconst(DexRef& con);
	virtual BaseNode* optimize(int optlevel);
	virtual void bytecode(bytes& code, bool set = false);
	bool is_def();
};
struct SBaseNode: public BaseNode {//For statements that effect break/continue/return functionality
	virtual ~SBaseNode() {
	}
};

DexRef handle_statements(VarContext& vars, const std::vector<BaseNode*>& bns, bool breakable);
void optimize_all(BaseNode** bns, int n, int optlevel);//Does element-wise optimization
void optimize_node(BaseNode*& bn, int optlevel);//Does element-wise optimization
void optimize_all(vector<BaseNode*>& bns, int optlevel);//Does element-wise optimization


#endif /* BASENODE_H_ */

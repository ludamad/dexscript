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

#include "LookupNodes.h"
#include "DataNodes.h"
#include <iostream>
#include "../../types/dexstr.h"
#include "../../types/dexstdops.h"
#include "../../interp/error.h"
#include "../../types/util/strmake.h"

using namespace std;

void SelfNode::print(int indent) {
	while (indent--)
		cout << '\t';
	cout << '@';
}
DexRef SelfNode::eval(VarContext& vars) {
	return *vars.self;
}
DexRef* SelfNode::set_eval(VarContext& vars) {
	return vars.self;
}
SelfNode::~SelfNode() {
}
bool SelfNode::settable() {
	return true;
}

VarNode::~VarNode() {
}
static inline DexRef __novar_msg(const DexRef& name, const DexRef& module) {
	DexRef str = str_make("Undefined symbol '", name, "' in module '");
	str_concat(str, get_dex_mod(module.ref)->fname);
	strc_concat(str, "'.");
	return str;
}
DexRef VarNode::eval(VarContext& vars) {
	if (!glob) {
		int loc_ind = vars.get_local_ind(name);
		if (loc_ind != -1)
			return vars.get_local(loc_ind);
	}
	int glob_ind = vars.get_global_ind(name);
	if (glob_ind != -1)
		return vars.get_global(glob_ind);
	error(DOMAIN_ERROR, __novar_msg(name, vars.mod));
	return NONE;
}
DexRef* VarNode::set_eval(VarContext& vars) {
	if (glob) {
		return vars.pget_global(name);
	} else {
		return vars.pget_local(name);
	}
}
bool VarNode::settable() {
	//TODO: add constants check
	return true;
}

void VarNode::print(int indent) {
	while (indent--)
		cout << '\t';
	cout << get_dex_str(name.ref)->data;
}

bool MemberNode::isconst(DexRef& con) {
	if (!val)
		return false;
	DexRef a;
	if (!val->isconst(a))
		return false;
	con = dex_get_member(a, str);
	return true;
}

BaseNode* MemberNode::optimize(int optlevel) {
	DexRef o;
	if (optlevel) {
		if (isconst(o)) {
			return new LiteralNode(o);
		}
		optimize_node(val, optlevel);
	}
	return this;
}
MemberNode::~MemberNode() {
	delete val;
}
void MemberNode::print(int indent) {
	while (indent--)
		cout << '\t';
	//cout << '(';
	if (!val)
		cout << '@';
	else
		val->print(0);
	cout << '.';
	dex_print(str);
	//cout << ')';
}
DexRef MemberNode::eval(VarContext& vars) {
	//a.=b is an ugly syntax
	//Only here for completion with
	//a.=method()
	if (assign) {
		DexRef* ptr = val ? val->set_eval(vars) : vars.self;
		*ptr = dex_get_member(*ptr, str);
		return NONE;
	}

	return dex_get_member(!val ? *vars.self : val->eval(vars), str);
}

DexRef* MemberNode::set_eval(VarContext& vars) {
	DexRef* obj = !val ? vars.self : val->set_eval(vars);
	if (!obj)
		return NULL;
	return dex_pget_member(*obj, str);
}
bool MemberNode::settable() {
	return val->settable();
}


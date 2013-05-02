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

#include "dexbuiltin.h"
#include <cstdio>
#include <iostream>
#include "../dexstr.h"
#include "../../ast/nodes/nodes.h"
#include "../../stdobjs/basic.h"
#include "../util/strmake.h"

using namespace std;

DexObj* create_builtin(DexBnF* func, const char* name){
	DexObj* ret = alloc_dex_base(&builtinType, sizeof(DexObj) + sizeof(DexBuiltin));
	DexBuiltin* db = get_dex_builtin(ret);
	db->func = func;
	db->name = name;
	return ret;
}

static DexRef builtin_str(const DexRef& dr) {
	DexBuiltin* dbn = get_dex_builtin(dr.ref);
	return str_make("<builtin ", dbn->name, ">");
}

DexRef builtin_call(const DexRef& bn, DexRef* args, size_t n){
	DexBuiltin* db = get_dex_builtin(bn.ref);
	return (*db->func)(args, n);
}

BaseNode* create_load_module(BaseNode* targ, const DexRef& mname, const DexRef& fname){
	OpNode* on = new OpNode(0);
	on->assign = true;
	CallNode* cn = new CallNode;
	LiteralNode* f = new LiteralNode(DFUNC_MODULE);
	LiteralNode* ln1 = new LiteralNode(mname);
	LiteralNode* ln2 = new LiteralNode(fname);


	cn->func = f;
	cn->args.push_back(ln1);
	cn->args.push_back(ln2);

	on->val1 = targ;
	on->val2 = cn;

	return on;
}

DexType builtinType = make_dex_type(&add_std_methods, &none_construct, &default_destroy,
		&builtin_str, "builtin");

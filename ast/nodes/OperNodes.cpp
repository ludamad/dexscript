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

#include "OperNodes.h"
#include <iostream>
#include "nodes.h"
#include "../../mainh.h"
#include "../../interp/interp_state.h"
using namespace std;

CallNode::~CallNode() {
	delete func;
	for (size_t i = 0; i < args.size(); i++) {
		delete args[i];
	}
}

BaseNode* CallNode::optimize(int optlevel) {
	if (optlevel) {
		optimize_all(args, optlevel);
		optimize_node(func, optlevel);
	}
	return this;
}
void CallNode::print(int indent) {
	int i = indent;
	while (i--)
		cout << '\t';

	func->print(0);
	cout << '(';
	int e = args.size();
	for (i = 0; i < e; i++) {
		if (in == i)
			cout << "in ";
		args[i]->print(0);
		if (i != e - 1)
			cout << ", ";
	}
	cout << ')';
}

DexRef CallNode::eval(VarContext& vars) {
	int e = args.size();
	DexRef* argv = dex_raw_alloca(e);
	DexRef* inref, *inarg;
	int i = 0;
	for (; i < in - 1; i++) {
		dex_manual_init(argv + i, args[i]->eval(vars));
	}
	if (in >= 0) {
		inref = args[in]->set_eval(vars);
		inarg = argv + in;
		//Spooky reference diddling...
		//Copy the in ref without increasing refcount
		inarg->ref = inref->ref;
		i++;
	}
	for (; i < e; i++) {
		dex_manual_init(argv + i, args[i]->eval(vars));
	}

	DexRef val = dex_call(func->eval(vars), argv, e);

	if (in >= 0) {
		//Spooky reference undiddling...
		*inarg = NONE;
		dex_manual_init(inref, val);
		dex_dealloca(argv, e);
		return NONE;
	}
	dex_dealloca(argv, e);
	return val;
}

OpNode::~OpNode() {
	delete val1;
	delete val2;
}

bool OpNode::isconst(DexRef& con) {
	DexRef a, b;
	if (!val1->isconst(a) || !val2->isconst(b))
		return false;
	con = a;
	dex_operation(type, con, b);
	return true;
}

BaseNode* OpNode::optimize(int optlevel) {
	DexRef o;
	if (optlevel) {
		if (isconst(o)) {
			return new LiteralNode(o);
		}
		optimize_node(val1, optlevel);
		optimize_node(val2, optlevel);
	}
	return this;
}
bool OpNode::is_funcdef() {
	if (!assign)
		return false;
	LiteralNode* ln;
	VarNode* vn;
	ln = dynamic_cast<LiteralNode*> (val2);
	if (!ln || ln->dr->type != &funcType)
		return false;
	vn = dynamic_cast<VarNode*> (val1);
	if (!vn)
		return false;
	return true;
}
OpNode* OpNode::make_def(const DexRef& name, const DexRef& lit, bool glob) {
	OpNode* on = new OpNode(opNOP);
	on->set_assign();
	VarNode* vn = new VarNode(name);
	vn->glob = glob;
	on->val1 = vn;

	on->val2 = new LiteralNode(lit);
	return on;
}
bool OpNode::is_selfdef(DexRef& name) {
	if (!assign)
		return false;
	MemberNode* m;
	VarNode* v = NULL;
	m = dynamic_cast<MemberNode*> (val1);
	if (m)
		v = dynamic_cast<VarNode*> (m->val);
	if (!v || str_get_cstr(v->name)[0] != '@')
		return false;
	name = m->str;
	return true;
}
DexRef OpNode::eval(VarContext& vars) {
	if (assign) {
		//set_eval mutates, eval first for safety
		DexRef val = val2->eval(vars);
		DexRef* targ = val1->set_eval(vars);
		if (targ) {
			if (type)
				dex_operation(type, *targ, val);
			else
				*targ = val;
		}
		return NONE;
	}
	DexRef ret = val1->eval(vars);
	bool islogical = (type == opOR) || (type == opAND);
	if (islogical && ((type == opOR) == dex_is_true(ret))) {
		return ret;
	}

	DexRef o = val2->eval(vars);
	if (islogical)
		return o;
	dex_operation(type, ret, o);
	return ret;
}
static bool printIfFunc(BaseNode* val1, BaseNode* val2, int indent) {
	LiteralNode* ln;
	VarNode* vn;
	ln = dynamic_cast<LiteralNode*> (val2);
	if (!ln || ln->dr->type != &funcType)
		return false;
	vn = dynamic_cast<VarNode*> (val1);
	if (!vn)
		return false;
	get_dex_func(ln->dr.ref)->print(indent);
	return true;
}
void OpNode::print(int indent) {
	if (assign && printIfFunc(val1, val2, indent))
		return;
	while (indent--)
		cout << '\t';

	if (!assign)
		cout << '(';

	val1->print(0);

	cout << ' ' << getOpStr((OpType) type);
	if (assign)
		cout << '=';
	cout << ' ';

	val2->print(0);

	if (!assign)
		cout << ')';
}

bool SubscrNode::isconst(DexRef& con) {
	DexRef a, b;
	if (!val->isconst(a) || !index->isconst(b))
		return false;
	con = dex_get_subscr(a, b);
	return true;
}

BaseNode* SubscrNode::optimize(int optlevel) {
	DexRef o;
	if (optlevel) {
		if (isconst(o)) {
			return new LiteralNode(o);
		}
		optimize_node(val, optlevel);
		optimize_node(index, optlevel);
	}
	return this;
}
SubscrNode::~SubscrNode() {
	delete val;
	delete index;
}
void SubscrNode::print(int indent) {
	while (indent--)
		cout << '\t';
	val->print(0);
	cout << '[';
	index->print(0);
	cout << ']';
}
DexRef SubscrNode::eval(VarContext& vars) {
	return dex_get_subscr(val->eval(vars), index->eval(vars));
}

DexRef* SubscrNode::set_eval(VarContext& vars) {
	DexRef* obj = val->set_eval(vars);
	if (!obj)
		return NULL;
	/*if (is_utype(obj->ref->type)){

	 }*/
	return dex_pget(*obj, index->eval(vars));
}
bool SubscrNode::settable() {
	return val->settable();
}

MembCallNode::~MembCallNode() {
	delete self;
	for (size_t i = 0; i < args.size(); i++) {
		delete args[i];
	}
}

BaseNode* MembCallNode::optimize(int optlevel) {
	if (optlevel) {
		optimize_node(self, optlevel);
		optimize_all(args, optlevel);
	}
	return this;
}
void MembCallNode::print(int indent) {
	int i = indent;
	while (i--)
		cout << '\t';
	if (self)
	self->print(0);
	else
		cout << '@';
	if (assigns()) {
		cout << ".=";
	} else
		cout << ".";

	dex_print(istr);
	cout << '(';
	int e = args.size();
	for (i = 0; i < e; i++) {
		args[i]->print(0);
		if (i != e - 1)
			cout << ", ";
	}
	cout << ')';
}

DexRef MembCallNode::eval(VarContext& vars) {
	if (error())
		return NONE;

	int e = args.size();
	DexRef* argv = dex_raw_alloca(e);
	DexRef* inref, *inarg;
	int i = 0;
	for (; i < in - 1; i++) {
		dex_manual_init(argv + i, args[i]->eval(vars));
	}
	if (in >= 0) {
		inref = args[in]->set_eval(vars);
		inarg = argv + in;
		//Spooky reference diddling...
		//Copy the in ref without increasing refcount
		inarg->ref = inref->ref;
		i++;
	}
	for (; i < e; i++) {
		dex_manual_init(argv + i, args[i]->eval(vars));
	}

	DexRef val;
	if (assigns())
		dex_call_memb_equ(istr, self ? *self->set_eval(vars) : *vars.self, argv, e);
	else
		val = dex_call_member(istr, self ? self->eval(vars) : *vars.self, argv, e);

	if (in >= 0) {
		//Spooky reference undiddling...
		*inarg = NONE;
		dex_manual_init(inref, val);
		dex_dealloca(argv, e);
		return NONE;
	}
	dex_dealloca(argv, e);
	return val;
}

NotNode::~NotNode() {
	delete val;
}
BaseNode* NotNode::optimize(int optlevel) {
	DexRef d;
	if (isconst(d)) {
		return new LiteralNode(d);
	}
	return this;
}

bool NotNode::isconst(DexRef& con) {
	DexRef r;
	if (!val->isconst(r))
		return false;
	con = dex_is_true(con) ? dex_false : dex_true;
	return true;
}
DexRef NotNode::eval(VarContext& vars) {
	return dex_is_true(val->eval(vars)) ? dex_false : dex_true;
}
void NotNode::print(int indent) {
	while (indent--)
		cout << '\t';
	cout << '!';
	val->print(0);
}


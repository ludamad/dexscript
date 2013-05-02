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

#include "BaseNode.h"
#include "nodes.h"
#include "../../interp/error.h"



BaseNode::~BaseNode() {
}
DexRef* BaseNode::set_eval(VarContext& vars) {
	error(PARSE_ERROR, "Improperly formed left hand expression.");
	return NULL;
}

bool BaseNode::settable(){
	return false;
}

bool BaseNode::is_def(){
	OpNode* opn = dynamic_cast<OpNode*>(this);
	if (!opn || !opn->assigns() || opn->type)
		return false;
	if (!dynamic_cast<VarNode*>(opn->val1))
		return false;
	if (!dynamic_cast<LiteralNode*>(opn->val2))
		return false;
	return true;
}

bool BaseNode::isconst(DexRef& con) {
	return false;
}
BaseNode* BaseNode::optimize(int optlevel) {
	return this;
}

void BaseNode::bytecode(bytes& code, bool set){

}



DexRef handle_statements(VarContext& vars, const std::vector<BaseNode*>& bns, bool breakable) {
	for (size_t i = 0; i < bns.size(); i++) {
		BaseNode* st = bns[i];
		DexRef v = bns[i]->eval(vars);
		if (error())
			return DNONE_NOACTION;

		if (v.ref == DNONE_NOACTION)
			continue;

		if ( dynamic_cast<SBaseNode*> (st) ) {
			if (breakable && v.ref == DNONE_CONTINUE)
				break;
			return v;
		}
	}
	return DNONE_NOACTION;
}

void optimize_all(BaseNode** s, int n, int optlevel){
	for (int i = 0; i < n; i++){
		if (s[i] == NULL)
			continue;
		BaseNode* o = s[i]->optimize(optlevel);
		if (o != s[i]){
			delete s[i];
			s[i] = o;
		}
	}
}
void optimize_node(BaseNode*& s, int optlevel){
	if (s != NULL){
		BaseNode* o = s->optimize(optlevel);
		if (o != s){
			delete s;
			s = o;
		}
	}
}


void optimize_all(vector<BaseNode*>& bns, int optlevel){//Does element-wise optimization
	for (int i = 0; i < bns.size(); i++){
		if (bns[i] == NULL)
			continue;
		BaseNode* o = bns[i]->optimize(optlevel);
		if (o != bns[i]){
			delete bns[i];
			bns[i] = o;
		}
	}
}

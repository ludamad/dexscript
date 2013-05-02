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

#include "parse_structs.h"
#include "../stdobjs/str.h"
//[i for i in range(100) while i < 50 if i % 2]
static BaseNode* parse_comprehension(ParseContext& pc, vector<LexToken*>& ops,
		vector<BaseNode*>& outstack) {

	/*size_t s = outstack.size();
	 LexToken::LexType type = ltoke->type;
	 size_t n = ltoke->values;*/
	//of
			return NULL;//TODO: Implement

}
//Returns assignment node if applicable
static BaseNode* parse_list(ParseContext& pc, LexToken* ltoke,
		vector<BaseNode*>& outstack) {
	size_t s = outstack.size();
	LexToken::LexType type = ltoke->type;
	size_t n = ltoke->values;
	if (type == LexToken::START_SET) {
		if (s < n) {
			parse_err(pc, "Improper syntax.");
			return NULL;
		}
		SetNode* sn = new SetNode;
		for (size_t i = n; i > 0; i--) {
			BaseNode* k = outstack[s - i];
			sn->entries.push_back(k);
		}
		outstack.resize(s - n + 1);
		outstack.back() = sn;
	} else if (type == LexToken::START_DICT) {
		if (s < n) {
			parse_err(pc, "Improper syntax.");
			return NULL;
		}
		DictNode* dn = new DictNode;
		for (size_t i = n; i > 0; i -= 2) {
			BaseNode* k = outstack[s - i], *v = outstack[s - i + 1];
			dn->entries.push_back(pair<BaseNode*, BaseNode*> (k, v));
		}
		outstack.resize(s - n + 1);
		outstack.back() = dn;
	} else if (type == LexToken::START_LIST) {
		if (s < n) {
			parse_err(pc, "Improper syntax.");
			return NULL;
		}
		ListNode* ln = new ListNode;
		for (size_t i = n; i > 0; i--) {
			ln->entries.push_back(outstack[s - i]);
		}
		outstack.resize(s - n + 1);
		outstack.back() = ln;

	} else if (type == LexToken::START_SUB) {
		if (n != 1 || s < 2) {
			parse_err(pc, "Improper syntax.");
			return NULL;
		}
		SubscrNode* sn = new SubscrNode;
		sn->index = outstack.back();
		outstack.pop_back();
		sn->val = outstack.back();
		outstack.back() = sn;
	} else if (type == LexToken::START_PAREN) {
		if (n != 1 || s < 1) {
			parse_err(pc, "Improper syntax.");
			return NULL;
		}//Nothing really needs to be done here...
	} else if (type == LexToken::START_ARGS) {
		if (s < n + 1) {
			parse_err(pc, "Malformed arguments.");
			return NULL;
		}
		CallNode* cn = new CallNode();
		for (size_t i = n; i > 0; i--) {
			BaseNode* v = outstack[s - i];
			cn->args.push_back(v);
		}
		outstack.resize(s - n);
		cn->func = outstack.back();
		MemberNode* mfunc = dynamic_cast<MemberNode*> (cn->func);
		if (mfunc) {
			MembCallNode* mn = new MembCallNode();
			mn->args.swap(cn->args);
			mn->self = mfunc->val;
			mn->istr = dex_intern(mfunc->str);
			bool asgn = mfunc->assigns();
			if (asgn)
				mn->set_assign();
			mfunc->val = NULL;
			outstack.back() = mn;
			delete cn;
			if (asgn)
				return mn;
		} else
			outstack.back() = cn;
	}
	return NULL;
}

static void parse_op(ParseContext& pc, vector<LexToken*>& ops,
		vector<BaseNode*>& outstack) {
	Operator op = ops.back()->o;
	ops.pop_back();
	if (op.opcode == opNOT) {
		if (outstack.empty()) {
			parse_err(pc, "Improper syntax.");
			return;
		}
		NotNode* ret = new NotNode;
		ret->val = outstack.back();
		outstack.back() = ret;
	} else {
		if (outstack.size() < 2) {
			parse_err(pc, "Improper syntax.");
			return;
		}
		OpType ot = (OpType) op.opcode;
		OpNode* ret = new OpNode(ot);
		ret->val2 = outstack.back();
		outstack.pop_back();
		ret->val1 = outstack.back();
		outstack.back() = ret;
	}
}
BaseNode* parse_value(ParseContext& pc, bool can_asgn) {
	vector<BaseNode*> outstack;
	vector<LexToken*> opstack;

	//Node that will make a change to the object
	BaseNode* dn = NULL;
	const LexLine& ll = pc.getLine();
	size_t& tpos = pc.pos;
	size_t epos = ll.tokens.size();
	for (; tpos < epos; tpos++) {
		LexToken* lt = (LexToken*) &ll.tokens[tpos];

		int p = lt->o.prec - lt->o.left_assoc;
		if (lt->type == LexToken::OPERATOR) {
			while (!opstack.empty() && opstack.back()->type
					== LexToken::OPERATOR) {
				Operator o = opstack.back()->o;
				if (p < o.prec)
					parse_op(pc, opstack, outstack);
				else
					break;
			}
			opstack.push_back(lt);
		} else if (lt->type == LexToken::IDENTIFIER) {
			VarNode* v = new VarNode(lt->data);
			outstack.push_back(v);
			if (pc.func) {
				if (pc.func->vars.get_var_ind(lt->data) == -1)
					v->glob = true;
			} else
				v->glob = true;
		} else if (lt->type == LexToken::SELF){
			if (!pc.func) {
				parse_err(pc, "'@' cannot appear outside of function call");
			}
			outstack.push_back(new SelfNode());
		} else if (lt->type == LexToken::LITERAL) {
			outstack.push_back(new LiteralNode(lt->data));
		} else if (is_start_toke(lt)) {
			opstack.push_back(lt);
		} else if (lt->type == LexToken::END_ANY) {
			LexToken* start_toke;
			while (true) {
				if (opstack.empty())
					goto End;
				if (is_start_toke(opstack.back())) {
					start_toke = opstack.back();
					opstack.pop_back();
					break;
				}
				parse_op(pc, opstack, outstack);
			}
			BaseNode* asgn = parse_list(pc, start_toke, outstack);
			if (asgn)
				dn = asgn;
		} else if (lt->type == LexToken::MEMBEQU || lt->type
				== LexToken::MEMBER) {
			if (outstack.empty()) {
				parse_err(pc, "Invalid use of member.");
			}
			MemberNode* mn = new MemberNode;
			mn->str = lt->data;
			mn->val = outstack.back();
			if (lt->type == LexToken::MEMBEQU) {
				if (!mn->val->settable()) {
					parse_err(pc, "Invalid left side for mutation call.");
				}
				if (dn) {
					parse_err(pc, "Assignments cannot be chained.");
				}
				mn->set_assign();
				dn = mn;
			}
			outstack.back() = mn;
		} else if (lt->type == LexToken::ASSIGNMENT) {
			if (dn) {
				parse_err(pc, "Assignments cannot be chained.");
			}
			if (!can_asgn)
				break;
			while (!opstack.empty())
				parse_op(pc, opstack, outstack);
			if (outstack.size() != 1) {
				parse_err(pc, "Improper assignment.");
				break;
			}
			OpNode* on = new OpNode(lt->o.opcode);
			on->val1 = outstack.back();
			on->set_assign();
			dn = on;
			outstack.pop_back();
		} else if (is_separator_toke(lt)) {
			while (true) {
				if (opstack.empty()) {
					if (error(PARSE_ERROR, "Improper syntax."))
						log_line_pos(pc);
					goto End;
				}
				if (is_start_toke(opstack.back())) {
					break;
				}
				parse_op(pc, opstack, outstack);
			}
		} else if (lt->type == LexToken::FOR_LIST) {
			parse_comprehension(pc, opstack, outstack);
		} else {
			break;
		}
	}
	while (!opstack.empty())
		parse_op(pc, opstack, outstack);
	End: if (outstack.size() != 1) {
		if (error(PARSE_ERROR, "Improper syntax, outstack not empty."))
			log_line_pos(pc);
		return NULL;
	}
	BaseNode* ret = outstack.back();
	if (dn) {
		OpNode* on = dynamic_cast<OpNode*> (dn);
		if (on) {
			if (!on->val1->settable()) {
				if (error(PARSE_ERROR, "Invalid left hand side for assignment."))
					log_line_pos(pc);
			}
			on->val2 = ret;
			ret = on;
			if (on->type == opNOP) {
				DexRef name;
				if (on->is_selfdef(name)) {
					if (pc.func) {
						set_add(pc.func->membset, name);
					}
				} else {
					if (pc.func) {
						VarNode* varn = dynamic_cast<VarNode*> (on->val1);
						if (varn) {
							//We are defining a local, we cannot alter a global
							//in a function context
							varn->glob = false;
							int ind = pc.func->vars.add_var(varn->name);
							//LiteralNode* lite =
							//		dynamic_cast<LiteralNode*> (on->val2);

						}
					}
				}
			}
		}
	}
	return ret;
}

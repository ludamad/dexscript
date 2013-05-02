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
#include "../mainh.h"
#include "op.h"
#include "../interp/path.h"
#include "../types/funcs/dexbuiltin.h"
#include "../types/funcs/dexfunc.h"
#include "../types/dexstr.h"
#include "../ast/nodes/nodes.h"
#include "../stdobjs/struct.h"
#include <vector>
#include <iostream>
#include <sstream>
#include "../stdobjs/methods.h"
using namespace std;

const StackFrame PARSE_SP(create_string("parse"), create_string("parse.cpp"));

static string line_pos_str(size_t line, size_t tpos) {
	stringstream ss;
	ss << " at pos " << (tpos + 1);
	ss << " at line " << (line + 1);
	return ss.str();
}

void log_line_pos(size_t line, size_t tpos) {
	stack_trace(PARSE_SP);
	string s = line_pos_str(line, tpos);
	strc_concat(stack_trace_top().src_file, s.c_str(), s.size());
}

BaseNode* parse_line(ParseContext& pc);
void parse_block(ParseContext& pc, vector<BaseNode*>& nodes, size_t indent,
		bool fblock = false);
WhileNode* parse_while(ParseContext& pc);
IfNode* parse_if(ParseContext& pc, bool elseif = false);
ForNode* parse_for(ParseContext& pc);
OpNode* parse_func(ParseContext& pc);
OpNode* parse_type(ParseContext& pc);
JumpNode* parse_jump(ParseContext& pc);
BaseNode* parse_import(ParseContext& pc);

void parse_to_tree(AST& ast, const LexContext& lc, int optlevel) {
	ParseContext pc(lc, optlevel);
	parse_block(pc, ast.root, 0);
	ast.optimize(optlevel);
}

void parse_block(ParseContext& pc, vector<BaseNode*>& nodes, size_t indent,
		bool fblock) {
	bool first = true;
	while (!pc.isFileEnd()) {
		if (pc.getTokens().empty())
			continue;
		size_t line_indent = pc.getLine().indent;
		if ((first && line_indent != indent) || line_indent > indent) {
			parse_err(pc, "Improper indent level in block");
			break;
		} else if (line_indent < indent) {
			break;
		}
		BaseNode* bn;
		if (fblock) {
			bn = parse_func(pc);
			if (!bn) {
				parse_err(pc,
						"Only function definitions expected in type block");
			}
		} else
			bn = parse_line(pc);

		if (bn == NULL)
			return;
		nodes.push_back(bn);
		if (error())
			break;
		first = false;
	}
}

JumpNode* parse_jump(ParseContext& pc) {
	const LexLine& ll = pc.getLine();
	const vector<LexToken>& t = ll.tokens;
	const LexToken& lt = pc.getToken();
	JumpNode* ret = NULL;
	bool did = true;
	if (lt.type == LexToken::RETURN) {
		if (!pc.func) {
			parse_err(pc, "Return statement outside of function");
		} else {
			JumpNode* jn = new JumpNode();
			jn->type = JumpNode::RETURN;
			pc.nextToken();
			BaseNode* val = parse_value(pc, false);
			jn->optional = val;
			ret = jn;
		}
	} else if (lt.type == LexToken::BREAK) {
		if (!pc.inloop) {
			parse_err(pc, "Break statement outside of loop");
		} else {
			JumpNode* jn = new JumpNode();
			jn->type = JumpNode::BREAK;
			ret = jn;
		}
	} else if (lt.type == LexToken::CONTINUE) {
		if (!pc.inloop) {
			parse_err(pc, "Continue statement outside of loop");
		} else {
			JumpNode* jn = new JumpNode();
			jn->type = JumpNode::CONTINUE;
			ret = jn;
		}
	} else
		did = false;
	if (did) {
		pc.nextLine();
	}
	return ret;
}
BaseNode* parse_line(ParseContext& pc) {
	size_t opos = pc.pos;
	BaseNode* ret = NULL;
	if ((ret = parse_import(pc)))
		return ret;
	if ((ret = parse_for(pc)))
		return ret;
	if ((ret = parse_while(pc)))
		return ret;
	if ((ret = parse_if(pc)))
		return ret;
	if ((ret = parse_func(pc)))
		return ret;
	if ((ret = parse_jump(pc)))
		return ret;
	if ((ret = parse_type(pc)))
		return ret;
	if (opos != pc.pos) {
		parse_err(pc, "Improper syntax at start of line.");
	}
	if (error())
		return NULL;
	int in_ind = pc.getLine().in_loc;
	ret = parse_value(pc, true);
	if (in_ind >= 0) {
		MembCallNode* membn;
		CallNode* calln;
		bool baduse = false;
		if ((membn = dynamic_cast<MembCallNode*> (ret))) {
			if (membn->assigns())
				baduse = true;
			membn->in = in_ind;
			if (!membn->args[in_ind]->settable())
				baduse = true;
		} else if ((calln = dynamic_cast<CallNode*> (ret))) {
			calln->in = in_ind;
			if (!calln->args[in_ind]->settable())
				baduse = true;
		} else
			baduse = true;
		if (baduse) {
			parse_err(pc, "Improper use of 'in' keyword.");
			delete ret;
			return NULL;
		}
	}

	if (!pc.isLineEnd()) {
		parse_err(pc, "Improper syntax at end of line.");
		delete ret;
		return NULL;
	}
	pc.nextLine();
	return ret;
}
static const int MODULE_MEMB_MAX = 100;
BaseNode* parse_import(ParseContext& pc) {
	if (pc.getTokens().size() < 2)
		return NULL;

	if (!pc.toke_is(LexToken::IMPORT))
		return NULL;

	pc.nextToken();

	if (!pc.toke_is(LexToken::IDENTIFIER)) {
		parse_err(pc, "Improper import, requires module name.");
		return NULL;
	}
	DexRef mname = create_string("", 0, 32);
	DexRef names[MODULE_MEMB_MAX];
	DexRef varname;
	int i = 0;
	bool has_as = false;
	while (true) {
		varname = pc.getToken().data;
		names[i] = varname;
		str_concat(mname, names[i]);
		i++;
		pc.nextToken();
		if (pc.isLineEnd())
			break;
		if (pc.toke_is(LexToken::AS)) {
			has_as = true;
			break;
		}
		if (!pc.toke_is(LexToken::MEMBER))
			parse_err(pc, "Improper import, requires module name.");
		strc_concat(mname, ".", 1);
	}
	BaseNode* targ = NULL;
	if (has_as) {
		pc.nextToken();
		if (pc.isLineEnd()) {
			parse_err(pc, "Improper use of 'import ... as'.");
		} else {
			targ = parse_value(pc, true);
			if (!pc.isLineEnd() || !targ->settable()) {
				parse_err(pc,
						"Improper use of 'import ... as', requires settable expression.");
			}
		}
	} else {
		VarNode* vn = new VarNode(varname);
		vn->glob = pc.func == NULL;
		targ = vn;
	}
	//cout << mname << endl;
	//cout << varname << endl;
	if (error()){
		delete targ;
		return NULL;
	}
	DexRef fname = path_from_strs(names, i);
	pc.nextLine();
	return create_load_module(targ, mname, fname);
}

ForNode* parse_for(ParseContext& pc) {

	const LexLine& ll = pc.getLine();
	if (!ll.startsBlock || !pc.toke_is(LexToken::FOR))
		return NULL;
	pc.nextToken();
	BaseNode* value = parse_value(pc);
	if (!pc.toke_is(LexToken::IN)) {
		parse_err(pc, "Improper for loop syntax, requires 'in' keyword.");
		delete value;
		return NULL;
	}
	pc.nextToken();
	BaseNode* collection = parse_value(pc);
	ForNode* fn = new ForNode;
	fn->collection = collection;
	fn->value = value;
	pc.nextLine();
	bool wasloop = pc.inloop;
	pc.inloop = true;
	parse_block(pc, fn->statements, ll.indent + 1);
	pc.inloop = wasloop;
	return fn;
}
OpNode* parse_type(ParseContext& pc) {
	const LexLine& ll = pc.getLine();
	int ind = ll.indent + 1;
	DexRef name;
	if (ll.tokens.size() != 2) {
		return NULL;
	}
	if (!ll.startsBlock || !pc.toke_is(LexToken::TYPE)) {
		return NULL;
	}
	pc.nextToken();
	if (!pc.toke_is(LexToken::IDENTIFIER)) {
		parse_err(pc, "Improper use of type keyword");
		return NULL;
	}
	name = pc.getToken().data;
	pc.nextLine();
	TypeInfo tif;
	tif.membset = create_set(64);
	TypeInfo* oldtype = pc.type;
	FunctionInfo* oldfunc = pc.func;
	vector<BaseNode*> nodes;

	pc.type = &tif;
	pc.func = NULL;
	parse_block(pc, nodes, ll.indent + 1, true);
	pc.type = oldtype;
	pc.func = oldfunc;
	DexRef methmap = create_dict(64);
	for (size_t i = 0; i < tif.funcs.size(); i++) {
		DexRef fun = tif.funcs[i];
		DexFunc* df = get_dex_func(fun.ref);
		dict_put(methmap, dex_intern(df->name), fun);
	}
	//dex_print(methmap);
	DexRef memblist = set_to_list(tif.membset);
	DexRef type = (DexObj*) create_utype(name, methmap, memblist);
	for (int i = 0; i < nodes.size(); i++)
		delete nodes[i];
	return OpNode::make_def(name, type, pc.func == NULL);
}
IfNode* parse_if(ParseContext& pc, bool elseif) {
	const LexLine& ll = pc.getLine();
	if (pc.getTokens().size() < 2)
		return NULL;
	LexToken::LexType lt = (elseif ? LexToken::ELSEIF : LexToken::IF);
	if (!ll.startsBlock || !pc.toke_is(lt))
		return NULL;
	pc.nextToken();
	BaseNode* cond = parse_value(pc);
	if (!pc.isLineEnd()) {
		parse_err(pc, "Improper syntax in if statement.");
		return NULL;
	}
	IfNode* ret = new IfNode;
	ret->condition = cond;
	pc.nextLine();

	parse_block(pc, ret->trueb, ll.indent + 1);
	if (!pc.isFileEnd()) {
		const LexLine& elsel = pc.getLine();
		if (elsel.tokens.size() > 0 && elsel.indent == ll.indent) {
			if (elsel.tokens[0].type == LexToken::ELSE) {
				if (elsel.tokens.size() != 1) {
					parse_err(pc, "Improper use of 'else' keyword.");
					delete ret;
					return NULL;
				}
				pc.nextLine();
				parse_block(pc, ret->falseb, ll.indent + 1);
			} else {
				IfNode* ifn = parse_if(pc, true);
				if (ifn) {
					ret->falseb.push_back(ifn);
				}
			}
		}
	}
	return ret;
}
WhileNode* parse_while(ParseContext& pc) {
	const LexLine& ll = pc.getLine();
	const vector<LexToken>& t = ll.tokens;
	if (t.size() < 2)
		return NULL;

	if (!ll.startsBlock || !pc.toke_is(LexToken::WHILE))
		return NULL;
	pc.nextToken();
	BaseNode* cond = parse_value(pc);
	if (!pc.isLineEnd()) {
		parse_err(pc, "Improper syntax.");
		return NULL;
	}
	WhileNode* ret = new WhileNode;
	ret->condition = cond;
	pc.nextLine();
	bool wasloop = pc.inloop;
	pc.inloop = true;
	parse_block(pc, ret->statements, ll.indent + 1);
	pc.inloop = wasloop;
	return ret;
}
OpNode* parse_func(ParseContext& pc) {
	const LexLine& ll = pc.getLine();
	const vector<LexToken>& t = ll.tokens;
	if (t.size() < 4)
		return NULL;

	if (!ll.startsBlock || !pc.toke_is(LexToken::FUNC))
		return NULL;

	pc.nextToken();
	if (!ll.startsBlock || !pc.toke_is(LexToken::IDENTIFIER)) {
		parse_err(pc, "Improper func.");
		return NULL;
	}
	DexRef fname = pc.getToken().data;
	pc.nextToken();
	if (!pc.toke_is(LexToken::START_ARGS)) {
		parse_err(pc, "Improper func.");
		return NULL;
	}
	size_t paramn = pc.getToken().values;
	ASTParam* params = new ASTParam[paramn];
	DexRef func = create_func();
	DexFunc* f = get_dex_func(func.ref);
	f->name = fname;
	f->fname = get_dex_mod(dex_state.module.ref)->fname;
	f->maxargs = paramn;
	f->params = params;

	FunctionInfo* oldinf = pc.func;
	FunctionInfo finfo;
	finfo.membset = EMPTY_SET;

	bool opt_only = false;
	for (size_t i = 0; i < paramn; i++) {
		pc.nextToken();
		if (!pc.toke_is(LexToken::IDENTIFIER))
			return NULL;
		params[i].name = pc.getToken().data;
		finfo.vars.add_var(pc.getToken().data);
		pc.nextToken();
		LexToken::LexType exp = (i == paramn - 1) ? LexToken::END_ANY
				: LexToken::COMMA;
		if (pc.toke_is(exp)) {
			if (opt_only)
				return NULL;
			f->minargs++;
		} else if (pc.toke_is(LexToken::ASSIGNMENT) && pc.getToken().o.opcode
				== 0) {
			pc.nextToken();
			pc.func = &finfo;
			BaseNode* val = parse_value(pc);
			pc.func = oldinf;
			params[i].opt = val;
			opt_only = true;
			if (!pc.toke_is(exp))
				return NULL;
		}
	}
	pc.nextToken();
	if (paramn == 0 && !pc.toke_is(LexToken::END_ANY))
		return NULL;
	if (pc.pos < t.size() - 1) {
		parse_err(pc, "Improper func.");
		return NULL;
	}
	pc.nextLine();
	//You can define a function in a loop, why the hell not
	bool wasloop = pc.inloop;
	pc.func = &finfo;
	pc.inloop = false;
	parse_block(pc, f->body, ll.indent + 1);
	pc.func = oldinf;
	pc.inloop = wasloop;
	if (pc.type) {
		set_merge(pc.type->membset, finfo.membset);
		pc.type->funcs.push_back(func);
	}

	f->varmap.merge(finfo.vars);
	return OpNode::make_def(f->name, func, pc.func == NULL);
}

void parse_and_add_methods(DexType* type, const LexContext& lc, int optlevel){
	ParseContext pc(lc,optlevel);
	TypeInfo tif;
	tif.membset = create_set(64);

	FunctionInfo* oldfunc = pc.func;
	vector<BaseNode*> nodes;

	pc.type = &tif;
	pc.func = NULL;
	parse_block(pc, nodes, 0, true);

	for (size_t i = 0; i < tif.funcs.size(); i++) {
		DexRef fun = tif.funcs[i];
		DexFunc* df = get_dex_func(fun.ref);
		add_method(type, dex_intern(df->name), fun);
	}
	for (int i = 0; i < nodes.size(); i++)
		delete nodes[i];
}


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

#ifndef PARSE_STRUCTS_H_
#define PARSE_STRUCTS_H_
#include "nparse.h"
#include "../mainh.h"
#include "op.h"
#include "../types/funcs/dexbuiltin.h"
#include "../types/funcs/dexfunc.h"
#include "../types/dexstr.h"
#include "../ast/nodes/nodes.h"
#include <vector>
#include "../types/util/VoidDict.h"
#include "../types/util/VarToVect.h"

struct FunctionInfo {
	VarToVect vars;
	DexRef membset;
};

struct TypeInfo {
	VarToVect vars;
	vector<DexRef> funcs;
	DexRef membset;
};

struct ParseContext {
	const LexContext& lc;
	int optlevel;
	FunctionInfo* func;
	TypeInfo* type;
	bool inloop;

	size_t line, pos;

	ParseContext(const LexContext& lc, int opt = 0) : lc(lc){
		optlevel = opt;
		func = NULL;
		inloop = false;
		line = 0;
		pos = 0;
		type = NULL;
	}

	inline bool isLineEnd() {
		return pos >= lc.lines[line].tokens.size();
	}
	inline bool isFileEnd(){
		return line >= lc.lines.size();
	}
	inline void nextToken() {
		pos++;
	}
	inline void nextLine() {
		line++;
		pos = 0;
	}
	inline const LexToken& getToken() {
		return lc.lines[line].tokens[pos];
	}
	inline bool toke_is(LexToken::LexType lt) {
		return getToken().type == lt;
	}
	inline const LexLine& getLine() {
		return lc.lines[line];
	}
	inline const vector<LexToken>& getTokens() {
		return lc.lines[line].tokens;
	}
};


void log_line_pos(size_t line, size_t tpos);
//in shuntparse
BaseNode* parse_value(ParseContext& pc, bool can_asgn = false);
inline static void log_line_pos(ParseContext& pc) {
	log_line_pos(pc.line, pc.pos);
}

static inline void parse_err(ParseContext& pc, const char* msg) {
	if (error(PARSE_ERROR, msg))
		log_line_pos( pc);
}

static inline bool is_start_toke(LexToken* lt) {
	return (lt->type >= LexToken::START_ARGS && lt->type
			<= LexToken::START_DICT);
}
static inline bool is_separator_toke(LexToken* lt) {
	return (lt->type == LexToken::COMMA || lt->type == LexToken::DICT_ARROW);
}
#endif /* PARSE_STRUCTS_H_ */

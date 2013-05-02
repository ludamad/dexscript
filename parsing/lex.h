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

#ifndef LEX_H_
#define LEX_H_
#include "../DexRef.h"
#include <vector>
#include "op.h"


struct LexToken {
	enum LexType {
		LITERAL, IDENTIFIER,
		ASSIGNMENT, OPERATOR,
		START_ARGS, START_SUB,
		START_PAREN, START_LIST,
		START_SET, START_DICT, END_ANY,
		COMMA, TYPE, OBJ, IF, ELSE, ELSEIF,
		WHILE, FOR, FUNC, DICT_ARROW,
		IN, BREAK, CONTINUE, RETURN,
		IMPORT, MEMBER, MEMBEQU, SELF, AS,
		FOR_LIST, WHILE_LIST, IF_LIST,
		FROM, SWITCH, CASE,
		BIND, MBIND, BASE, DERIVE
	};
	LexType type;
	size_t pos, values;//values is for lists
	//Shows how many tokens until the matching END_ANY token
	DexRef data;
	Operator o;

	void print() const;
};

struct LexLine {
	std::vector<LexToken> tokens;
	size_t indent;
	int in_loc;//One per line
	bool startsBlock;
	void print () const;
};
struct LexContext {
	std::vector< LexLine> lines;
	size_t lastIndent;
	bool lastStartsBlock;
	LexContext(){
		lastIndent = 0;
		lastStartsBlock = false;
	}
	void update(const LexLine& pl);

	void print() const;
	void set_from_string(const char* string);
};

LexLine lex_line(const char* line, size_t linen, size_t line_num);
#endif /* LEX_H_ */

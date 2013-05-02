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

#include "lex.h"
#include "../mainh.h"
#include <cctype>
#include "keywords.h"
#include <iostream>
#include <sstream>

using namespace std;
const StackFrame LEX_SP(create_string("lex"), create_string("lex.cpp"));

void LexContext::print() const {
	for (size_t i = 0; i < lines.size(); i++)
		lines[i].print();
}
void LexContext::set_from_string(const char* string){
	const char* last = string;
	const char* delim = string;
	int line_num = 1;

	while (true){
		if (*delim == '\n' || *delim == '\0'){
			this->update(lex_line(last, delim - last, line_num++));
			last = delim+1;
		}
		if (*delim == '\0') break;
		delim++;
	}
}
void LexLine::print() const {
	for (size_t i = 0; i < indent; i++)
		cout << '\t';
	for (size_t i = 0; i < tokens.size(); i++)
		tokens[i].print();
	if (startsBlock)
		cout << ':';
	cout << endl;
}
void LexContext::update(const LexLine& pl) {
	if (pl.tokens.empty()) {
		//lastIndent = 0;
		lastStartsBlock = false;
	} else {
		bool badindent;
		if (lastStartsBlock) {
			badindent = pl.indent != lastIndent + 1;
		} else
			badindent = pl.indent > lastIndent;
		if (badindent) {
			error(LEX_ERROR, "Indent should indicate nesting level.");
			return;
		}

		lines.push_back(pl);
		lastIndent = pl.indent;
		lastStartsBlock = pl.startsBlock;
	}
}
void LexToken::print() const {
	DexRef str;
	DexStr* dd;
	if (data.ref != NONE) {
		str = dex_to_string(data.ref);
		dd = get_dex_str(str.ref);
	}
	switch (type) {
	case LITERAL: {
		bool isstr = data.ref == str.ref;
		if (isstr)
			cout << '"';
		cout << dd-> data;
		if (isstr)
			cout << '"';
		break;
	}
	case MEMBEQU:
	case MEMBER:
		cout << '.';
		if (type == MEMBEQU)
			cout << '=';
	case IDENTIFIER:
		cout << dd->data;
		break;
	case ASSIGNMENT:
		cout << ' ' << getOpStr((OpType) o.opcode) << "= ";
		break;
	case OPERATOR:
		cout << ' ' << getOpStr((OpType) o.opcode) << ' ';
		break;
	case START_ARGS:
		cout << "(START ARGS: ";
		break;
	case START_SUB:
		cout << "(START SUBSCR: ";
		break;
	case START_PAREN:
		cout << "(START EXPR: ";
		break;
	case START_LIST:
		cout << "(START LIST: ";
		break;
	case START_DICT:
		cout << "(START DICT: ";
		break;
	case START_SET:
		cout << "(START SET: ";
		break;
	case END_ANY:
		cout << " END)";
		break;
	case COMMA:
		cout << ", ";
		break;
	case WHILE:
		cout << "(WHILE) ";
		break;
	case IF:
		cout << "(IF) ";
		break;
	case IN:
		cout << "(IN) ";
		break;
	case ELSEIF:
		cout << "(ELSEIF) ";
		break;
	case ELSE:
		cout << "(ELSE) ";
		break;
	case FOR:
		cout << "(FOR) ";
		break;
	case FUNC:
		cout << "(FUNC) ";
		break;
	case TYPE:
		cout << "(TYPE) ";
		break;
	case IMPORT:
		cout << "(IMPORT) ";
		break;
	case BREAK:
		cout << "(BREAK) ";
		break;
	case CONTINUE:
		cout << "(CONTINUE) ";
		break;
	case RETURN:
		cout << "(RETURN) ";
		break;
	case DICT_ARROW:
		cout << " => ";
		break;
	}

}
inline bool starts_var(char c) {
	return (isalpha(c) || c == '_' || c == '@');
}
inline bool part_of_var(char c) {
	return (isalnum(c) || c == '_');
}

struct LexStatus {
	LexLine ll;
	bool needsblock, valexpect;
	vector<LexToken>& v;
	vector<int> startIndices;
};
LexLine lex_line(const char* start, size_t linen, size_t line_num) {
	
	LexLine ll;
	ll.startsBlock = false;
	ll.in_loc = -1;
	vector<LexToken>& v = ll.tokens;
	vector<int> startIndices;
	LexToken lt;
	bool badval = false;
	bool needsblock = false, valexpect = true;
	const char* ptr = start, *end = start + linen;
	if (start != end)
	while ((end[-1] == '\0' || isspace((unsigned char)end[-1])) && end > start) {//Trim spaces
		end--;
	}
	ll.indent = 0;
	while (*ptr == '\t' && ptr != end) {
		ll.indent++;
		ptr++;
	}
	while (ptr < end) {
		while (isspace(*ptr)) {
			ptr++;
			if (ptr >= end)
				continue;
		}

		lt.data = NONE;
		lt.pos = ptr - start;
		lt.values = 0;
		char c = *ptr;
		char nextc;
		if (ptr + 1 < end) {
			nextc = ptr[1];
		} else
			nextc = '\0';

		if ((!valexpect && c == '.') || starts_var(c)) {
			bool member = c == '.';
			bool membequ = false;

			if (member) {
				if (nextc == '=') {
					ptr += 2;
					membequ = true;
				} else
					ptr++;
				c = *ptr;
				if (!starts_var(c)) {
					error(LEX_ERROR, "Improperly formatted member.");
					break;
				}
			}
			const char* vars = ptr;
			do {
				ptr++;
			} while (ptr != end && part_of_var(*ptr));
			size_t strsize = ptr - vars;
			ptr--;
			if (c == '@') {
				if (strsize != 1) {
					error(LEX_ERROR, "Cannot start variable names with '@'.");
					break;
				}
				//^Except of course for '@', the 'self' variable

				lt.type = LexToken::SELF;
				valexpect = false;

			} else {
				DexRef istr = dex_intern(vars, strsize);
				Keyword* key = get_keyword(istr);
				lt.pos = vars - start;
				if (key) {
					if (key->type == LexToken::IN) {
						if (valexpect) {
							if (startIndices.size() != 1) {
								error(LEX_ERROR,
										"Improper use of 'in' argument syntax.");
								break;
							}
							LexToken& last = v[startIndices.back()];
							bool ispar = last.type == LexToken::START_ARGS;
							if (!ispar) {
								error(LEX_ERROR,
										"Improper use of 'in' argument syntax.");
								break;
							}
							ll.in_loc = last.values;
							ptr++;
							continue;
						}
					}
					bool baduse = false;
					bool isstart = ll.tokens.empty();
					char type = key->type;

					if (member)
						baduse = true;
					else {
						if (isstart) {
							if (key->nblock)
								needsblock = true;
						} else {
							if (key->start) {
								type = key->otype;
								if (!type)
									baduse = true;
							}
						}
					}
					if (key->op)
						lt.o = getOpInfo((OpType) key->op);
					lt.type = (LexToken::LexType) type;

					if (baduse) {
						DexRef errstr = create_string(
								"Improper use of keyword \'");
						str_concat(errstr, istr);
						strc_concat(errstr, "\'.", 2);
						error(LEX_ERROR, errstr);
						break;
					}
					valexpect = key->valexp;

				} else {
					lt.data = istr;
					if (membequ)
						lt.type = LexToken::MEMBEQU;
					else if (member)
						lt.type = LexToken::MEMBER;
					else
						lt.type = LexToken::IDENTIFIER;
					valexpect = false;
				}
			}
		} else if (isdigit(c) || (valexpect && (c == '-' || c == '.'))) {
			if (!valexpect) {
				badval = true;
				break;
			}
			char* end;
			double val = strtod(ptr, &end);
			if (starts_var(*end)) {
				error(LEX_ERROR,
						"Can't start variables with digits (instead of 0var, use var0).");
				break;
			}
			lt.type = LexToken::LITERAL;
			lt.data = dex_intern(val);
			ptr = end - 1;
			valexpect = false;
		} else if (c == '\"') {
			ptr++;
			const char* s = ptr;

			while (*ptr != '\"') {
				ptr++;
				if (ptr == end)
					goto End;
			}
			lt.type = LexToken::LITERAL;
			lt.data = dex_intern(s, ptr - s);
			valexpect = false;
		} else if (c == ':') {//Signify start of block
			if (ptr != end - 1) {
				error(LEX_ERROR, "Colon must be at end of line.");
				break;
			}
			if (needsblock) {
				if (valexpect) {
					badval = true;
					break;
				}
				ll.startsBlock = true;
				ptr++;
				continue;
			} else {
				error(LEX_ERROR, "Colon must be before start of block.");
				break;
			}
		} else if (c == ',') {
			if (startIndices.empty()) {
				error(LEX_ERROR, "Comma outside of comma-delimited list.");
				break;
			}
			LexToken& l = v[startIndices.back()];
			LexToken::LexType t = l.type;
			if (t == LexToken::START_PAREN || t == LexToken::START_SUB) {
				error(LEX_ERROR, "Comma inside list where commas are illegal.");
				break;
			}
			if (t == LexToken::START_SET) {

			} else if (t == LexToken::START_DICT) {
				if (l.values % 2 == 0) {
					error(LEX_ERROR, "Dict assumed, incomplete key-value pair.");
					break;
				}
			}
			l.values++;
			lt.type = LexToken::COMMA;

			valexpect = true;
		} else if (c == '(') {
			if (valexpect) {
				lt.type = LexToken::START_PAREN;
			} else {
				if (ll.in_loc >= 0) {
					if (startIndices.empty()) {
						error(LEX_ERROR,
								"Improper use of 'in' argument syntax.");
						break;
					}
				}
				lt.type = LexToken::START_ARGS;
			}
			startIndices.push_back(v.size());
			valexpect = true;
		} else if (c == '[') {
			if (valexpect) {
				lt.type = LexToken::START_LIST;
			} else {
				lt.type = LexToken::START_SUB;
			}
			startIndices.push_back(v.size());
			valexpect = true;
		} else if (c == '=' && nextc == '>') {
			if (startIndices.empty()) {
				error(LEX_ERROR, "Improper place for key-value pair.");
				break;
			}

			LexToken& l = v[startIndices.back()];

			if (l.type == LexToken::START_SET) {
				if (l.values <= 1)
					l.type = LexToken::START_DICT;
				else {
					error(LEX_ERROR,
							"Set assumed, improper place for key-value pair.");
					break;
				}
			} else if (l.type != LexToken::START_DICT) {
				error(LEX_ERROR, "Improper place for key-value pair.");
				break;
			}
			lt.type = LexToken::DICT_ARROW;
			if (valexpect) {
				if (v.back().type != LexToken::START_DICT) {
					badval = true;
					break;
				}
				valexpect = false;
				ptr += 2;
				continue;
			} else {
				if (l.values % 2 == 1) {
					error(LEX_ERROR, "Improper place for key-value pair.");
					break;
				}
				l.values++;
				valexpect = true;
			}

			ptr++;
		} else if (c == '{') {
			if (!valexpect) {
				badval = true;
				break;
			}
			lt.type = LexToken::START_SET;
			startIndices.push_back(v.size());
			valexpect = true;
		} else if (c == '}') {
			bool isset, lastisset;

			if (startIndices.empty()) {
				error(LEX_ERROR, "No match for '}'.");
				break;
			}

			LexToken& l_ = v[startIndices.back()];
			isset = l_.type == LexToken::START_SET;

			if (!isset && l_.type != LexToken::START_DICT) {
				error(LEX_ERROR, "No match for '}'.");
				break;
			}
			lastisset = v.back().type == LexToken::START_SET;
			if (!lastisset) {
				if (valexpect) {
					badval = true;
					break;
				}
				if (v.back().type != LexToken::START_DICT)
					l_.values++;
			}
			lt.type = LexToken::END_ANY;
			valexpect = false;
			startIndices.pop_back();
		} else if (c == ']') {
			if (startIndices.empty()) {
				error(LEX_ERROR, "No match for ']'.");
				break;
			}
			LexToken& l_ = v[startIndices.back()];
			bool islist = l_.type == LexToken::START_LIST;
			if (!islist && l_.type != LexToken::START_SUB) {
				error(LEX_ERROR, "No match for ']'.");
				break;
			}
			if (v.back().type != LexToken::START_LIST) {
				l_.values++;
				if (valexpect) {
					badval = true;
					break;
				}
			}
			lt.type = LexToken::END_ANY;
			valexpect = false;
			startIndices.pop_back();
		} else if (c == ')') {
			if (startIndices.empty()) {
				error(LEX_ERROR, "No match for ')'.");
				break;
			}
			LexToken& l_ = v[startIndices.back()];
			bool isparen = l_.type == LexToken::START_PAREN;
			if (!isparen && l_.type != LexToken::START_ARGS) {
				error(LEX_ERROR, "No match for ')'.");
				break;
			}
			if (v.back().type != LexToken::START_ARGS) {
				l_.values++;
				if (valexpect) {
					badval = true;
					break;
				}
			}
			lt.type = LexToken::END_ANY;
			valexpect = false;
			startIndices.pop_back();
		} else {//Check for operators
			//Rest of line is a comment
			if (c == '#') {
				if (v.empty()) {
					ll.indent = 0;
				}
				break;
			}

			Operator op = getOpInfo(c, nextc);

			if (op.opcode == 0) {
				op = getOpInfo(c);
				bool isval = op.args == 1;
				if (valexpect != isval) {
					badval = true;
					break;
				}
				bool isset = c == '=';
				if (op.opcode == 0 && !isset) {
					error(LEX_ERROR, "Improper operator");
					break;
				}
				lt.o = op;
				if (isset || nextc == '=') {
					lt.type = LexToken::ASSIGNMENT;
					if (!isset)
						ptr++;
				} else {
					lt.type = LexToken::OPERATOR;
				}
			} else {
				if (valexpect) {
					badval = true;
					break;
				}
				ptr++;
				lt.o = op;
				lt.type = LexToken::OPERATOR;
			}
			valexpect = true;
		}
		ptr++;
		v.push_back(lt);
	}
	End: if (!v.empty() && (valexpect || badval))
		error(LEX_ERROR, "Value/Operator mismatch.");

	if (error()) {
		stack_trace(LEX_SP);
		stringstream ss;
		ss << " at pos " << (ptr - start + 1);
		ss << " at line " << line_num;
		string s = ss.str();
		StackFrame& st = stack_trace_top();
		strc_concat(st.src_func, s.c_str(), s.size());
	}
	return ll;
}

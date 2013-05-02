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

#include "keywords.h"
#include "../types/util/VoidDict.h"
#include "../stdobjs/str.h"
#include "../types/dexstdops.h"
#include "lex.h"

Keyword::Keyword(char type, char op, bool nblock, bool valexp, bool start, char otype) :
	type(type), op(op), otype(otype), nblock(nblock), valexp(valexp), start(start){
}

Keyword keyOR(LexToken::OPERATOR, opOR);
Keyword keyAND(LexToken::OPERATOR, opAND);

Keyword keyBITOR(LexToken::OPERATOR, opBITOR);
Keyword keyBITXOR(LexToken::OPERATOR, opBITXOR);
Keyword keyBITAND(LexToken::OPERATOR, opBITAND);
Keyword keyBITSUP(LexToken::OPERATOR, opBITSUP);
Keyword keyBITSDOWN(LexToken::OPERATOR, opBITSDOWN);

Keyword keyIN(LexToken::IN);
Keyword keyFROM(LexToken::FROM);
Keyword keyCASE(LexToken::CASE);
Keyword keySWITCH(LexToken::SWITCH);
Keyword keyBIND(LexToken::BIND);
Keyword keyMBIND(LexToken::MBIND);
Keyword keyAS(LexToken::AS);
//Keyword keyINITBASE(LexToken::INITBASE);

//Keyword keyINITBASE(LexToken::INITBASE);

Keyword keyRETURN(LexToken::RETURN, 0, false, true, true);
Keyword keyIMPORT(LexToken::IMPORT, 0, false, true, true);

Keyword keyBREAK(LexToken::BREAK, 0, false, false, true);
Keyword keyCONTINUE(LexToken::CONTINUE, 0, false, false, true);

Keyword keyTYPE(LexToken::TYPE, 0, true, true, true);
Keyword keyOBJ(LexToken::OBJ, 0, true, true, true);
Keyword keyFOR(LexToken::FOR, 0, true, true, true, LexToken::FOR_LIST);
Keyword keyIF(LexToken::IF, 0, true, true, true, LexToken::IF_LIST);
Keyword keyFUNC(LexToken::FUNC, 0, true, true, true);
Keyword keyELSE(LexToken::ELSE, 0, true, false, true);
Keyword keyELSEIF(LexToken::ELSEIF, 0, true, true, true);
Keyword keyWHILE(LexToken::WHILE, 0, true, true, true, LexToken::WHILE_LIST);


static VoidDict KEYWORD_MAP;

static inline void kmap_add(const DexRef& str, Keyword& key) {
	KEYWORD_MAP.put((void*) str.ref, (void*) &key);
}

Keyword* get_keyword(const DexRef& istr) {
	return (Keyword*) KEYWORD_MAP.get((void*) istr.ref);
}

void init_keywords() {
	kmap_add(DISTR_OR, keyOR);
	kmap_add(DISTR_AND, keyAND);
	kmap_add(DISTR_BITOR, keyBITOR);
	kmap_add(DISTR_BITXOR, keyBITXOR);
	kmap_add(DISTR_BITAND, keyBITAND);
	kmap_add(DISTR_BITSDOWN, keyBITSDOWN);
	kmap_add(DISTR_BITSUP, keyBITSUP);
	kmap_add(DISTR_IN, keyIN);
	kmap_add(DISTR_AS, keyAS);
	kmap_add(DISTR_FROM, keyFROM);
//	kmap_add(DISTR_INITBASE, keyINITBASE);

	kmap_add(DISTR_CASE, keyCASE);
	kmap_add(DISTR_SWITCH, keySWITCH);

	kmap_add(DISTR_BIND, keyBIND);
	kmap_add(DISTR_MBIND, keyMBIND);

	kmap_add(DISTR_RETURN, keyRETURN);
	kmap_add(DISTR_IMPORT, keyIMPORT);

	kmap_add(DISTR_BREAK, keyBREAK);
	kmap_add(DISTR_CONTINUE, keyCONTINUE);

	kmap_add(DISTR_TYPE, keyTYPE);
	kmap_add(DISTR_FOR, keyFOR);
	kmap_add(DISTR_FUNC, keyFUNC);
	kmap_add(DISTR_IF, keyIF);
	kmap_add(DISTR_ELSE, keyELSE);
	kmap_add(DISTR_ELSEIF, keyELSEIF);
	kmap_add(DISTR_WHILE, keyWHILE);
}

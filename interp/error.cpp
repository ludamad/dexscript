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

#include "error.h"
#include "../mainh.h"
#include "../types/util/strmake.h"

void arg_type_err(const char* name, const char* tn, DexType* callt) {
	DexRef s = str_make("Excepted '", tn,
			"' but recieved incompatible argument in call to ");
	if (callt) {
		str_concat(s, callt->tname);
		strc_concat(s, ".");
	}

	strc_concat(s, name);
	error(ARG_ERROR, s);
}

void arg_amnt_err(const char* name, size_t n, size_t min, size_t max, DexType* callt) {
	char buff[50];
	int_to_cstr(min, buff);
	DexRef ret = str_make("Expected ", buff);
	if (min != max) {
		strc_concat(ret, " to ");
		int_to_cstr(max, buff);
		strc_concat(ret, buff);
	}
	strc_concat(ret, " but recieved ");
	int_to_cstr(n, buff);
	strc_concat(ret, buff);
	strc_concat(ret, " arguments in call to ");
	if (callt) {
		str_concat(ret, callt->tname);
		strc_concat(ret, ".");
	}


	strc_concat(ret, name);

	error(ARG_ERROR, ret);
}

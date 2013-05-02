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

#include "path.h"
#include "interp_state.h"
#include "error.h"
#include <cstring>
#include "../types/dexstr.h"
#include "../types/dexmodule.h"
#include "../stdobjs/struct.h"
#ifdef _MSC_VER
#include <direct.h>
#else
#include <unistd.h>
#endif

using namespace std;
const char DEX_EXT[] = ".dex";
const char SLASH = '/', SLASH_S[] = "/", BACK_S[] = "../";
void use_forward_slash(char* c) {
	while (*c) {
		if (*c == '\\')
			*c = SLASH;
		c++;
	}
}
void init_path(const char* path) {
	if (path) {
		strncpy(dex_state.workPath, path, WORKPATH_LIM);
	} else {
		getcwd(dex_state.workPath, WORKPATH_LIM);
	}
	use_forward_slash(dex_state.workPath);
}

static const char* prev_slash(const char* s, int& slashes) {
	const char* end = s + strlen(s) - 1;
	bool hadslash = false;
	if (slashes) {
		for (;;) {
			end--;
			if (*end == SLASH) {
				hadslash = true;
				if (0 == --slashes)
					break;
			}
			if (end <= s) {
				if (!hadslash)
					slashes--;
				return NULL;
			}
		}
	}
	return end + 1;
}
//Utilizes global information
DexRef path_from_strs(DexRef* strs, size_t n) {
	DexModule* dm = get_dex_mod(dex_state.module.ref);
	const char* base = str_get_cstr(dm->fname);
	int slashes = 1;
	while (strcmp(str_get_cstr(*strs), "back") == 0) {
		strs++;
		slashes++;
		n--;
	}
	const char* end = prev_slash(base, slashes);
	DexRef ss = create_string("", 0, 64);
	if (!end) {
		for (int i = 0; i < slashes; i++) {
			strc_concat(ss, BACK_S);
		}
	} else {
		strc_concat(ss, base, end - base);
	}
	str_concat(ss, strs[0]);

	size_t i = 1;
	for (; i < n; i++) {
		strc_concat(ss, SLASH_S);
		str_concat(ss, strs[i]);
	}
	strc_concat(ss, DEX_EXT);
	return ss;
}

DexRef path_from_abs(const DexRef& path) {

	size_t len = strlen(dex_state.workPath);
	char* p;
	DexRef cs = create_string(str_get_cstr(path.ref), str_len(path));
	p =  str_get_cstr(cs.ref);
	use_forward_slash(p);
	if (strncmp(dex_state.workPath, p, len) == 0) {
		cs = create_string(p + len + 1);
	}
	return cs;
}

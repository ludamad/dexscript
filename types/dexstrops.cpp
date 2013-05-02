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

#include "dexstr.h"
#include "typetools.h"
#include <cstring>
#include "dexstdops.h"
#include <iostream>
#include "../stdobjs/methods.h"
#include "../stdobjs/str.h"
#include "../stdobjs/struct.h"


using namespace std;
DexRef DEXSTROPSCPP = create_string("dexstrops.cpp");

void str_replace(DexRef& str, const DexRef& str_before, const DexRef& str_after) {
	//Create a temporary buffer
	DexStr* dstr = get_dex_str(str.ref);
	DexRef out = create_string(NULL,0,dstr->strlen);

	DexStr* dstr_b = get_dex_str(str_before.ref);
	DexStr* dstr_a = get_dex_str(str_after.ref);
	DexStr* dstr_out = get_dex_str(out.ref);

	int blen = dstr_b->strlen, alen = dstr_a->strlen;

	if (!blen) return;

	char* buffiter = dstr_out->data, *buffend = buffiter + dstr_out->strlen;

	char* data = dstr->data, *bdata = dstr_b->data, *adata = dstr_a->data;
	char* enddata = data + dstr->strlen, *enditer = enddata - blen + 1;

	char* laststart = data;
	while (data < enditer) {
		char* write;
		int wlen;
		if (strncmp(data, bdata, blen) == 0) {
			for (int i = 0; i < 2; i++) {
				 if (i){
					write = adata;
					wlen = alen;
					laststart = data + blen;
				 } else {
					 write = laststart;
					 wlen = data - laststart;
				 }
					if (buffiter + wlen <= buffend) {
						memcpy(buffiter, write, wlen);
						buffiter += wlen;
						dstr_out->strlen += wlen;
					} else {
						strc_concat(out, write, wlen);
						dstr_out = get_dex_str(out.ref);
						buffiter = &dstr_out->data[dstr_out->strlen];
						buffend = &dstr_out->data[str_capacity(out)];
					}
				}
			data += blen;
		}
		else
			data++;
	}

	//will null terminate
	strc_concat(out, laststart, enddata - laststart);

	str = out;
}

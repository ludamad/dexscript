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

#include "dexfile.h"
#include "../../stdobjs/methods.h"
#include "../../interp/error.h"
#include "../dexbool.h"
#include "../util/strmake.h"
#include <new>

DexObj* create_file(const char* name, const char* mode) {
	FILE* file = fopen(name, mode);
	if (!file) {
		error(IO_ERROR, str_make("Error loading file \"",name,"\"."));
		return NONE;
	}
	DexObj* ret = alloc_dex_base(&fileType, sizeof(DexObj) + sizeof(DexFile));
	DexFile* df = get_dex_file(ret);
	new (df) DexFile();
	df->file = file;
	df->strbuff = create_string("", 0, 128);
	return ret;
}
DexRef read_num(const DexRef& fobj, size_t num){
	DexFile* file = get_dex_file(fobj.ref);
	FILE* f = file->file;
	//TODO: implement this
	return NONE;
}
DexRef read_line(const DexRef& fobj) {
	DexFile* file = get_dex_file(fobj.ref);
	FILE* f = file->file;
	DexRef ret = file->strbuff;
	if (f) {
		char* dst = str_get_cstr(ret);
		size_t cap = str_capacity(ret);
		if (!fgets(dst,cap,f)){
			error(IO_ERROR, "End-of-file has been reached.");
			return NONE;
		}
		size_t sz = strlen(dst);
		if (dst[sz-1] == '\n')
			sz--;
		str_set_len(ret, sz);
	} else {
		error(IO_ERROR, "Attempt to read from empty/closed file.");
		return NONE;
	}
	return ret;
}


static DexRef create_fileobj(DexType* dext, DexRef* dobjs, size_t len) {
	DexRef& o1 = dobjs[0], o2 = dobjs[1];
	if (len != 2 || o1->type != &strType || o2->type != &strType) {
		error(ARG_ERROR, "Expected two strings");
		return NONE;
	}
	return create_file(str_get_cstr(o1), str_get_cstr(o2));
}

static void destroy_fileobj(DexObj* d) {
	FILE* file = get_dex_file(d)->file;
	if (file)
		fclose(file);
}

static DexRef filestr(const DexRef& file) {
	if (get_dex_file(file.ref)->file == NULL)
		return create_string("<empty file>", 12);
	return create_string("<file>", 6);
}
static void dm_fileclose(DexRef& obj, DexRef* args, size_t n) {
	if (n != 0)
		error(ARG_ERROR, NOARGS);
	DexFile* file = get_dex_file(obj.ref);
	FILE* f = file->file;
	if (f) {
		fclose(f);
		file->file = NULL;
	}
}
static void dm_fileread(DexRef& obj, DexRef* args, size_t n) {
	if (n != 2)
		error(ARG_ERROR, TWOARGS);
	DexFile* file = get_dex_file(obj.ref);
	if (file->file) {
		fclose(file->file);
		file->file = NULL;
	} else {
		error(IO_ERROR, "Attempt to read from uninitialized/closed file.");
	}
}
static void dm_filereadln(DexRef& obj, DexRef* args, size_t n) {
	if (n != 0) {
		error(ARG_ERROR, NOARGS);
	}
	obj = read_line(obj);
}


static void file_print(const DexRef& fobj, bool newline, DexRef* args, size_t n) {
	DexFile* file = get_dex_file(fobj.ref);
	FILE* f = file->file;
	DexRef ret = file->strbuff;
	if (!f) {
		error(IO_ERROR, "Attempt to write to uninitialized/closed file.");
		return;
	}
	DexRef str;
	for (size_t i = 0; i < n; i++) {
		str = dex_to_string(args[i]);
		if (error()) {
			stack_trace(
				newline ? "file.println" : "file.print",
				create_string("dexfile.cpp")
			);
			break;
		}
		DexStr* ds = get_dex_str(str.ref);
		char* s = ds->data;
		size_t sn = ds->strlen;
		fwrite(s, 1, sn, f);
		if (i < n - 1)
			fputc(' ', f);
	}
	if (newline)
		fputc('\n', f);
	fflush(f);
}
static void dm_fileprint(DexRef& obj, DexRef* args, size_t n) {
	file_print(obj, false, args, n);
}

static void dm_fileprintln(DexRef& obj, DexRef* args, size_t n) {
	file_print(obj, true, args, n);
}
static void dm_fileeof(DexRef& obj, DexRef* args, size_t n) {
	if (n != 0) {
		error(ARG_ERROR, NOARGS);
	}
	FILE* f = get_dex_file(obj.ref)->file;
	if (!f)
		obj = dex_true;
	else
		obj = feof(f) ? dex_true : dex_false;
}
void add_file_methods(DexType* det) {
	add_std_methods(det);
	add_method(det, DISTR_CLOSE, &dm_fileclose);
	add_method(det, DISTR_READLN, &dm_filereadln);
	add_method(det, DISTR_READ, &dm_fileread);
	add_method(det, DISTR_PRINTLN, &dm_fileprintln);
	add_method(det, DISTR_PRINT, &dm_fileprint);
	add_method(det, DISTR_EOF, &dm_fileeof);
}
DexType fileType = make_dex_type(&add_file_methods, &create_fileobj,
		&destroy_fileobj, &filestr, "file", &dex_identity_equal,
		&dex_identity_hash);

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

#ifndef STRMAKE_H_
#define STRMAKE_H_
#include "../dexstr.h"
//Convenient string make methods especially for errors
//For syntactic completion, use str_make for conversion
inline DexRef str_make(const char* c) {
	return create_string(c);
}
inline DexRef str_make(const char* c, size_t len) {
	return create_string(c, len);
}
inline DexRef str_make(const char* a, const char* b) {
	DexRef s = str_make(a);
	strc_concat(s, b);
	return s;
}
inline DexRef str_make(const char* a, const DexRef& b) {
	DexRef s = str_make(a);
	str_concat(s, b);
	return s;
}
inline DexRef str_make(const DexRef& a, const char* b) {
	DexRef o = a;
	strc_concat(o, b);
	return o;
}
inline DexRef str_make(const char* a, const DexRef& b, const char* c) {
	DexRef s = str_make(a);
	str_concat(s, b);
	strc_concat(s, c);
	return s;
}
inline DexRef str_make(const DexRef& a, const char* b, const DexRef& c) {
	DexRef s = a;
	strc_concat(s, b);
	str_concat(s, c);
	return s;
}
inline DexRef str_make(const char* a, const char* b, const char* c) {
	DexRef s = str_make(a);
	strc_concat(s, b);
	strc_concat(s, c);
	return s;
}
#endif /* STRMAKE_H_ */

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

#ifndef DEXSTROPS_H_
#define DEXSTROPS_H_

void str_unjoin(DexRef& delim, const DexRef& str);
void str_replace(DexRef& str1, const DexRef& str_before, const DexRef& str_after);
void str_uppercase(DexRef& str);
void str_lowercase(DexRef& str);
void str_substr(DexRef& d1, int start, int end);
//subdivide a string based on a token?
#endif /* DEXSTROPS_H_ */

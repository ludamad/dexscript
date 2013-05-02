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
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * error.h
 * Functionality for signalling errors (uses global variables and checks)
 * Stack trace functions (eg stack_trace) provide context for when errors occur
 * NB: DexScript can be compiled with -fno-exceptions.
 *
 *  Created on: Mar 11, 2011
 *      Author: 100397561
 */

#ifndef ERROR_H_
#define ERROR_H_
#include "interp_state.h"
#include "../types/dexstr.h"

/* Basic function for checking error state
 * Typically we are only concerned if an error exists,
 * for this use eg "if (error()) {return;}" */
inline err_t& error() {
	return dex_state.err.error;
}
/*Retrieve the most recent stack log*/
inline StackFrame& stack_trace_top() {
	return dex_state.err.stack.back();
}
void stack_trace(const StackFrame& sf);

void stack_trace(const char* c, const DexRef& f);

void stack_trace(const DexRef& c, const DexRef& f);

bool error(err_t type, const char* c);
bool error(err_t type, const DexRef& err);

bool handle_error();

inline bool arg_err() {
	return error(ARG_ERROR, "Invalid arguments passed.");
}
void arg_type_err(const char* name, const char* tn, DexType* callt = NULL);

void arg_amnt_err(const char* name, size_t n, size_t min, size_t max, DexType* callt =
		NULL);
static inline void arg_amnt_err(const char* name, size_t n, size_t min,
		DexType* callt = NULL) {
	arg_amnt_err(name, n, min, min, callt);
}

static inline bool argCheck(const char* name, size_t n, size_t min, size_t max,
		DexType* callt = NULL) {
	if (n < min || n > max) {
		arg_amnt_err(name, n, min, max, callt);
		return false;
	}
	return true;
}
#endif /* ERROR_H_ */

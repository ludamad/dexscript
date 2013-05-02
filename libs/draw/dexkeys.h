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


#ifndef DEXKEYS_H_
#define DEXKEYS_H_
#include "../../mainh.h"
static void initKeyData(DexRef& strList) {
	strList = create_list(314);
	DexList* dl = get_dex_list(strList.ref);
	dl->len = 314;
	dex_manual_initv(dl->data, 314);//down and dirty
	struct T {
		const char* name;
		int key;
		T(const char* n, int k) {
			name = n;
			key = k;
		}
	};
	T keyData[] = {
		T("BACKSPACE", 8),
		T("TAB", 9),
		T("CLEAR", 12),
		T("RETURN", 13),
		T("PAUSE", 19),
		T("ESCAPE", 27),
		T("SPACE", 32),
		T("EXCLAIM", 33),
		T("QUOTEDBL", 34),
		T("HASH", 35),
		T("DOLLAR", 36),
		T("AMPERSAND", 38),
		T("QUOTE", 39),
		T("LEFTPAREN", 40),
		T("RIGHTPAREN", 41),
		T("ASTERISK", 42),
		T("PLUS", 43),
		T("COMMA", 44),
		T("MINUS", 45),
		T("PERIOD", 46),
		T("SLASH", 47),
		T("0", 48),
		T("1", 49),
		T("2", 50),
		T("3", 51),
		T("4", 52),
		T("5", 53),
		T("6", 54),
		T("7", 55),
		T("8", 56),
		T("9", 57),
		T("COLON", 58),
		T("SEMICOLON", 59),
		T("LESS", 60),
		T("EQUALS", 61),
		T("GREATER", 62),
		T("QUESTION", 63),
		T("AT", 64),
		T("LEFTBRACKET", 91),
		T("BACKSLASH", 92),
		T("RIGHTBRACKET", 93),
		T("CARET", 94),
		T("UNDERSCORE", 95),
		T("BACKQUOTE", 96),
		T("a", 97),
		T("b", 98),
		T("c", 99),
		T("d", 100),
		T("e", 101),
		T("f", 102),
		T("g", 103),
		T("h", 104),
		T("i", 105),
		T("j", 106),
		T("k", 107),
		T("l", 108),
		T("m", 109),
		T("n", 110),
		T("o", 111),
		T("p", 112),
		T("q", 113),
		T("r", 114),
		T("s", 115),
		T("t", 116),
		T("u", 117),
		T("v", 118),
		T("w", 119),
		T("x", 120),
		T("y", 121),
		T("z", 122),
		T("DELETE", 127),
		/* End of ASCII mapped keysyms */
		/*@}*/

		/** @name Numeric keypad */
		/*@{*/
		T("KP0", 256),
		T("KP1", 257),
		T("KP2", 258),
		T("KP3", 259),
		T("KP4", 260),
		T("KP5", 261),
		T("KP6", 262),
		T("KP7", 263),
		T("KP8", 264),
		T("KP9", 265),
		T("KP_PERIOD", 266),
		T("KP_DIVIDE", 267),
		T("KP_MULTIPLY", 268),
		T("KP_MINUS", 269),
		T("KP_PLUS", 270),
		T("KP_ENTER", 271),
		T("KP_EQUALS", 272),
		/*@}*/

		/** @name Arrows + Home/End pad */
		/*@{*/
		T("UP", 273),
		T("DOWN", 274),
		T("RIGHT", 275),
		T("LEFT", 276),
		T("INSERT", 277),
		T("HOME", 278),
		T("END", 279),
		T("PAGEUP", 280),
		T("PAGEDOWN", 281),
		/*@}*/

		/** @name Function keys */
		/*@{*/
		T("F1", 282),
		T("F2", 283),
		T("F3", 284),
		T("F4", 285),
		T("F5", 286),
		T("F6", 287),
		T("F7", 288),
		T("F8", 289),
		T("F9", 290),
		T("F10", 291),
		T("F11", 292),
		T("F12", 293),
		T("F13", 294),
		T("F14", 295),
		T("F15", 296),
		/*@}*/

		/** @name Key state modifier keys */
		/*@{*/
		T("NUMLOCK", 300),
		T("CAPSLOCK", 301),
		T("SCROLLOCK", 302),
		T("RSHIFT", 303),
		T("LSHIFT", 304),
		T("RCTRL", 305),
		T("LCTRL", 306),
		T("RALT", 307),
		T("LALT", 308),
		T("RMETA", 309),
		T("LMETA", 310),
		T("LSUPER", 311),/**< Left "Windows" key */
		T("RSUPER", 312),/**< Right "Windows" key */
		T("MODE", 313),/**< "Alt Gr" key */
		T("COMPOSE", 314),/**< Multi-key compose key */
	};
	for (int i = 0; i < sizeof(keyData))
}

#endif /* DEXKEYS_H_ */

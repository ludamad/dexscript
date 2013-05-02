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

#include <iostream>
#include <string>
#include "mainh.h"
#include "datastruct/dicttest.h"
#include "parsing/lex.h"
#include "parsing/nparse.h"
#include "ast/ast.h"
#include "interp/interp_state.h"
#include "types/util/VoidDict.h"
#include <cstdio>

using namespace std;

void voidtest() {
	VoidDict vd(32);
	for (int i = 0; i < 100; i++) {
		vd.put((void*) (i + 1), (void*) (i + 1));
	}
	for (int i = 0; i < 100; i++) {
		cout << (int)(long) vd.get((void*) (i + 1)) << endl;
	}
}
void settest() {
	const int trials = 1000;
	DexRef set = create_set(64);
	for (size_t i = 0; i < trials; i++) {
		size_t num = (i + 1) ^ 0xBABE0000;
		num ^= num >> 8;
		num ^= num << 7;
		DexRef dr = create_number(num);
		set_add(set, dr);
		if (set_find(set, dr) == NULL)
			cout << "WHATUP" << endl;
	}
	cout << set_size(set) << " vs " << trials << endl;
	for (size_t i = 0; i < 1000; i++) {
		size_t num = (i + 1) ^ 0xBABE0000;
		num ^= num >> 8;
		num ^= num << 7;
		DexRef dr = create_number(num);
		if (!set_remove(set, dr))
			cout << "WHATUP" << endl;
	}
	cout << set_size(set) << " vs 0" << endl;
}

void dicttest() {
	int data[20000];
	DTest::E* datap = (DTest::E*) data;
	size_t entryn = roundUpPower2(1000);
	for (int i = 0; i < 100; i++) {
		data[i] = 0;
	}
	for (size_t i = 0; i < entryn; i++) {
		size_t num = (i + 1) ^ 0xBABE0000;
		num ^= num >> 8;
		num ^= num << 7;
		tdict_put<DTest> (num, i, datap, entryn);
		if (i % 11 == 0) {
			cout << tdict_get<DTest> (num, datap, entryn)->second << endl;
			tdict_remove<DTest> (num, datap, entryn);
		}
	}
}

void dict2test() {
	DexRef str1 = create_string("Hell", 5), str2 = create_string("Well", 5),
			str3 = create_string("Lell", 5), str4 = create_string("Nell", 5);
	DexRef num1 = create_number(1.), num2 = create_number(2.), num3 =
			create_number(3.), num4 = create_number(4.);

	DexRef dr = create_dict();
	dict_put(dr, str1, num1);
	dict_put(dr, str2, num2);
	dict_put(dr, str3, num3);
	dict_put(dr, str4, num4);
	cout << get_dex_num(dict_get(dr, str1).ref) << endl;
	cout << get_dex_num(dict_get(dr, str2).ref) << endl;
	cout << get_dex_num(dict_get(dr, str3).ref) << endl;
	cout << get_dex_num(dict_get(dr, str4).ref) << endl;
}

void strtest() {
	DexRef str1 = create_string("Hello", 5), str2 = create_string("World", 5);
	str_append(str1, ' ');
	str_concat(str1, str2);
	cout << get_dex_str(str1.ref)->data << endl;
	cout << get_dex_str(str2.ref)->data << endl;
}

void dex_simple_interactive(DexRef& mod) {
	while (!cin.eof()) {
		if (error()) {
			handle_error();
		}
		//cout << "Refcount: " << dex_state.globals << endl;
		LexContext lc;
		char buff[500] = "";
		cout << ">>> ";
		cin.getline(buff, sizeof(buff));
		if (buff[0] == '\0')
			continue;
		if (strcmp(buff, "exit") == 0)
			break;
		if (strcmp(buff, "globals") == 0) {
			print_globals();
			cout << endl;
			continue;
		}
		int line_num = 1;
		LexLine ll = lex_line(buff, strlen(buff), line_num);

		lc.update(ll);

		if (!error() && ll.startsBlock) {
			while (true) {
				cout << "... ";
				cin.getline(buff, sizeof(buff));
				if (buff[0] == '\0')
					break;
				ll = lex_line(buff, strlen(buff), line_num);

				if (error())
					break;
				lc.update(ll);
				if (error())
					break;
				line_num++;
			}
		}
		//lc.print();

		if (error()) {
			handle_error();
			continue;
		}


		DexRef oldmod = dex_state.module;
		DexRef inters = create_string("__INTER");
		DexRef interMod = load_module(NONE, inters);
		dex_state.module = interMod;
		AST ast;
		parse_to_tree(ast, lc);
		dex_state.module = oldmod;
		if (error()) {
			handle_error();
			continue;
		}

		//Opt level 3
		//ast.optimize(3);
		VarContext varc(mod.ref == NONE ? interMod : mod);
		//ast.print();
		DexRef dr = ast.eval(varc);
		//dex_print(dex_state.globals);

		if (dr.ref != NONE) {
			dex_str_print(dr);
			if (!error())
				cout << endl;
		}
		if (error()) {
			handle_error();
			continue;
		}
	}

	//DexRef str = dex_to_string(ast.eval());
	//cout << str_get_cstr(str);
}

void dex_file_interactive(const char* filename, int argc, const char** argv) {
	handle_error();
	DexRef mod = load_module(NONE, create_string(filename));
	handle_error();

	if (mod.ref != NONE){
		set_module(mod);


		DexRef mainf = mod_get(mod,DISTR_MAIN);
		if (mainf.ref != NONE){
			DexRef l = create_list(argc);
			for (int i = 0; i < argc; i++)
				list_push(l,create_string(argv[i]));
			DexRef ret = dex_call(mainf, &l, 1);
			//Can use ret somehow ?
		}
		handle_error();
	}
	
	cout << endl << "-----------------------------" << endl << endl;
	dex_simple_interactive(mod);
}

void interntest() {
	DexRef dr = dex_intern(20.0);
	cout << "N:" << endl;
	dex_print(dr);
	cout << endl;
	cout << dr.ref << endl;
	DexRef dro = dex_intern(20.0);
	cout << dro.ref << endl;
	dr = dex_true;
	dro = dr;
	dr = dex_intern(20.0);
	cout << "N:" << endl;
	dex_print(dr);
	cout << endl;
	cout << dr.ref << endl;
	int a;
	cin >> a;
}
#ifndef DEX_LIBRARY
int main(int argc, const char** argv) {
	dex_init_interpreter();
	//interntest();
	//cout << dex_state.globals << endl;
	//voidtest();
	//dicttest();
	//settest();
	if (argc > 1) {
	dex_file_interactive(argv[1], argc - 2, argv - 2);
	} else {
		//DexRef mod;
		dex_file_interactive("run.dex", argc - 1 , argv - 1);
		//lextest(mod);
	}
	return 0;
}
#endif

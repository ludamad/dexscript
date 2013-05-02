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

#include "DataNodes.h"
#include <iostream>
#include "../../types/dexlist.h"
#include "../../types/dexstr.h"
#include "../../interp/intern.h"
#include "nodes.h"
using namespace std;

LiteralNode::~LiteralNode() {
}

LiteralNode::LiteralNode(const DexRef& lit) {
	dr = lit;
}

bool LiteralNode::isconst(DexRef& con){
	con = dr;
	return true;
}
DexRef LiteralNode::eval(VarContext& vars){
	return dr;
}
BaseNode* LiteralNode::optimize(int optlevel){
	if (optlevel)
		dr = dex_intern(dr);
	return this;
}
void LiteralNode::print(int indent){
	while(indent--) cout << '\t';
	DexRef str = dex_to_string(dr);
	if (str.ref == dr.ref)
		cout << '"';
	cout << get_dex_str(str.ref)->data;
	if (str.ref == dr.ref)
		cout << '"';
}

BaseNode* DictNode::optimize(int optlevel) {
	DexRef o;
	if (optlevel) {
		if (isconst(o)) {
			return new LiteralNode(o);
		}
		if (!entries.empty())
			optimize_all(&entries[0].first, entries.size() * 2, optlevel);
	}
	return this;
}

DexRef DictNode::eval(VarContext& vars) {
	int e = entries.size();
	DexRef dict = create_dict(e * 4 / 3 + 1);

	for (int i = 0; i < e; i++) {
		dict_put(dict, entries[i].first->eval(vars), entries[i].second->eval(vars));
	}
	return dict;
}
void DictNode::print(int indent) {
	while (indent--)
		cout << '\t';
	cout << '{';
	int e = entries.size();
	for (int i = 0; i < e; i++) {
		entries[i].first->print(0);
		cout << " => ";
		entries[i].second->print(0);
		if (i != e - 1)
			cout << ", ";
	}
	if (e == 0)
		cout << "=>";
	cout << '}';
}

DictNode::~DictNode() {
	int e = entries.size();
	for (int i = 0; i < e; i++) {
		delete entries[i].first;
		delete entries[i].second;
	}
}

bool DictNode::isconst(DexRef& con) {
	int e = entries.size();
	bool res = true;
	pair<DexRef, DexRef>* refs = NULL;
	if (e > 0) {
		refs = new pair<DexRef, DexRef> [e];

		for (int i = 0; i < e; i++) {
			if (!entries[i].first->isconst(refs[i].first)) {
				res = false;
				break;
			}
			if (!entries[i].second->isconst(refs[i].second)) {
				res = false;
				break;
			}
		}
	}
	if (res)
		con = create_dict(refs, e);
	delete[] refs;
	return res;
}


BaseNode* ListNode::optimize(int optlevel){
	DexRef o;
	if (optlevel){
		if (isconst(o)){
			return new LiteralNode(o);
		}
		optimize_all(entries, optlevel);
	}
	return this;
}

DexRef ListNode::eval(VarContext& vars){
	int e = entries.size();
	DexRef l = create_list(e);

	for (int i = 0; i < e; i++)
		list_push(l, entries[i]->eval(vars));

	return l;
}
void ListNode::print(int indent){
	while(indent--) cout << '\t';
	cout << '[';
	int e = entries.size();
	for (int i = 0; i < e; i++){
		entries[i]->print(0);
		if (i != e - 1)
			cout << ", ";
	}
	cout << ']';
}

ListNode::~ListNode() {
	int e = entries.size();
	for (int i = 0; i < e; i++){
		delete entries[i];
	}
}


bool ListNode::isconst(DexRef& con){
	int e = entries.size();
	DexRef* refs = new DexRef[e];
	bool res = true;
	for (int i = 0; i < e; i++){
		if (!entries[i]->isconst(refs[i])){
			res = false;
			break;
		}
	}
	if (res)
		con = create_list(refs,e);
	delete[] refs;
	return res;
}


BaseNode* SetNode::optimize(int optlevel) {
	DexRef o;
	if (optlevel) {
		if (isconst(o)) {
			return new LiteralNode(o);
		}
		optimize_all(entries, optlevel);
	}
	return this;
}

DexRef SetNode::eval(VarContext& vars) {
	int e = entries.size();
	DexRef st = create_set(e * 4 / 3 + 1);

	for (int i = 0; i < e; i++) {
		set_add(st, entries[i]->eval(vars));
	}
	return st;
}
void SetNode::print(int indent) {
	while (indent--)
		cout << '\t';
	cout << '{';
	int e = entries.size();
	for (int i = 0; i < e; i++) {
		entries[i]->print(0);
		if (i != e - 1)
			cout << ", ";
	}
	cout << '}';
}

SetNode::~SetNode() {
	int e = entries.size();
	for (int i = 0; i < e; i++) {
		delete entries[i];
	}
}

bool SetNode::isconst(DexRef& con) {
	int e = entries.size();
	bool res = true;
	DexRef* refs = NULL;
	if (e > 0) {
		refs = new DexRef[e];

		for (int i = 0; i < e; i++) {
			if (!entries[i]->isconst(refs[i])) {
				res = false;
				break;
			}
		}
	}
	if (res)
		con = create_set(refs, e);
	delete[] refs;
	return res;
}

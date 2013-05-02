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

#if 0
#include <IL/il.h>
#include "deximage.h"
#include "../../interp/interp_state.h"
#include "../../types/funcs/dexbuiltin.h"

//DexGLUT module
static struct DexDrawModule {
	//Overrides various methods
	bool was_init;
	int lastmouse_x;
	int lastmouse_y;
	Uint8 mouse_state;
	DexRef nameList;//index -> str map
	DexRef keySet;
	bool mouse_left_down;
	bool mouse_right_down;
	bool mouse_moving;
} module_data;

static DexRef mouseLeftButton(DexRef* args, size_t n) {
	const char* NAME = "mouseLeftButton";
	if (n != 0)
		error(ARG_ERROR, NOARGS);
	Uint8 ms = SDL_GetMouseState(NULL, NULL);
	return (ms & SDL_BUTTON(SDL_BUTTON_LEFT)) ? dex_true : dex_false;
}
static DexRef mouseRightButton(DexRef* args, size_t n) {
	const char* NAME = "mouseRightButton";
	if (n != 0)
		error(ARG_ERROR, NOARGS);
	Uint8 ms = SDL_GetMouseState(NULL, NULL);
	return (ms & SDL_BUTTON(SDL_BUTTON_LEFT)) ? dex_true : dex_false;
}
static DexRef mouseX(DexRef* args, size_t n) {
	const char* NAME = "mouseX";
	if (n != 0)
		error(ARG_ERROR, NOARGS);
	int cx;
	SDL_GetMouseState(&cx, NULL);
	return (double) cx;
}
static DexRef mouseY(DexRef* args, size_t n) {
	const char* NAME = "mouseY";
	if (n != 0)
		error(ARG_ERROR, NOARGS);
	int cy;
	SDL_GetMouseState(NULL, &cy);
	return (double) cy;
}

static void drawCircle(double radius, double originX, double originY) {
	glBegin(GL_TRIANGLES);
	float vectorY1 = originY;
	float vectorX1 = originX;

	for (float i = 0; i <= 3.141592 * 2; i += 3.141592 / 360) {
		double vectorX = originX + (radius * sin(i));
		double vectorY = originY + (radius * cos(i));
		glVertex2i(lrintf(originX), lrintf(originY));
		glVertex2i(lrintf(vectorX1), lrintf(vectorY1));
		glVertex2i(lrintf(vectorX), lrintf(vectorY));
		vectorY1 = vectorY;
		vectorX1 = vectorX;
	}

	glEnd();
}
static DexRef draw_circle(DexRef* args, size_t n) {
	const char* NAME = "draw_circle";

	if (n != 3) {
		arg_amnt_err(NAME, n, 3);
		return NONE;
	}
	double radius, originX, originY;
	dex_econv(&radius, args[0], NAME);
	dex_econv(&originX, args[1], NAME);
	dex_econv(&originY, args[2], NAME);
	if (error())
		return NONE;
	if (!module_data.was_init)
		return NONE;
	drawCircle(radius, originX, originY);

	return NONE;
}

//Handle all current events
static DexRef update_handlers(DexRef* args, size_t n) {
	if (n != 0)
		error(ARG_ERROR, NOARGS);
	//Handle all events
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		switch (event.type) {
		case SDL_KEYDOWN:
			if (event.key.keysym.sym == SDLK_ESCAPE)
				finished = 1;
			break;
		case SDL_QUIT:
			finished = 1;
			break;
		}
	}
	ms = SDL_GetMouseState(NULL, NULL);
}
//Takes lists of renderable objects
//Renderable object is any object with a .draw() method
//Updates handlers and such
static DexRef render(DexRef* args, size_t n) {
	const char* NAME = "render";

	if (n != 1) {
		arg_amnt_err(NAME, n, 1);
		return NONE;
	}
	DexRef distr_draw = dex_intern("draw");
	DexRef args[2] = {DNUM_ZERO, DNUM_ZERO};

	DexRef draw_obj = args[0];
	if (dex_has_member(draw_obj),distr_draw )
		dex_call_member(draw_obj, args);
	DexList* obj_list;
	dex_econv(&obj_list, args[0], NAME);
	if (error())
		return NONE;

	return NONE;
}

//Takes callback object, window name, winX, winY
static DexRef init_display(DexRef* args, size_t n) {
	const char* NAME = "init_display";

	if (n != 4) {
		arg_amnt_err(NAME, n, 4);
		return NONE;
	}

	DexStr* winName;
	size_t winX, winY;
	dex_econv(&winName, args[1], NAME);
	dex_econv(&winX, args[2], NAME);
	dex_econv(&winY, args[3], NAME);
	if (error())
		return NONE;

	if (!module_data.was_init) {
		ilInit();
		SDL_Init(SDL_INIT_VIDEO);
	}
	SDL_SetVideoMode(winX, winY, 32, SDL_OPENGL | SDL_HWSURFACE);
	module_data.was_init = true;

}
DexRef dex_draw_module = create_module("dex.draw");
void add_draw_module() {
	//Also initializes
	dict_put(dex_state.internalModules, create_string("dex.draw"),
			dex_draw_module);
	mod_put(dex_draw_module, dex_intern("init_display"), create_builtin(
			&init_display, "init_display"));
	mod_put(dex_draw_module, dex_intern("render"), create_builtin(
			&render, "render"));
	mod_put(dex_draw_module, dex_intern("update_handlers"), create_builtin(
			&update_handlers, "update_handlers"));
	mod_put(dex_draw_module, dex_intern("image"),(DexObj*)&imgType);
}

#endif

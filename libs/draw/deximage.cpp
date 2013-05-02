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
#include "deximage.h"

static DexRef image_str(const DexRef& d) {
	char buff[100];
	DexImage* img = dex_get_image(d.ref);
	int len = snprintf(buff, sizeof(buff),
			"<'image' object: width = %d, height = %d >",
			img->width, img->height
		);
	return create_string(buff, len);
}

static void dm_imagewidth(DexRef& obj, DexRef* args, size_t n) {
	if (n != 0)
		error(ARG_ERROR, NOARGS);
	DexImage* dl = get_dex_image(obj.ref);
	obj = (double) dl->width;
}

static void dm_imageheight(DexRef& obj, DexRef* args, size_t n) {
	if (n != 0)
		error(ARG_ERROR, NOARGS);
	DexImage* dl = get_dex_image(obj.ref);
	obj = (double) dl->height;
}

static void dm_imagedraw(DexRef& obj, DexRef* args, size_t n) {
	if (n != 0)
		error(ARG_ERROR, NOARGS);
	DexImage* dl = get_dex_image(obj.ref);
	obj = (double) dl->height;
}

void destroy_image(DexObj* dobj) {
	DexImage* dl = get_dex_image(dobj.ref);
	glDeleteTextures(1, &dl->glTexture);
}
DexObj* dex_image_base() {
	DexObj* ret = alloc_dex_base(&imgType, sizeof(DexObj) + sizeof(DexImage));
	dex_init_clr(ret, sizeof(DexImage));
	return ret;
}

static void add_image_methods(DexType* det) {
	add_std_methods(det);
	add_method(det, dex_intern("width"), &dm_imagewidth);
	add_method(det, dex_intern("height"), &dm_imageheight);
	add_method(det, dex_intern("draw"), &dm_imagedraw);
}

//Takes a filename
static DexRef create_imageobj(DexRef* args, size_t n) {
	const char* NAME = "init";

	if (n != 1) {
		arg_amnt_err(NAME, n, 1, &imgType);
		return NONE;
	}
	DexStr* name;
	dex_econv(&name, args[0], NAME);
	if (error())
		return NONE;

	return dex_load_image(name->data);
}

DexType imgType = make_dex_type(&add_image_methods, &create_imageobj,
		&destroy_image, &image_str, "image", &dex_identity_equal,
		&dex_identity_hash);

DexRef dex_load_image(const char* fname) {
	// Generate an image name and bind it so we can use it
	ILuint image;
	GLuint texture;
	ILboolean success;
	ilGenImages(1, &image);
	ilBindImage(image);

	// Try to determine the image type
	ILenum imageType = ilDetermineType(fname);
	// Try to load the image
	success = ilLoadImage(fname);

	// Retrieve image information
	int width = ilGetInteger(IL_IMAGE_WIDTH);
	int height = ilGetInteger(IL_IMAGE_HEIGHT);
	ILenum format = ilGetInteger(IL_IMAGE_FORMAT);
	ILenum type = ilGetInteger(IL_IMAGE_TYPE);

	// If the format is not supported, convert to a supported format
	// Also convert if the pixel type is not unsigned byte
	ILenum convertToFormat = format;

	switch (format) {
	case IL_COLOUR_INDEX:
		convertToFormat = IL_RGB;
		break;
	case IL_ALPHA:
		convertToFormat = IL_LUMINANCE_ALPHA;
		break;
	case IL_BGR:
		convertToFormat = IL_RGB;
		break;
	case IL_BGRA:
		convertToFormat = IL_RGBA;
		break;
	default:
		break;
	}

	if ((convertToFormat != format) || (type != IL_UNSIGNED_BYTE)) {
		success = ilConvertImage(convertToFormat, IL_UNSIGNED_BYTE);
	}
	glGenTextures(1, &texture); /* Texture name generation */
	glBindTexture(GL_TEXTURE_2D, texture); /* Binding of texture name */
	/* We will use linear interpolation for magnification filter */
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	/* We will use linear interpolation for minifying filter */
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, ilGetInteger(IL_IMAGE_BPP), width, height,
			0, ilGetInteger(IL_IMAGE_FORMAT), GL_UNSIGNED_BYTE, ilGetData()); /* Texture specification */
	ilDeleteImages(1, &image);
	DexRef imgAlloc = dex_image_base();
	DexImage* img = get_dex_image(imgAlloc.ref);
	img->width = width;
	img->height = height;
	img->glTexture = texture;
}

#endif

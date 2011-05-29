/**
 * @file PngImage.cpp
 *
 * @brief PNG helpers.
 *
 * @section LICENSE
 *
 * Copyright (c) 2011 Olivier Favre
 *
 * This file is part of Breach.
 *
 * Licensed under the Simplified BSD License,
 * for details please see LICENSE file or the website
 * http://www.opensource.org/licenses/BSD-2-Clause
 *
 * @section ACKNOWLEDGEMENTS
 *
 * Adapted from a code by David HENRY
 * @see http://tfc.duke.free.fr/coding/png.html
 */

#include "PngImage.hpp"
#include <png.h>
#include <GL/gl.h>

PngImage::PngImage() {
    this->width = 0;
    this->height = 0;
    this->glFormat = 0;
    this->glInternalFormat = 0;
    this->texels = NULL;
}

PngImage::PngImage(const PngImage& orig) {
    this->width = orig.width;
    this->height = orig.height;
    this->glFormat = orig.glFormat;
    this->glInternalFormat = orig.glInternalFormat;
    this->texels = new GLubyte [this->width * this->height * this->glInternalFormat];
    memcpy(this->texels, orig.texels, sizeof(GLubyte)*(this->width * this->height * this->glInternalFormat));
}

PngImage::~PngImage() {
    delete[] this->texels;
}

void PngImage::GetPNGtextureInfo(int color_type) {
    switch (color_type) {
        case PNG_COLOR_TYPE_GRAY:
            this->glFormat = GL_LUMINANCE;
            this->glInternalFormat = 1;
            break;

        case PNG_COLOR_TYPE_GRAY_ALPHA:
            this->glFormat = GL_LUMINANCE_ALPHA;
            this->glInternalFormat = 2;
            break;

        case PNG_COLOR_TYPE_RGB:
            this->glFormat = GL_RGB;
            this->glInternalFormat = 3;
            break;

        case PNG_COLOR_TYPE_RGB_ALPHA:
            this->glFormat = GL_RGBA;
            this->glInternalFormat = 4;
            break;

        default:
            /* Badness */
            break;
    }
}

bool PngImage::read_from_file(const char* filename) {
    png_byte magic[8];
    png_structp png_ptr;
    png_infop info_ptr;
    int bit_depth, color_type;
    FILE *fp = NULL;
    png_bytep *row_pointers = NULL;
    png_uint_32 w, h;
    int i;

    /* Open image file */
    fp = fopen(filename, "rb");
    if (!fp) {
        fprintf(stderr, "error: couldn't open \"%s\"!\n", filename);
        return false;
    }

    /* Read magic number */
    if (!fread(magic, 1, sizeof (magic), fp)) {
        fprintf(stderr, "error: Cannot read \"%s\"!\n", filename);
        fclose(fp);
        return false;
    }
    /* Check for valid magic number */
    if (!png_check_sig(magic, sizeof (magic))) {
        fprintf(stderr, "error: \"%s\" is not a valid PNG image!\n", filename);
        fclose(fp);
        return false;
    }

    /* Create a png read struct */
    png_ptr = png_create_read_struct (PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr) {
        fclose(fp);
        return false;
    }

    /* Create a png info struct */
    info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
        fclose(fp);
        png_destroy_read_struct(&png_ptr, NULL, NULL);
        return false;
    }

    /* Initialize the setjmp for returning properly after a libpng
       error occured */
    if (setjmp(png_jmpbuf(png_ptr))) {
        fclose(fp);
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);

        if (row_pointers)
            delete[] row_pointers;

        if (this->texels) {
            delete[] this->texels;
            this->texels = NULL;
        }

        return false;
    }

    /* Setup libpng for using standard C fread() function
       with our FILE pointer */
    png_init_io(png_ptr, fp);

    /* Tell libpng that we have already read the magic number */
    png_set_sig_bytes(png_ptr, sizeof (magic));

    /* Read png info */
    png_read_info(png_ptr, info_ptr);

    /* Get some usefull information from header */
    bit_depth = png_get_bit_depth(png_ptr, info_ptr);
    color_type = png_get_color_type(png_ptr, info_ptr);

    /* Convert index color images to RGB images */
    if (color_type == PNG_COLOR_TYPE_PALETTE)
        png_set_palette_to_rgb(png_ptr);

    /* Convert 1-2-4 bits grayscale images to 8 bits
       grayscale. */
    if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
        png_set_gray_1_2_4_to_8(png_ptr);

    if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
        png_set_tRNS_to_alpha(png_ptr);

    if (bit_depth == 16)
        png_set_strip_16(png_ptr);
    else if (bit_depth < 8)
        png_set_packing(png_ptr);

    /* Update info structure to apply transformations */
    png_read_update_info(png_ptr, info_ptr);

    /* Retrieve updated information */
    png_get_IHDR(png_ptr, info_ptr, &w, &h, &bit_depth, &color_type, NULL, NULL, NULL);
    this->width = w;
    this->height = h;

    /* Get image format and components per pixel */
    GetPNGtextureInfo(color_type);

    /* We can now allocate memory for storing pixel data */
    this->texels = new GLubyte [this->width * this->height * this->glInternalFormat];

    /* Setup a pointer array.  Each one points at the begening of a row. */
    row_pointers = new png_bytep[this->height];

    for (i = 0; i < this->height; ++i) {
#if 1
        row_pointers[i] = (png_bytep) (this->texels + ((this->height - (i + 1)) * this->width * this->glInternalFormat));
#else
        row_pointers[i] = (png_bytep) (this->texels + (this->width * i * this->internalFormat));
#endif
    }

    /* Read pixel data using row pointers */
    png_read_image(png_ptr, row_pointers);

    /* Finish decompression and release memory */
    png_read_end(png_ptr, NULL);
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);

    /* We don't need row pointers anymore */
    delete[] row_pointers;

    fclose(fp);
    return true;
}
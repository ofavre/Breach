/**
 * @file PngImage.hpp
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
 */

#ifndef _PNGIMAGE_HPP
#define _PNGIMAGE_HPP 1

#include <GL/gl.h>

/**
 * @brief Class used to read PNG images and use them in OpenGL.
 *
 * This class is simply used to ease use of PNG as textures for direct OpenGL usage.
 */
class PngImage {

    private:
        //! @brief Width of the image
        int width;
        //! @brief Height of the image
        int height;
        //! @brief Corresponding OpenGL format of the image
        GLenum glFormat;
        //! @brief Corresponding OpenGL internal format of the image
        GLint glInternalFormat;
        //! @brief Access to the decoded pixels
        GLubyte* texels;

    public:
        //! @brief Creates a new instance to be fed using \link read_from_file(const char*) \endlink
        PngImage();
        //! @brief Copy constructor.
        PngImage(const PngImage& orig);
        //! @brief Destructor.
        virtual ~PngImage();

        //! @brief Returns the width of the image
        inline int getWidth() { return this->width; }
        //! @brief Returns the height of the image
        inline int getHeight() { return this->height; }
        //! @brief Returns the OpenGL format of the image
        inline GLenum getGLFormat() { return this->glFormat; }
        //! @brief Returns the OpenGL internal format of the image
        inline GLint getGLInternalFormat() { return this->glInternalFormat; }
        //! @brief Returns access to the decoded pixels
        inline GLubyte* getTexels() { return this->texels; }

        /** @brief Feeds the current instance with the given PNG file.
         *
         * @param filename Path to the PNG image to be read.
         * @return true if success, false on error.
         */
        bool read_from_file(const char* filename);

    private:
        //! @brief Tries to guess the OpenGL format and internal format from the PNG image type.
        void GetPNGtextureInfo (int color_type);

};

#endif	/* _PNGIMAGE_HPP */


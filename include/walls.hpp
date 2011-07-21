/**
 * @file walls.hpp
 *
 * @brief Controls the displayed walls.
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

#ifndef _WALLS_HPP
#define _WALLS_HPP 1



#include <vector>

#include "renderable.hpp"



/** @brief Defines a wall.
 */
class Wall {
    public:
        //! @brief The standard world-space to texture-space scaling factor.
        /// Using this value everywhere ensures that all walls will seem to have the same grain fineness.
        /// To make some walls twice as fine, simply take this constant and multiply it by 2.
        static const float STANDARD_TEXTURE_SCALE;
        //! @brief The standard world-space size to tessel number scaling factor.
        /// Using this value everywhere ensures that all walls will seem to have the same quality when looked at from the same distance.
        /// To make some walls twice as fine (tesselation speaking), simply take this constant and multiply it by 2.
        static const float STANDARD_TESSELATION_SCALE;

    protected:
        //! @brief World-space position of the first corner
        Matrix<float,4,1> corner;
        //! @brief World-space translation vector from the first to the second corner
        Matrix<float,3,1> axisA;
        /** @brief World-space translation vector from the first to the fourth corner.
         *
         * The third corner is calculated so that it defines a parallelogram.
         */
        Matrix<float,3,1> axisB;
        //! @brief The world-space size to tessel count scaling factor.
        /// @see Wall::STANDARD_TESSELATION_SCALE
        float tesselationScale;
        //! @brief The world-space to texture-space scaling factor.
        /// @see Wall::STANDARD_TEXTURE_SCALE
        float textureScale;

    public:
        /** @brief Defines a wall as a parallelogram with the given corner and two adjacent sides, as well as tesselation and texturing parameters.
         * @param corner            World-space position of the first corner
         * @param axisA             World-space translation vector from the first to the second corner
         * @param axisB             World-space translation vector from the first to the fourth corner
         * @param tesselationScale  The world-space size to tessel count scaling factor
         * @param textureScale      The world-space to texture-space scaling factor
         */
        Wall(Matrix<float,4,1> corner, Matrix<float,3,1> axisA, Matrix<float,3,1>axisB, float tesselationScale = STANDARD_TESSELATION_SCALE, float textureScale = STANDARD_TEXTURE_SCALE);
        //! @brief Destructor.
        virtual ~Wall();

        //! @brief Returns the world-space position of the first corner
        Matrix<float,4,1> getCorner() const;
        //! @brief Returns the world-space translation vector from the first to the second corner
        Matrix<float,3,1> getAxisA() const;
        //! @brief Returns the world-space translation vector from the first to the fourth corner
        Matrix<float,3,1> getAxisB() const;
        //! @brief Returns the world-space size to tessel count scaling factor
        float getTesselationScale() const;
        //! @brief Returns the world-space to texture-space scaling factor
        float getTextureScale() const;
};



/** @brief Renders a wall.
 *
 * The walls are selectable, so that breaches can be cast onto the clicked wall.
 *
 * Uses a double sided \link TesseledRectangle \endlink for correct lightning.
 *
 * No texture is applied at this level.
 * It should be manager in a parent \link CompositeRenderable \endlink,
 * in order to apply texturing only once, in a batch manner.
 */
class WallRenderer : public SelectableLeafRenderable {
    protected:
        //! @brief Wall to render
        Wall& wall;
        //! @brief Tesseled rectangle used for both rendering and selection
        TesseledRectangle renderRenderable;

    public:
        //! @brief Constructs a wall rendered for the given wall with the given name.
        //! @param wall The wall to render
        //! @param name The name of the wall
        WallRenderer(Wall& wall, GLuint name);
        //! @brief Destructor.
        virtual ~WallRenderer();

        //! @brief Applies material
        virtual void configure(GLenum renderingMode);
        //! @brief Renders the wall
        virtual void render(GLenum renderingMode);
        //! @brief Deconfigure any changed OpenGL state.
        virtual void deconfigure(GLenum renderingMode);
};



//! @brief The defined walls
//! @see initWalls()
extern std::vector<Wall> walls;

//! @brief Renderable for all the walls
//! @see initWalls()
extern IRenderable* wallsRenderer;



//! @brief Initializes \link ::walls \endlink and \link ::wallsRenderer \endlink.
void initWalls(Texture texture);



#endif /*_WALLS_HPP*/

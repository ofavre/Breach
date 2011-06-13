/**
 * @file targets.hpp
 *
 * @brief Controls the displayed targets.
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

#ifndef _TARGETS_HPP
#define _TARGETS_HPP 1



#include <vector>

#include "renderable.hpp"



/** @brief Defines a target.
 */
class Target {
    protected:
        //! @brief The X ordinate of the center of the target
        float x;
        //! @brief The Y ordinate of the center of the target
        float y;
        //! @brief The Z ordinate of the center of the target
        float z;
        //! @brief The size of the target
        float size;
        //! @brief Wether the target has already been hit
        bool hit;

    public:
        /** @brief Constructs a target, usign the given center and size.
         * @param x X coordinate of the center
         * @param y Y coordinate of the center
         * @param z Z coordinate of the center
         * @param size Diameter of the target
         */
        Target(float x, float y, float z, float size);
        //! @brief Destructor.
        virtual ~Target();

        //! @brief Returns the X coordinate of the center
        float getX();
        //! @brief Returns the Y coordinate of the center
        float getY();
        //! @brief Returns the Z coordinate of the center
        float getZ();
        //! @brief Returns the diameter of the target
        float getSize();
        //! @brief Whether the target has been hit
        bool isHit();
        //! @brief Sets the target as hit
        void setHit();
};



/** @brief Renders a target.
 *
 * Uses a double sided tesseled rectangle with alpha test and no blending.
 * Uses a polygon with many sides for selection, and disabled culling.
 *
 * If the target is hit, nothing is rendered.
 */
class TargetRenderer : public SelectableRenderable {
    protected:
        //! @brief The target to render
        Target& target;
        //! @brief The tesseled rectangle used for rendering (for correct lightning).
        TesseledRectangle renderRenderable;
        //! @brief The polygon used for selection (as alpha test is not enabled in selection mode).
        RegularPolygon selectionRenderable;

    public:
        //! @brief Constructs a target renderer with the given name and target.
        //! @param target Target to render
        //! @param name   Name of the selectable target
        TargetRenderer(Target& target, GLuint name);
        //! @brief Destructor.
        virtual ~TargetRenderer();

        //! @brief Returns the target to render.
        Target& getTarget();

        //! @brief Configures alpha test, material, and disables alpha channel writing, for rendering.
        //!        Simply disables culling, for selection.
        virtual void configure(GLenum renderingMode);
        //! @brief Use a double sided \link TesseledRectangle \endlink for rendering,
        //!        and a \link RegularPolygon \endlink for selection.
        virtual void render(GLenum renderingMode);
        //! @brief Deconfigures the changed OpenGL states
        virtual void deconfigure(GLenum renderingMode);
};



//! @brief The defined targets
//! @see initTargets()
extern std::vector<Target> targets;

//! @brief Renderable for all the unshot targets
//! @see initTargets()
extern IRenderable* targetsRenderer;



//! @brief Initializes \link ::targets \endlink and \link ::targetsRenderer \endlink.
void initTargets(Texture texture);



#endif /*_TARGETS_HPP*/
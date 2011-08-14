/**
 * @file crosshair.hpp
 *
 * @brief Controls the crosshair.
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

#ifndef _CROSSHAIR_HPP
#define _CROSSHAIR_HPP 1



#include "breaches.hpp"
#include "renderable.hpp"



class Crosshair {
    protected:
        std::vector<Breach*> breaches;
        int count;

    public:
        Crosshair();
        Breach* addBreach(Breach& breach, unsigned int position);
        Breach* removeBreach(unsigned int position);
        int getBreachCount();
        Breach* getBreachAt(unsigned int position);
};

class CrosshairRenderer : public LeafRenderable {
    protected:
        //! @brief The crosshair to be rendered
        Crosshair& crosshair;
        //! @brief Width of the crosshair (the texture one)
        int width;
        //! @brief Height of the crosshair (the texture one)
        int height;
        //! @brief Window width, an always updated value
        int& windowWidth;
        //! @brief Window height, an always updated value
        int& windowHeight;
        //! @brief Texture for the pointer
        Texture pointerTexture;
        //! @brief Texture for each breach indicator
        Texture breachTexture;

    public:
        CrosshairRenderer(Crosshair& crosshair, int width, int height, int& windowWidth, int& windowHeight, const Texture& pointerTexture, const Texture& breachTexture);
        virtual ~CrosshairRenderer();

        virtual void render(GLenum renderingMode);
};



#endif /*_CROSSHAIR_HPP*/

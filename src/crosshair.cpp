/**
 * @file crosshair.cpp
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

#include "crosshair.hpp"



Crosshair::Crosshair()
: count(0)
{
}

/**
 * @brief Adds a breach at the specified position,
 * returning the old one.
 * @param breach The new breach to be set at the given position
 * @param position The position the breach will occupy
 * @return The old breach at the given position. Can be NULL.
 */
Breach* Crosshair::addBreach(Breach& breach, unsigned int position)
{
    if (position >= breaches.size())
        breaches.resize(position+1, NULL);
    Breach* old;
    if (position >= breaches.size())
        old = NULL;
    else
        old = breaches[position];
    if (old == NULL) count++;
    breaches[position] = &breach;
    return old;
}

/**
 * @brief Removes the breach at the specified position,
 * returning it.
 * @param position The position of the breach to remove.
 * @return The removed breach. Can be NULL.
 */
Breach* Crosshair::removeBreach(unsigned int position)
{
    if (position >= breaches.size())
        return NULL;
    Breach* old = breaches[position];
    if (old != NULL) {
        breaches[position] = NULL;
        count--;
    }
    return old;
}

/**
 * @brief Returns the current count of valid breaches held.
 * Does not count empty positions.
 */
int Crosshair::getBreachCount()
{
    return count;
}

/**
 * @brief Returns the breach at the given position.
 * @param position The position of the breach to return.
 * @return The breach lying at the given position. Can be NULL.
 */
Breach* Crosshair::getBreachAt(unsigned int position)
{
    if (position >= breaches.size())
        return NULL;
    return breaches[position];
}



CrosshairRenderer::CrosshairRenderer(Crosshair& crosshair, int width, int height, int& windowWidth, int& windowHeight, const Texture& pointerTexture, const Texture& breachTexture)
: crosshair(crosshair)
, width(width)
, height(height)
, windowWidth(windowWidth)
, windowHeight(windowHeight)
, pointerTexture(pointerTexture)
, breachTexture(breachTexture)
{
}

CrosshairRenderer::~CrosshairRenderer()
{
}

void CrosshairRenderer::render(GLenum renderingMode)
{
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, pointerTexture.getName());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(1,1,1,1);
    float x = windowWidth/2;
    float y = windowHeight/2;
    glBegin(GL_QUADS);
    x -= width/2;
    y -= height/2;
    glTexCoord2f(0,0);
    glVertex2f(x,y);
    x += width;
    glTexCoord2f(1,0);
    glVertex2f(x,y);
    y += height;
    glTexCoord2f(1,1);
    glVertex2f(x,y);
    x -= width;
    glTexCoord2f(0,1);
    glVertex2f(x,y);
    glEnd();

    if (crosshair.getBreachCount() > 0) {
        glBindTexture(GL_TEXTURE_2D, breachTexture.getName());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
        float texXs[4] = {0, 1, 1, 0};
        float texYs[4] = {0, 0, 1, 1};

        for (int i = 0 ; i < 4 ; i++) {
            Breach* breach = crosshair.getBreachAt(i);
            if (breach == NULL || !breach->isOpened()) continue;
            glColor4fv(breach->getColor().values);
            float x = windowWidth/2;
            float y = windowHeight/2;
            glBegin(GL_QUADS);
            x -= width/2;
            y -= height/2;
            glTexCoord2f(texXs[(i+0)%4],texYs[(i+0)%4]);
            glVertex2f(x,y);
            x += width;
            glTexCoord2f(texXs[(i+1)%4],texYs[(i+1)%4]);
            glVertex2f(x,y);
            y += height;
            glTexCoord2f(texXs[(i+2)%4],texYs[(i+2)%4]);
            glVertex2f(x,y);
            x -= width;
            glTexCoord2f(texXs[(i+3)%4],texYs[(i+3)%4]);
            glVertex2f(x,y);
            glEnd();
        }
    }

    glDisable(GL_BLEND);
    glBindTexture(GL_TEXTURE_2D, Texture::NO_TEXTURE.getName());
    glDisable(GL_TEXTURE_2D);
}


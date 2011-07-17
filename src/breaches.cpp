/**
 * @file breaches.cpp
 *
 * @brief Controls the breaches.
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

#include "breaches.hpp"

using namespace std;



std::vector<Breach> breaches;

IRenderable* breachesRenderer;



const float Breach::DEFAULT_BREACH_WIDTH = 0.4;
const float Breach::DEFAULT_BREACH_HEIGHT = 0.4;

Breach::Breach(bool opened, Matrix<float,4,4> transformation)
: opened(opened)
, transformation(transformation)
{
}

Breach::~Breach()
{
}

Matrix<float,4,4> Breach::getTransformation() const
{
    return transformation;
}

bool Breach::isOpened() const
{
    return opened;
}



BreachRenderer::BreachRenderer(Breach& breach, GLuint name, Texturer& texturer, Texturer& highlightTexturer)
: SelectableLeafRenderable(name, Any().set(breach))
, MatrixTransformerRenderable(breach.getTransformation(), MatrixTransformerRenderable::MODELVIEW)
, breach(breach)
, texturer(texturer)
, highlightTexturer(highlightTexturer)
, renderRenderable(Matrix<float,4,1>((float[]){1,1,0,0}), MatrixHelper::unitAxisVector<float>(0)*-2, MatrixHelper::unitAxisVector<float>(1)*-2, 10, 10, (Rect){0,0,-1,-1}, false)
{
}

BreachRenderer::~BreachRenderer()
{
}

void BreachRenderer::render(GLenum renderingMode)
{
    if (!breach.isOpened() || renderingMode != GL_RENDER) return;
    // Hidden highlight
    {
        highlightTexturer.configure(renderingMode);
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_FALSE);
        // Draw the outline of the hidden breach
        GLfloat mat_ambiant[] = { 10, 5, 0, 1 }; // FIXME Strange to be obliged to set to a vector not normalized to get the right color!
        GLfloat mat_diffuse[] = { 10, 5, 0, 1 };
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, mat_ambiant);
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, mat_diffuse);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonOffset(0, -10);

        // Appear on top of occulting objects
        glDisable(GL_CULL_FACE);
        glDepthFunc(GL_GREATER);
        renderRenderable.fullRender(renderingMode);
        glDepthFunc(GL_LESS);

        // Appear directly onto the porting wall (only when seen from the cull face)
        glEnable(GL_CULL_FACE);
        glCullFace(GL_FRONT);
        renderRenderable.fullRender(renderingMode);
        glCullFace(GL_BACK);

        glPolygonOffset(0, 0);
        glDisable(GL_POLYGON_OFFSET_FILL);
        glPopMatrix();
        glDisable(GL_BLEND);
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        highlightTexturer.deconfigure(renderingMode);
    } //! Hidden highlight
}



void initBreaches(Texture texture, Texture highlight)
{
    breaches.push_back(Breach(true,  Matrix<float,4,4>((float[]){Breach::DEFAULT_BREACH_WIDTH,0,0,0, 0,Breach::DEFAULT_BREACH_HEIGHT,0,0, 0,0,1,0, -.5,.5,-2,1})));
    breaches.push_back(Breach(false, Matrix<float,4,4>()));

    TexturerCompositeRenderable* breachTexturer = new TexturerCompositeRenderable(texture);
    TexturerCompositeRenderable* breachHighlightTexturer = new TexturerCompositeRenderable(highlight);
    SelectableCompositeRenderable* selectable = new SelectableCompositeRenderable(3, Any()); //3=breaches
    GLuint name = 1;
    for (vector<Breach>::iterator it = breaches.begin() ; it < breaches.end() ; it++) {
        selectable->components.push_back(new BreachRenderer(*it, name, *breachTexturer, *breachHighlightTexturer));
        name++;
    }
    breachesRenderer = selectable;
}

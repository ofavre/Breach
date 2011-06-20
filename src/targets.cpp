/**
 * @file targets.cpp
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

#include "targets.hpp"

using namespace std;



vector<Target> targets;

IRenderable* targetsRenderer = NULL;



Target::Target(float x, float y, float z, float size)
: x(x)
, y(y)
, z(z)
, size(size)
, hit(false)
{
}

Target::~Target()
{
}

float Target::getX()
{
    return x;
}

float Target::getY()
{
    return y;
}

float Target::getZ()
{
    return z;
}

float Target::getSize()
{
    return size;
}

bool Target::isHit()
{
    return hit;
}

void Target::setHit()
{
    hit = true;
}



TargetRenderer::TargetRenderer(Target& target, GLuint name)
: SelectableRenderable(name)
, target(target)
, renderRenderable(Matrix<float,4,1>((float[]){target.getX()-target.getSize()/2, target.getY()-target.getSize()/2, target.getZ(), 1}), MatrixHelper::unitRotationAxisVector<float>(0)*target.getSize(), MatrixHelper::unitRotationAxisVector<float>(1)*target.getSize(), 10, 10, (Rect){0,0,1,1}, true)
, selectionRenderable(Matrix<float,4,1>((float[]){target.getX(), target.getY(), target.getZ(), 1}), MatrixHelper::unitRotationAxisVector<float>(0)*target.getSize()/2.045, MatrixHelper::unitRotationAxisVector<float>(1)*target.getSize()/2.045, 20)
{
}

TargetRenderer::~TargetRenderer()
{
}

Target& TargetRenderer::getTarget()
{
    return target;
}

void TargetRenderer::configure(GLenum renderingMode)
{
    if (target.isHit()) return;
    SelectableRenderable::configure(renderingMode);
    if (renderingMode == GL_RENDER) {
        glEnable(GL_ALPHA_TEST);
        glAlphaFunc(GL_GREATER, 0.75f);
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_FALSE);

        GLfloat mat_ambiant[] = { 1, 1, 1, 1 }; // default: .2,.2,.2,1
        GLfloat mat_diffuse[] = { 1, 1, 1, 1 }; // default: .8,.8,.8,1
        GLfloat mat_specular[] = { 0, 0, 0, 1 }; // default: 0,0,0,1
        GLfloat mat_shininess = 0; // default: 0, range [0-128]
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, mat_ambiant);
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, mat_diffuse);
        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat_specular);
        glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, mat_shininess);
    } else if (renderingMode == GL_SELECT) {
        glDisable(GL_CULL_FACE);
    }
}

void TargetRenderer::render(GLenum renderingMode)
{
    if (target.isHit()) return;
    switch (renderingMode) {
        case GL_FEEDBACK:
        case GL_RENDER:
            renderRenderable.fullRender(renderingMode);
            break;
        case GL_SELECT:
            selectionRenderable.fullRender(renderingMode);
            break;
    }
}

void TargetRenderer::deconfigure(GLenum renderingMode)
{
    if (target.isHit()) return;
    SelectableRenderable::deconfigure(renderingMode);
    if (renderingMode == GL_RENDER) {
        glDisable(GL_ALPHA_TEST);
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    } else if (renderingMode == GL_SELECT) {
        glEnable(GL_CULL_FACE);
    }
}



void initTargets(Texture texture)
{
    targets.push_back(Target( 0.0,  0.0, -4.0, 4.0));
    targets.push_back(Target( 0.0,  0.0, -1.0, 0.4));
    targets.push_back(Target( 0.0,  0.0,  0.1, 0.4));
    targets.push_back(Target( 0.0,  0.0, -0.5, 0.4));
    targets.push_back(Target( 0.6,  0.3,  1.0, 0.4));
    targets.push_back(Target( 0.5,  0.7,  0.5, 0.4));
    targets.push_back(Target( 0.3,  0.6, -0.5, 0.4));
    targets.push_back(Target( 0.8,  0.2, -1.0, 0.4));
    targets.push_back(Target( 0.6, -0.3,  1.0, 0.4));
    targets.push_back(Target( 0.5, -0.7,  0.5, 0.4));
    targets.push_back(Target( 0.3, -0.6, -0.5, 0.4));
    targets.push_back(Target( 0.8, -0.2, -1.0, 0.4));
    targets.push_back(Target(-0.6,  0.3,  1.0, 0.4));
    targets.push_back(Target(-0.5,  0.7,  0.5, 0.4));
    targets.push_back(Target(-0.3,  0.6, -0.5, 0.4));
    targets.push_back(Target(-0.8,  0.2, -1.0, 0.4));
    targets.push_back(Target(-0.6, -0.3,  1.0, 0.4));
    targets.push_back(Target(-0.5, -0.7,  0.5, 0.4));
    targets.push_back(Target(-0.3, -0.6, -0.5, 0.4));
    targets.push_back(Target(-0.8, -0.2, -1.0, 0.4));

    //TODO Create classes to manage the targets and the renderables
    //     The topmost renderable should add a name hierarchy (ID_TARGETS/id_target_1, ...)

    TexturerCompositeRenderable* targetsTexturer = new TexturerCompositeRenderable(texture);
    SelectableCompositeRenderable* selectable = new SelectableCompositeRenderable(1); //1=targets
    GLuint name = 1;
    for (vector<Target>::iterator it = targets.begin() ; it < targets.end() ; it++) {
        selectable->components.push_back(new TargetRenderer(*it, name));
        name++;
    }
    targetsTexturer->components.push_back(selectable);
    targetsRenderer = targetsTexturer;
}

/**
 * @file walls.cpp
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

#include "walls.hpp"

using namespace std;



const float Wall::STANDARD_TEXTURE_SCALE = 2;
const float Wall::STANDARD_TESSELATION_SCALE = 10;

vector<Wall> walls;

IRenderable* wallsRenderer = NULL;



Wall::Wall(Matrix<float,4,1> corner, Matrix<float,3,1> axisA, Matrix<float,3,1>axisB, float tesselationScale /*= STANDARD_TESSELATION_SCALE*/, float textureScale /*= STANDARD_TEXTURE_SCALE*/)
: corner(corner)
, axisA(axisA)
, axisB(axisB)
, tesselationScale(tesselationScale)
, textureScale(textureScale)
{
}

Wall::~Wall()
{
}

Matrix<float,4,1> Wall::getCorner()
{
    return corner;
}

Matrix<float,3,1> Wall::getAxisA()
{
    return axisA;
}

Matrix<float,3,1> Wall::getAxisB()
{
    return axisB;
}

float Wall::getTesselationScale()
{
    return tesselationScale;
}

float Wall::getTextureScale()
{
    return textureScale;
}



WallRenderer::WallRenderer(Wall& wall, GLuint name)
: SelectableRenderable(name)
, wall(wall)
, renderRenderable(wall.getCorner(), wall.getAxisA(), wall.getAxisB(), wall.getAxisA().norm()*wall.getTesselationScale(), wall.getAxisB().norm()*wall.getTesselationScale(), (Rect){0,0,wall.getAxisA().norm()*wall.getTextureScale(),wall.getAxisB().norm()*wall.getTextureScale()}, true)
{
}

WallRenderer::~WallRenderer()
{
}

void WallRenderer::configure(GLenum renderingMode)
{
    SelectableRenderable::configure(renderingMode);
    if (renderingMode == GL_RENDER) {
        GLfloat mat_ambiant[] = { 1, 1, 1, 1 }; // default: .2,.2,.2,1
        GLfloat mat_diffuse[] = { 1, 1, 1, 1 }; // default: .8,.8,.8,1
        GLfloat mat_specular[] = { 1, 1, 1, 1 }; // default: 0,0,0,1
        GLfloat mat_shininess = 40; // default: 0, range [0-128]
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, mat_ambiant);
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, mat_diffuse);
        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat_specular);
        glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, mat_shininess);
    }
}

void WallRenderer::render(GLenum renderingMode)
{
    renderRenderable.fullRender(renderingMode);
}

void WallRenderer::deconfigure(GLenum renderingMode)
{
    SelectableRenderable::deconfigure(renderingMode);
}



void initWalls(Texture texture)
{
    walls.push_back(Wall(Matrix<float,4,1>((float[]){ 1,-1, 2,1}), Matrix<float,3,1>((float[]){-2,0,0}), Matrix<float,3,1>((float[]){0,2,0})));
    walls.push_back(Wall(Matrix<float,4,1>((float[]){-1,-1,-2,1}), Matrix<float,3,1>((float[]){ 2,0,0}), Matrix<float,3,1>((float[]){0,2,0})));

    walls.push_back(Wall(Matrix<float,4,1>((float[]){-1,-1,-2,1}), Matrix<float,3,1>((float[]){0,0, 4}), Matrix<float,3,1>((float[]){2,0,0})));
    walls.push_back(Wall(Matrix<float,4,1>((float[]){-1, 1, 2,1}), Matrix<float,3,1>((float[]){0,0,-4}), Matrix<float,3,1>((float[]){2,0,0})));
    walls.push_back(Wall(Matrix<float,4,1>((float[]){-1,-1, 2,1}), Matrix<float,3,1>((float[]){0,0,-4}), Matrix<float,3,1>((float[]){0,2,0})));
    walls.push_back(Wall(Matrix<float,4,1>((float[]){ 1,-1,-2,1}), Matrix<float,3,1>((float[]){0,0, 4}), Matrix<float,3,1>((float[]){0,2,0})));

    Texturer* wallsTexturer = new Texturer(texture);
    SelectableRenderable* selectable = new SelectableRenderable(2); //2=walls
    GLuint name = 1;
    for (vector<Wall>::iterator it = walls.begin() ; it < walls.end() ; it++) {
        selectable->components.push_back(new WallRenderer(*it, name));
        name++;
    }
    wallsTexturer->components.push_back(selectable);
    wallsRenderer = wallsTexturer;
}

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



Wall::Wall(Matrix<float,4,1> corner, Matrix<float,4,1> axisA, Matrix<float,4,1>axisB, float tesselationScale /*= STANDARD_TESSELATION_SCALE*/, float textureScale /*= STANDARD_TEXTURE_SCALE*/)
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

Matrix<float,4,1> Wall::getCorner() const
{
    return corner;
}

Matrix<float,4,1> Wall::getAxisA() const
{
    return axisA;
}

Matrix<float,4,1> Wall::getAxisB() const
{
    return axisB;
}

float Wall::getTesselationScale() const
{
    return tesselationScale;
}

float Wall::getTextureScale() const
{
    return textureScale;
}

Matrix<float,4,1> Wall::projectOnto(Matrix<float,4,1> point) const
{
    Matrix<float,4,1> pt = point - corner;
    Matrix<float,1,4> ptT (pt.values);
    float aNorm = axisA.norm();
    float bNorm = axisB.norm();
    Matrix<float,4,1> axisA4 ((float[]){axisA[0], axisA[1], axisA[2], 1});
    Matrix<float,4,1> axisB4 ((float[]){axisB[0], axisB[1], axisB[2], 1});
    float a = (ptT * axisA4/aNorm/aNorm)[0];
    float b = (ptT * axisB4/bNorm/bNorm)[0];
    Matrix<float,4,1> rtn = corner + axisA4 * a + axisB4 * b;
    return rtn;
}

Matrix<float,2,1> Wall::inWallCoordinates(Matrix<float,4,1> point) const
{
    Matrix<float,4,1> pt = point - corner;
    Matrix<float,1,4> ptT (pt.values);
    float aNorm = axisA.norm();
    float bNorm = axisB.norm();
    Matrix<float,4,1> axisA4 ((float[]){axisA[0], axisA[1], axisA[2], 1});
    Matrix<float,4,1> axisB4 ((float[]){axisB[0], axisB[1], axisB[2], 1});
    float a = (ptT * axisA4/aNorm/aNorm)[0];
    float b = (ptT * axisB4/bNorm/bNorm)[0];
    Matrix<float,2,1> rtn ((float[]){a,b});
    return rtn;
}



WallRenderer::WallRenderer(Wall& wall, GLuint name)
: SelectableLeafRenderable(name, Any().set(wall))
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
    walls.push_back(Wall(Matrix<float,4,1>((float[]){-1,-1,-2,1}), Matrix<float,4,1>((float[]){ 2,0,0,1}), Matrix<float,4,1>((float[]){0,2,0,1})));
    walls.push_back(Wall(Matrix<float,4,1>((float[]){ 1,-1, 2,1}), Matrix<float,4,1>((float[]){-2,0,0,1}), Matrix<float,4,1>((float[]){0,2,0,1})));

    walls.push_back(Wall(Matrix<float,4,1>((float[]){-1,-1,-2,1}), Matrix<float,4,1>((float[]){0,0, 4,1}), Matrix<float,4,1>((float[]){2,0,0,1})));
    walls.push_back(Wall(Matrix<float,4,1>((float[]){-1, 1, 2,1}), Matrix<float,4,1>((float[]){0,0,-4,1}), Matrix<float,4,1>((float[]){2,0,0,1})));
    walls.push_back(Wall(Matrix<float,4,1>((float[]){-1,-1, 2,1}), Matrix<float,4,1>((float[]){0,0,-4,1}), Matrix<float,4,1>((float[]){0,2,0,1})));
    walls.push_back(Wall(Matrix<float,4,1>((float[]){ 1,-1,-2,1}), Matrix<float,4,1>((float[]){0,0, 4,1}), Matrix<float,4,1>((float[]){0,2,0,1})));

    TexturerCompositeRenderable* wallsTexturer = new TexturerCompositeRenderable(texture);
    SelectableCompositeRenderable* selectable = new SelectableCompositeRenderable(2, Any()); //2=walls
    GLuint name = 1;
    for (vector<Wall>::iterator it = walls.begin() ; it < walls.end() ; it++) {
        selectable->components.push_back(new WallRenderer(*it, name));
        name++;
    }
    wallsTexturer->components.push_back(selectable);
    wallsRenderer = wallsTexturer;
}

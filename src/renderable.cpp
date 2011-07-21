/**
 * @file renderable.cpp
 *
 * @brief Interfaces for OpenGL.
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

#include "renderable.hpp"

#include <cfloat>

using namespace std;



void IRenderable::fullRender(GLenum renderingMode)
{
    configure(renderingMode);
    loadTransform(renderingMode);
    render(renderingMode);
    unloadTransform(renderingMode);
    deconfigure(renderingMode);
}

void IRenderable::configure(GLenum renderingMode)
{
}

void IRenderable::loadTransform(GLenum renderingMode)
{
}

void IRenderable::unloadTransform(GLenum renderingMode)
{
}

void IRenderable::deconfigure(GLenum renderingMode)
{
}



SelectableRenderable::SelectableRenderable(GLuint name, Any payload)
: name(name)
, payload(payload)
{
}

SelectableRenderable::~SelectableRenderable()
{
}

GLuint SelectableRenderable::getName()
{
    return name;
}

Any SelectableRenderable::getPayload()
{
    return payload;
}

void SelectableRenderable::configure(GLenum renderingMode)
{
    if (renderingMode == GL_SELECT) {
        glPushName(name);
    }
}

void SelectableRenderable::deconfigure(GLenum renderingMode)
{
    if (renderingMode == GL_SELECT) {
        glPopName();
    }
}



CompositeRenderable::CompositeRenderable()
: IRenderable()
, components()
{
}

CompositeRenderable::~CompositeRenderable()
{
    //TODO check safety
    //IMPORTANT: Update the documentation to reflect the fact that items are being freed or not
    /*for (vector<IRenderable*>::iterator it = components.begin() ; it < components.end() ; it++) {
        //delete(*it);
    }
    components.clear();*/
}

void CompositeRenderable::render(GLenum renderingMode)
{
    for (vector<IRenderable*>::iterator it = components.begin() ; it < components.end() ; it++) {
        IRenderable* target = *it;
        target->fullRender(renderingMode);
    }
}

bool CompositeRenderable::accept(HierarchicalVisitor<IRenderable>& visitor)
{
    if (visitor.visitEnter(this)) {
        for (vector<IRenderable*>::iterator it = components.begin() ; it < components.end() ; ++it) {
            if (!(*it)->accept(visitor)) break;
        }
        return visitor.visitLeave(this);
    } else {
        return false;
    }
}



LeafRenderable::LeafRenderable()
{
}

LeafRenderable::~LeafRenderable()
{
}

bool LeafRenderable::accept(HierarchicalVisitor<IRenderable>& visitor)
{
    return visitor.visitLeaf(this);
}





ConfigurerRenderable::ConfigurerRenderable()
{
}

ConfigurerRenderable::~ConfigurerRenderable()
{
}



TransformerRenderable::TransformerRenderable()
{
}

TransformerRenderable::~TransformerRenderable()
{
}



MatrixTransformerRenderable::MatrixTransformerRenderable(Matrix<float,4,4> transformation, MatrixMode matrixMode)
: matrixMode(matrixMode)
, transformation(transformation)
{
}

MatrixTransformerRenderable::MatrixTransformerRenderable(Matrix<float,4,4>& transformation, MatrixMode matrixMode)
: matrixMode(matrixMode)
, transformation(transformation)
{
}

MatrixTransformerRenderable::MatrixTransformerRenderable(Matrix<float,4,1> offset, Matrix<float,4,1> axisX, Matrix<float,4,1> axisY, MatrixMode matrixMode)
: matrixMode(matrixMode)
, transformation(computeTransformationMatrix(offset,axisX,axisY))
{
}

MatrixTransformerRenderable::~MatrixTransformerRenderable()
{
}

Matrix<float,4,4> MatrixTransformerRenderable::computeTransformationMatrix(Matrix<float,4,1> offset, Matrix<float,4,1> axisX, Matrix<float,4,1> axisY)
{
    Matrix<float,4,1> axisZ = axisX * axisY;
    axisZ = axisZ / axisZ.norm(); // important for the normals
    return Matrix<float,4,4>((float[]){axisX[0],axisX[1],axisX[2],0, axisY[0],axisY[1],axisY[2],0, axisZ[0],axisZ[1],axisZ[2],0, offset[0], offset[1], offset[2], 1});
}

MatrixTransformerRenderable::MatrixMode MatrixTransformerRenderable::getMatrixMode()
{
    return matrixMode;
}

Matrix<float,4,4> MatrixTransformerRenderable::getTransformation()
{
    return transformation;
}

void MatrixTransformerRenderable::loadTransform(GLenum renderingMode)
{
    glMatrixMode(matrixMode);
    glPushMatrix();
    glMultMatrixf(transformation.values);
}

void MatrixTransformerRenderable::unloadTransform(GLenum renderingMode)
{
    glMatrixMode(matrixMode);
    glPopMatrix();
}



const Texture Texture::NO_TEXTURE (0);

Texture::Texture(GLuint name)
: name(name)
, minFilter(LINEAR)
, magFilter(LINEAR)
, wrapS(REPEAT)
, wrapT(REPEAT)
{
}

Texture::Texture(GLuint name, GLint internalFormat, GLsizei width, GLsizei height, GLenum format, const void *pixels)
: name(name)
, minFilter(LINEAR)
, magFilter(LINEAR)
, wrapS(REPEAT)
, wrapT(REPEAT)
{
    glBindTexture(GL_TEXTURE_2D, name);
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, static_cast<const GLvoid*>(pixels));
    // Unbind the texture
    glBindTexture(GL_TEXTURE_2D, Texture::NO_TEXTURE.getName());
}

GLuint Texture::getName() const
{
    return name;
}

Texture::Filter Texture::getMinFilter() const
{
    return minFilter;
}

void Texture::setMinFilter(Filter value)
{
    minFilter = value;
}

Texture::Filter Texture::getMagFilter() const
{
    return magFilter;
}

void Texture::setMagFilter(Filter value)
{
    magFilter = value;
}

Texture::Wrap Texture::getWrapS() const
{
    return wrapS;
}

void Texture::setWrapS(Wrap value)
{
    wrapS = value;
}

Texture::Wrap Texture::getWrapT() const
{
    return wrapT;
}

void Texture::setWrapT(Wrap value)
{
    wrapT = value;
}



Texturer::Texturer(const Texture& texture)
: texture(texture)
{
}

Texturer::~Texturer()
{
}

const Texture Texturer::getTexture() const
{
    return texture;
}

void Texturer::configure(GLenum renderingMode)
{
    if (renderingMode == GL_SELECT) return;
    if (texture.getName() != Texture::NO_TEXTURE.getName()) {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, texture.getName());
    }
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, texture.getMinFilter());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, texture.getMagFilter());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, texture.getWrapS());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, texture.getWrapT());
}

void Texturer::deconfigure(GLenum renderingMode)
{
    if (renderingMode == GL_SELECT) return;
    glBindTexture(GL_TEXTURE_2D, Texture::NO_TEXTURE.getName());
    glDisable(GL_TEXTURE_2D);
}



TesseledRectangle::TesseledRectangle(unsigned int xSteps, unsigned int ySteps, const Rect textureOffsetAndSize, bool doubleSided)
: MatrixTransformerRenderable(MatrixHelper::identity<float>())
, doubleSided(doubleSided)
, xSteps(xSteps)
, ySteps(ySteps)
, textureOffsetAndSize(textureOffsetAndSize)
{
}

TesseledRectangle::TesseledRectangle(Matrix<float,4,1> offset, Matrix<float,4,1> axisX, Matrix<float,4,1> axisY, unsigned int xSteps, unsigned int ySteps, const Rect textureOffsetAndSize, bool doubleSided)
: MatrixTransformerRenderable(offset, axisX, axisY)
, doubleSided(doubleSided)
, xSteps(xSteps)
, ySteps(ySteps)
, textureOffsetAndSize(textureOffsetAndSize)
{
}

TesseledRectangle::~TesseledRectangle()
{
}

void TesseledRectangle::render(GLenum renderingMode)
{
    doRender(renderingMode, false);
    if (doubleSided) {
        glCullFace(GL_FRONT);
        doRender(renderingMode, true);
        glCullFace(GL_BACK);
    }
}

void TesseledRectangle::doRender(GLenum renderingMode, bool reverseNormal)
{
    glNormal3f(0,0,reverseNormal ? -1 : 1);
    glBegin(GL_QUADS);
    switch (renderingMode) {
        case GL_RENDER:{
            float dx = 1.0f / xSteps;
            float dy = 1.0f / ySteps;
            float dtx = textureOffsetAndSize.width  / (float)xSteps;
            float dty = textureOffsetAndSize.height / (float)ySteps;
            float _x = 0;
            float _y = 0;
            float _tx = textureOffsetAndSize.x;
            float _ty = textureOffsetAndSize.y;
            for (unsigned int s1 = 0 ; s1 < ySteps ; s1++, _y += dy, _ty += dty) {
                float __x = 0;
                float __y = 0;
                float __tx = 0;
                float __ty = 0;
                for (unsigned int s2 = 0 ; s2 < xSteps ; s2++, __x += dx, __tx += dtx) {
                    glTexCoord2f(textureOffsetAndSize.x + _tx+__tx    , textureOffsetAndSize.y + _ty+__ty    );
                    glVertex3f(_x+__x   , _y+__y   , 0);
                    glTexCoord2f(textureOffsetAndSize.x + _tx+__tx+dtx, textureOffsetAndSize.y + _ty+__ty    );
                    glVertex3f(_x+__x+dx, _y+__y   , 0);
                    glTexCoord2f(textureOffsetAndSize.x + _tx+__tx+dtx, textureOffsetAndSize.y + _ty+__ty+dty);
                    glVertex3f(_x+__x+dx, _y+__y+dy, 0);
                    glTexCoord2f(textureOffsetAndSize.x + _tx+__tx    , textureOffsetAndSize.y + _ty+__ty+dty);
                    glVertex3f(_x+__x   , _y+__y+dy, 0);
                }
            }
            break;}
        case GL_FEEDBACK:
        case GL_SELECT:
            glVertex3f(0,0,0);
            glVertex3f(1,0,0);
            glVertex3f(1,1,0);
            glVertex3f(0,1,0);
            break;
    }
    glEnd();
}



RegularPolygon::RegularPolygon(unsigned int sides)
: MatrixTransformerRenderable(MatrixHelper::identity<float>())
, sides(sides)
{
}

RegularPolygon::RegularPolygon(Matrix<float,4,1> centerOffset, Matrix<float,4,1> axisX, Matrix<float,4,1> axisY, unsigned int sides)
: MatrixTransformerRenderable(MatrixTransformerRenderable::computeTransformationMatrix(centerOffset,axisX,axisY))
, sides(sides)
{
}

RegularPolygon::~RegularPolygon()
{
}

void RegularPolygon::render(GLenum renderingMode)
{
    float stepSize = 2*M_PI / sides;
    glBegin(GL_TRIANGLE_FAN);
    // Center
    glTexCoord2f(0.5, 0.5);
    glVertex3f(0, 0, 0);
    // First point
    glTexCoord2f(1, 0.5);
    glVertex3d(1, 0, 0);
    for (float angle = stepSize ; angle < 2.0f * M_PI ; angle += stepSize) {
        glTexCoord2f(0.5+cos(angle)/2, 0.5+sin(angle)/2);
        glVertex3d(cos(-angle), sin(-angle), 0);
    }
    // Proprely close the polygon (prevent numerical calculus errors: sum of n times "X/n" is likely to be ≠ from X (but not very far away))
    // This way we prevent the shape from being a bit opened (almost closed)
    glTexCoord2f(1, 0.5);
    glVertex3d(1, 0, 0);
    glEnd();
}

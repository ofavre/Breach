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



// From main.cpp
// TODO: Create a separate header for the player
extern Matrix<float,4,1> playerInclinaison;



std::vector<Breach> breaches;

IRenderable* breachesRenderer;



const float Breach::DEFAULT_BREACH_WIDTH = 0.8;
const float Breach::DEFAULT_BREACH_HEIGHT = 0.8;

Matrix<float,2,1> Breach::getAdjustedShotPoint(const Wall& wall, const Matrix<float,2,1> shotPoint)
{
    float x = shotPoint[0];
    float y = shotPoint[1];
    float aNorm = wall.getAxisA().norm();
    float bNorm = wall.getAxisB().norm();
    if (x - DEFAULT_BREACH_WIDTH /2/aNorm < 0) x =     DEFAULT_BREACH_WIDTH /2/aNorm;
    if (x + DEFAULT_BREACH_WIDTH /2/aNorm > 1) x = 1 - DEFAULT_BREACH_WIDTH /2/aNorm;
    if (y - DEFAULT_BREACH_HEIGHT/2/bNorm < 0) y =     DEFAULT_BREACH_HEIGHT/2/bNorm;
    if (y + DEFAULT_BREACH_HEIGHT/2/bNorm > 1) y = 1 - DEFAULT_BREACH_HEIGHT/2/bNorm;
    if (x < 0 || y < 0) return shotPoint;
    return Matrix<float,2,1>((float[]){x, y});
}

Matrix<float,4,4> Breach::getTransformationFromWall(const Wall& wall, const Matrix<float,2,1> shotPoint)
{
    Matrix<float,4,1> a = wall.getAxisA();
    Matrix<float,4,1> b = wall.getAxisB();
    Matrix<float,4,1> c = wall.getCorner();
    Matrix<float,4,1> z = a * b;
    z = z / z.norm();
    Matrix<float,4,1> t = c + a*shotPoint[0] + b*shotPoint[1];
    a = a / a.norm();
    b = b / b.norm();
    Matrix<float,1,4> upT (playerInclinaison.values);
    upT = upT / upT.norm();
    a(3,0) = b(3,0) = upT(0,3) = 0; // merely for the following vector product
    float upA = (upT * a)[0];
    float upB = (upT * b)[0];
    a = a * (DEFAULT_BREACH_WIDTH /2);
    b = b * (DEFAULT_BREACH_HEIGHT/2);
    Matrix<float,4,4> rtn ((float[]){a[0],a[1],a[2],0, b[0],b[1],b[2],0, z[0],z[1],z[2],0, t[0],t[1],t[2],1});
    float upAngle = -atan2(upA,upB);
    rtn = rtn * MatrixHelper::rotation(upAngle, MatrixHelper::unitAxisVector<float>(2));
    return rtn;
}

bool Breach::shootBreach(unsigned int index, const Wall& wall, Matrix<float,2,1> shotPoint)
{
    if (index >= breaches.size())
        return false;
    Matrix<float,2,1> adjustedShotPoint = getAdjustedShotPoint(wall, shotPoint);
    // Check for overlapping
    float aNorm = wall.getAxisA().norm();
    float bNorm = wall.getAxisB().norm();
    float minDist = (DEFAULT_BREACH_WIDTH*DEFAULT_BREACH_WIDTH + DEFAULT_BREACH_HEIGHT*DEFAULT_BREACH_HEIGHT) / 2 * 0.9;
    for (unsigned int i = 0 ; i < breaches.size() ; i++) {
        if (i == index) continue; // ignore current reshot breach
        if (&wall != breaches[i].getWall()) continue; // ignore other walls
        float dist = 0;
        dist += pow(aNorm*(adjustedShotPoint[0] - breaches[i].getShotPoint()[0]), 2);
        dist += pow(bNorm*(adjustedShotPoint[1] - breaches[i].getShotPoint()[1]), 2);
        if (dist < minDist)
            return false;
    }
    breaches[index] = Breach(true, wall, breaches[index].getColor(), adjustedShotPoint);
    return true;
}

Breach::Breach(Matrix<float,4,1> color)
: opened(false)
, wall(NULL)
, color(color)
{
}

Breach::Breach(bool opened, const Wall& wall, Matrix<float,4,1> color, Matrix<float,2,1> shotPoint) //Matrix<float,4,4> transformation)
: opened(opened)
, wall(&wall)
, color(color)
, shotPoint(shotPoint)
, transformation(getTransformationFromWall(wall, shotPoint)) //transformation)
{
}

Breach::~Breach()
{
}

bool Breach::isOpened() const
{
    return opened;
}

const Wall* Breach::getWall() const
{
    return wall;
}

Matrix<float,4,1> Breach::getColor() const
{
    return color;
}

Matrix<float,2,1> Breach::getShotPoint() const
{
    return shotPoint;
}

Matrix<float,4,4> Breach::getTransformation() const
{
    return transformation;
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

void BreachRenderer::loadTransform(GLenum renderingMode)
{
    transformation = breach.getTransformation();
    MatrixTransformerRenderable::loadTransform(renderingMode);
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
        glDisable(GL_BLEND);
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        highlightTexturer.deconfigure(renderingMode);
    } //! Hidden highlight
}



void initBreaches(Texture texture, Texture highlight)
{
    breaches.push_back(Breach(Matrix<float,4,1>((float[]){0,0.5,1,1})));
    breaches.push_back(Breach(Matrix<float,4,1>((float[]){1,0.5,0,1})));

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

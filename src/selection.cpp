/**
 * @file selection.cpp
 *
 * @brief Selection management for OpenGL.
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

#include <vector>
#include <algorithm>

#include "selection.hpp"

using namespace std;



SelectionUtil SelectionUtil::finishGlSelection(GLuint* selectionBuffer)
{
    return SelectionUtil(glRenderMode(GL_RENDER), selectionBuffer);
}

SelectionUtil::SelectionUtil(const SelectionUtil& copy)
: hits(copy.hits)
{
}

SelectionUtil::SelectionUtil(GLint resultCount, GLuint* selectionBuffer)
{
    analyzeSelectionBuffer(resultCount, selectionBuffer);
}

SelectionUtil::~SelectionUtil()
{
}

bool SelectionUtil::Hit::operator<(const Hit& other) const
{
    return this->zMin < other.zMin;
}

void SelectionUtil::analyzeSelectionBuffer(GLint resultCount, GLuint* selectionBuffer)
{
    hits.clear();
    hits.reserve(resultCount);

    if (resultCount > 0) {
        GLuint *ptr = selectionBuffer;

        for (int i = 0 ; i < resultCount ; i++) {
            Hit hit;
            GLuint nameCount = ptr[0];
            GLfloat z1 = ptr[1] / (float)0xffffffff;
            GLfloat z2 = ptr[2] / (float)0xffffffff;
            hit.zMin = z1;
            hit.zMax = z2;
            ptr += 3;
            hit.nameHierarchy.reserve(nameCount);
            hit.nameHierarchy.insert(hit.nameHierarchy.begin(), ptr, ptr+nameCount);
            ptr += nameCount;
            hits.push_back(hit);
        }

        std::sort(hits.begin(), hits.end());
    }
}

vector<SelectionUtil::Hit>& SelectionUtil::getHits()
{
    return hits;
}

Any SelectionUtil::getTopMostPayload(IRenderable& sceneRenderable)
{
    if (hits.empty()) return Any();
    SelectionVisitor selectionResolver (hits[0].nameHierarchy);
    sceneRenderable.accept(selectionResolver);
    if (selectionResolver.isSelectedObjectFound()) {
        return selectionResolver.getSelectedObject();
    } else
        return Any().clear();
}

template <class TDesired>
TDesired* SelectionUtil::getTopMostTypedPayload(IRenderable& sceneRenderable)
{
    if (hits.empty()) return NULL;
    TypedSelectionVisitor<TDesired> selectionResolver (hits[0].nameHierarchy);
    sceneRenderable.accept(selectionResolver);
    if (selectionResolver.isSelectedObjectFound()) {
        return selectionResolver.getSelectedObject();
    } else
        return NULL;
}



SelectionVisitor::SelectionVisitor(std::vector<GLuint> desiredName)
: SpecializedHierachicalVisitor<IRenderable>(true, true, true)
, found(false)
, selectedObject()
, desiredName(desiredName)
, currentLevel(0)
{
    addSpecializationEnter((sigc::slot<bool,SelectableCompositeRenderable*>)sigc::mem_fun(this,&SelectionVisitor::visitSelectableEnter));
    addSpecializationLeaf((sigc::slot<bool,SelectableLeafRenderable*>)sigc::mem_fun(this,&SelectionVisitor::visitSelectableLeaf));
    addSpecializationLeave((sigc::slot<bool,SelectableCompositeRenderable*>)sigc::mem_fun(this,&SelectionVisitor::visitSelectableLeave));
}

SelectionVisitor::~SelectionVisitor()
{
}

bool SelectionVisitor::isSelectedObjectFound()
{
    return found;
}

Any SelectionVisitor::getSelectedObject()
{
    return selectedObject;
}

bool SelectionVisitor::visitSelectableEnter(SelectableCompositeRenderable* that)
{
    if (that == NULL) return false;
    if (that->getName() == desiredName[currentLevel]) {
        currentLevel++;
        if (currentLevel < desiredName.size()) {
            return !found;
        } else {
            Any data = that->getPayload();
            if (data.isSet()) {
                found = true;
                selectedObject = data;
            }
            return false;
        }
    } else {
        return false;
    }
}

bool SelectionVisitor::visitSelectableLeaf(SelectableLeafRenderable* that)
{
    if (that == NULL) return false;
    if (that->getName() == desiredName[currentLevel]) {
        currentLevel++;
        if (currentLevel < desiredName.size()) {
            return !found;
        } else {
            Any data = that->getPayload();
            if (data.isSet()) {
                found = true;
                selectedObject = data;
            }
            return !found;
        }
    } else {
        return !found;
    }
}

bool SelectionVisitor::visitSelectableLeave(SelectableCompositeRenderable* that)
{
    return !found;
}

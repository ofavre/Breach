/**
 * @file selection.tcc
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

#ifndef _SELECTION_HPP
#error You should include selection.hpp instead of this file directly
#endif

#ifndef _SELECTION_TCC
#define _SELECTION_TCC 1



#include <vector>

#include "selection.hpp"



template <class TDesired>
SelectionVisitor<TDesired>::SelectionVisitor(std::vector<GLuint> desiredName)
: SpecializedHierachicalVisitor<IRenderable>(true, true, true)
, found(false)
, selectedObject(NULL)
, desiredName(desiredName)
, currentLevel(0)
{
    addSpecializationEnter((sigc::slot<bool,SelectableCompositeRenderable*>)sigc::mem_fun(this,&SelectionVisitor::visitSelectableEnter));
    addSpecializationLeaf((sigc::slot<bool,SelectableLeafRenderable*>)sigc::mem_fun(this,&SelectionVisitor::visitSelectableLeaf));
    addSpecializationLeave((sigc::slot<bool,SelectableCompositeRenderable*>)sigc::mem_fun(this,&SelectionVisitor::visitSelectableLeave));
}

template <class TDesired>
SelectionVisitor<TDesired>::~SelectionVisitor()
{
}

template <class TDesired>
bool SelectionVisitor<TDesired>::isSelectedObjectFound()
{
    return found;
}

template <class TDesired>
TDesired* SelectionVisitor<TDesired>::getSelectedObject()
{
    return selectedObject;
}

template <class TDesired>
bool SelectionVisitor<TDesired>::visitSelectableEnter(SelectableCompositeRenderable* that)
{
    if (that == NULL) return false;
    if (that->getName() == desiredName[currentLevel]) {
        currentLevel++;
        if (currentLevel < desiredName.size()) {
            return !found;
        } else {
            TDesired* data = that->getPayload().get<TDesired>();
            if (data != NULL) {
                found = true;
            }
            return false;
        }
    } else {
        return false;
    }
}

template <class TDesired>
bool SelectionVisitor<TDesired>::visitSelectableLeaf(SelectableLeafRenderable* that)
{
    if (that == NULL) return false;
    if (that->getName() == desiredName[currentLevel]) {
        currentLevel++;
        if (currentLevel < desiredName.size()) {
            return !found;
        } else {
            TDesired* data = that->getPayload().get<TDesired>();
            if (data != NULL) {
                found = true;
                selectedObject = data;
            }
            return !found;
        }
    } else {
        return !found;
    }
}

template <class TDesired>
bool SelectionVisitor<TDesired>::visitSelectableLeave(SelectableCompositeRenderable* that)
{
    return !found;
}

#endif /* _SELECTION_TCC */

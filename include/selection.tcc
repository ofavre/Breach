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



template <class TDesired>
TypedSelectionVisitor<TDesired>::TypedSelectionVisitor(std::vector<GLuint> desiredName)
: SpecializedHierachicalVisitor<IRenderable>(true, true, true)
, found(false)
, selectedObject(NULL)
, desiredName(desiredName)
, currentLevel(0)
{
    addSpecializationEnter((sigc::slot<bool,SelectableCompositeRenderable*>)sigc::mem_fun(this,&TypedSelectionVisitor::visitSelectableEnter));
    addSpecializationLeaf((sigc::slot<bool,SelectableLeafRenderable*>)sigc::mem_fun(this,&TypedSelectionVisitor::visitSelectableLeaf));
    addSpecializationLeave((sigc::slot<bool,SelectableCompositeRenderable*>)sigc::mem_fun(this,&TypedSelectionVisitor::visitSelectableLeave));
}

template <class TDesired>
TypedSelectionVisitor<TDesired>::~TypedSelectionVisitor()
{
}

template <class TDesired>
bool TypedSelectionVisitor<TDesired>::isSelectedObjectFound()
{
    return found;
}

template <class TDesired>
TDesired* TypedSelectionVisitor<TDesired>::getSelectedObject()
{
    return selectedObject;
}

template <class TDesired>
bool TypedSelectionVisitor<TDesired>::visitSelectableEnter(SelectableCompositeRenderable* that)
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
            return false;
        }
    } else {
        return false;
    }
}

template <class TDesired>
bool TypedSelectionVisitor<TDesired>::visitSelectableLeaf(SelectableLeafRenderable* that)
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
bool TypedSelectionVisitor<TDesired>::visitSelectableLeave(SelectableCompositeRenderable* that)
{
    return !found;
}

#endif /* _SELECTION_TCC */

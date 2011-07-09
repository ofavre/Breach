/**
 * @file selection.hpp
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
#define _SELECTION_HPP 1



#include <vector>

#include "visitor.hpp"
#include "renderable.hpp"



class SelectionUtil {
    public:
        struct Hit {
            float zMin;
            float zMax;
            std::vector<GLuint> nameHierarchy;
            bool operator<(const Hit& other) const;
        };

    private:
        std::vector<Hit> hits;

    protected:
        void analyzeSelectionBuffer(GLint resultCount, GLuint* selectionBuffer);

    public:
        static SelectionUtil finishGlSelection(GLuint* selectionBuffer);
        SelectionUtil(const SelectionUtil& copy);
        SelectionUtil(GLint resultCount, GLuint* selectionBuffer);
        virtual ~SelectionUtil();

        std::vector<Hit>& getHits();

        Any getTopMostPayload(IRenderable& sceneRenderable);
        template <class TDesired>
        TDesired* getTopMostTypedPayload(IRenderable& sceneRenderable);
};



class SelectionVisitor : public SpecializedHierachicalVisitor<IRenderable> {
    private:
        bool found;
        Any selectedObject;
        std::vector<GLuint> desiredName;
        unsigned int currentLevel;

        virtual bool visitSelectableEnter(SelectableCompositeRenderable* that);
        virtual bool visitSelectableLeaf(SelectableLeafRenderable* that);
        virtual bool visitSelectableLeave(SelectableCompositeRenderable* that);

    public:
        SelectionVisitor(std::vector<GLuint> desiredName);
        virtual ~SelectionVisitor();

        bool isSelectedObjectFound();
        Any getSelectedObject();
};



template <class TDesired>
class TypedSelectionVisitor : public SpecializedHierachicalVisitor<IRenderable> {
    private:
        bool found;
        TDesired* selectedObject;
        std::vector<GLuint> desiredName;
        unsigned int currentLevel;

        virtual bool visitSelectableEnter(SelectableCompositeRenderable* that);
        virtual bool visitSelectableLeaf(SelectableLeafRenderable* that);
        virtual bool visitSelectableLeave(SelectableCompositeRenderable* that);

    public:
        TypedSelectionVisitor(std::vector<GLuint> desiredName);
        virtual ~TypedSelectionVisitor();

        bool isSelectedObjectFound();
        TDesired* getSelectedObject();
};



#include "selection.tcc"

#endif /* _SELECTION_HPP */

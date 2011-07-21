/**
 * @file breaches.hpp
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

#ifndef _BREACH_HPP
#define _BREACH_HPP 1

#include "matrix.hpp"
#include "renderable.hpp"
#include "walls.hpp"


/**
 * @brief Defines a breach.
 */
class Breach {
    public:
        //! @brief Default breach width
        static const float DEFAULT_BREACH_WIDTH;
        //! @brief Default breach height
        static const float DEFAULT_BREACH_HEIGHT;

    private:
        //! @brief A breach can be either opened or closed (and not yet shot)
        bool opened;
        //! @brief The porting wall. Transformations are relative to it.
        const Wall* wall;
        /**
        * @brief The transformation matrix to apply to translate, orient and scale the breach.
        *
        * After applying this transformation,
        * the breach must be rendered as a (possibly tesseled) quad,
        * using -1/+1 for X and Y, and 0 for Z.
        */
        Matrix<float,4,4> transformation;

        static Matrix<float,2,1> getAdjustedShotPoint     (const Wall& wall, const Matrix<float,2,1> shotPoint);
        static Matrix<float,4,4> getTransformationFromWall(const Wall& wall, const Matrix<float,2,1> shotPoint);

    public:
        Breach(bool opened, const Wall& wall, Matrix<float,2,1> shotPoint);
        Breach(bool opened, const Wall& wall, Matrix<float,4,4> transformation);
        virtual ~Breach();

        Matrix<float,4,4> getTransformation() const;
        bool isOpened() const;
};



/**
 * @brief Renders a breach.
 */
class BreachRenderer : public SelectableLeafRenderable, public MatrixTransformerRenderable {
    protected:
        //! @brief Wall to render
        Breach& breach;
        Texturer& texturer;
        Texturer& highlightTexturer;
        //! @brief Tesseled rectangle used for both rendering hidden highlight
        TesseledRectangle renderRenderable;

    public:
        //! @brief Constructs a breach renderer for the given breach with the given name.
        //! @param wall The breach to render
        //! @param name The name of the breach
        BreachRenderer(Breach& breach, GLuint name, Texturer& texturer, Texturer& highlightTexturer);
        //! @brief Destructor.
        virtual ~BreachRenderer();

        //! @brief Updates the transformation before applying it
        virtual void loadTransform(GLenum renderingMode);
        //! @brief Renders the breach
        virtual void render(GLenum renderingMode);
};



//! @brief The defined breaches
//! @see initBreaches()
extern std::vector<Breach> breaches;

//! @brief Renderable for all the breaches
//! @see initBreaches()
extern IRenderable* breachesRenderer;



//! @brief Initializes \link ::breaches \endlink and \link ::breachesRenderer \endlink.
void initBreaches(Texture texture, Texture highlight);



#endif /*_BREACH_HPP*/

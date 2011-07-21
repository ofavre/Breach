/**
 * @file renderable.hpp
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

#ifndef _RENDERABLE_HPP
#define _RENDERABLE_HPP 1

#include <vector>
#include <GL/gl.h>

#include "matrix.hpp"
#include "visitor.hpp"
#include "any.hpp"



//TODO Selectable name provider/tracker(for fixed names, under a provided one (TARGETS/target_index_0, ...))
//     It should be a recursive structure with a top most singleton instance
//     It should answer to queries:
//      - What is the class type of the instance for name x/y/z, x/y, x (incomplete should be supported)?
//      - Given this hit buffer (to analyzed here only), are any instance of this class type available?
//     It should also provide an instance of utility class that gives access to the analyzed hit buffer.



// Forward declaration for IRenderable's friend declaration
class LeafRenderable;
class CompositeRenderable;
/**
 * @brief Base class of all OpenGL-renderable object.
 *
 * This class defines the guidelines and separates the differents steps
 * involved when rendering an object on screen.
 *
 * In its member functions, \c GLenum \c renderingMode refers to
 * the current OpenGL render mode.
 * It can be any valid value for \c glRenderMode(GLenum): \c GL_RENDER, \c GL_SELECT, \c GL_FEEDBACK.
 * Using this parameter, the renderable object can adjust the pipeline primitives
 * to use while using the same overall rendering process.
 *
 * @remarks
 * For the 5 pipeline virtual functions:
 * An overloading function should always call the base classes'
 * implementation not to break functionnality.
 * The normal philosophy is to add modifications, not to replace them.
 */
class IRenderable : public Visitable<IRenderable> {
    private:
        friend class LeafRenderable;
        friend class CompositeRenderable;
        /** @brief Forbid direct non virtual inheritance of this class, use \link LeafRenderable \endlink or \link CompositeRenderable \endlink instead.
         *
         * The explicit name of this function is a hint of why the compiler is complaining.
         *
         * In order to preserve a clean hierarchy of renderables, only the few friends declared
         * will be able (and have to) implement this dummy function that should to nothing and
         * never be called.
         *
         * It has been made private for the sole purpose of design guidance:
         *  - Deriving this class directly in (forcefully virtual) inheritance
         *    should be used when the derived class provides partial functionnality,
         *    such as implementing only configure/deconfigure or loadTransform/unloadTransform.
         *    The fact that virtual inheritance is obliged is a good thing too, because
         *    it makes no sense to force the utility deriving class to have its own mother class,
         *    even if this base class has no state in itself.
         *    (Multiple base class are used with multiple non-virtual inheritance of the same base class)
         *  - Only LeafRenderable and CompositeRenderable can derive this class and implement
         *    this function. So any non-abstract class should inherit one of those class.
         */
        virtual void PLEASE_USE_LeafRenderable_OR_CompositeRenderable_INSTEAD_OF_IRenderable_DIRECTLY() = 0;
    public:
        //! @brief Invokes successively all the steps for rendering the object.
        //! @param renderingMode The current value of glRenderMode().
        void fullRender(GLenum renderingMode);
        /** @brief Configures necessary OpenGL states.
         *
         * Overload if needed, default implementation does nothing.
         * @param renderingMode The current value of glRenderMode().
         */
        virtual void configure(GLenum renderingMode);
        /** @brief Push matrices and applies any modification onto them.
         *
         * Overload if needed, default implementation does nothing.
         * @param renderingMode The current value of glRenderMode().
         */
        virtual void loadTransform(GLenum renderingMode);
        /** @brief Invokes the render OpenGL primitives.
         *
         * Must be overriden.
         * @param renderingMode The current value of glRenderMode().
         */
        virtual void render(GLenum renderingMode) = 0;
        /** @brief Pops any modified matrices.
         *
         * Overload if needed, default implementation does nothing.
         * @param renderingMode The current value of glRenderMode().
         */
        virtual void unloadTransform(GLenum renderingMode);
        /** @brief Reverts applied configuration to OpenGL states.
         *
         * Overload if needed, default implementation does nothing.
         * @param renderingMode The current value of glRenderMode().
         */
        virtual void deconfigure(GLenum renderingMode);
};



/**
 * @brief Groups renderable to benefit from ease of
 *        a single render call and grouped
 *        configuration and transformation.
 *
 * Many class will prefer deriving this class rather than \link IRenderable \endlink directly.
 * However, if you object is not composed, prefer using \link LeafRenderable \endlink instead.
 *
 * Configuration and transformation are applied before
 * each render component is rendered (using \link IRenderable::fullRender() \endlink).
 *
 * Therefore this class is a good place to factor configuration such as
 * texturing, materials, and transformations that apply to the whole
 * components group globally, where the components may transform again
 * in a relative manner the ones from the others, by example in order to
 * create a composed body.
 *
 * Please note that the components are rendered in order.
 * The vector of components is publicly accessible in order to facilitate
 * necessary manipulations, but you should remain thread-safe and avoid
 * modifying the vector while rendering.
 *
 * @see LeafRenderable
 */
class CompositeRenderable : public virtual IRenderable {
    private:
        //! @brief Implement the inheritance locking function of IRenderable base.
        inline void PLEASE_USE_LeafRenderable_OR_CompositeRenderable_INSTEAD_OF_IRenderable_DIRECTLY() {}
    public:
        /** @brief List of the contained components.
         *
         * They are rendered in order, using \link IRenderable::fullRender() \endlink,
         * after the configuration and transformation have been applied.
         */
        std::vector<IRenderable*> components;

        //! @brief Creates an initially empty composite renderable.
        CompositeRenderable();
        /** @brief Destructor.
         *
         * The items of \link #components \endlink are not freed.
         * Memory management is left to the developer.
         */
        virtual ~CompositeRenderable();
        /** @brief Renders successively all the components, in order.
         *
         * Calls \link IRenderable::fullRender() \endlink on each component.
         * @param renderingMode The current value of glRenderMode().
         */
        virtual void render(GLenum renderingMode);
        //! @copydoc Visitable::accept()
        virtual bool accept(HierarchicalVisitor<IRenderable>& visitor);
};



/**
 * @brief Defines a final object that knows what OpenGL primitives to call.
 *
 * Many class will prefer deriving this class rather than \link IRenderable \endlink directly.
 * However, if you object is composed, prefer using \link ComposedRenderable \endlink instead.
 *
 * Configuration and transformation are applied before
 * each render component is rendered (using \link IRenderable::fullRender() \endlink).
 *
 * Therefore this class is a good place to factor configuration such as
 * texturing, materials, and transformations that apply to the whole
 * components group globally, where the components may transform again
 * in a relative manner the ones from the others, by example in order to
 * create a composed body.
 *
 * @see CompositeRenderable
 */
class LeafRenderable : public virtual IRenderable {
    private:
        //! @brief Implement the inheritance locking function of IRenderable base.
        inline void PLEASE_USE_LeafRenderable_OR_CompositeRenderable_INSTEAD_OF_IRenderable_DIRECTLY() {}
    public:
        //! @brief Creates an leaf renderable to be implemented.
        LeafRenderable();
        //! @brief Destructor.
        virtual ~LeafRenderable();
        virtual void render(GLenum renderingMode) = 0;
        //! @copydoc Visitable::accept()
        virtual bool accept(HierarchicalVisitor<IRenderable>& visitor);
};



/**
 * @brief A variant of a renderable that forces
 *        redefinition of the \link configure() \endlink and
 *        \link deconfigure() \endlink functions.
 *
 * This class merely serves a semantic role and calls its base class's
 * functions for its non pure virtual members.
 */
class ConfigurerRenderable : public virtual IRenderable {
    public:
        ConfigurerRenderable();
        virtual ~ConfigurerRenderable();

        virtual void configure(GLenum renderingMode) = 0;
        virtual void deconfigure(GLenum renderingMode) = 0;
};



/**
 * @brief A variant of a renderable that forces
 *        redefinition of the \link loadTransform() \endlink and
 *        \link unloadTransform() \endlink functions.
 *
 * This class merely serves a semantic role and calls its base class's
 * functions for its non pure virtual members.
 */
class TransformerRenderable : public virtual IRenderable {
    public:
        TransformerRenderable();
        virtual ~TransformerRenderable();

        virtual void loadTransform(GLenum renderingMode) = 0;
        virtual void unloadTransform(GLenum renderingMode) = 0;
};



/**
 * @brief Pushes a name onto OpenGL name stack (for selection), and renders internal components.
 *
 * The \link configure() \endlink and \link deconfigure() \endlink merely pushes
 * and pops a name onto the name stack.
 * This way, every drawn primitive will get selectable under the pushed name.
 *
 * \see glPushName(GLuint)
 * \see glRenderMode(GL_SELECT)
 */
class SelectableRenderable : public ConfigurerRenderable {
    private:
        //! @brief The name to be pushed onto OpenGL name stack.
        GLuint name;
        //! @brief The data object to be retreived through this selectable.
        Any payload;
    public:
        //! @brief Creates a selectable composite renderable with the given name.
        SelectableRenderable(GLuint name, Any payload);
        //! @brief Destructor.
        virtual ~SelectableRenderable();

        //! @brief Returns the name given in the constructor.
        GLuint getName();
        //! @brief Returns the data object associated with the selectable.
        Any getPayload();

        //! @brief Pushes the configured name onto the name stack.
        virtual void configure(GLenum renderingMode);
        //! @brief Pops the configured name from the name stack.
        virtual void deconfigure(GLenum renderingMode);
};

/**
 * @brief A selectable, composed renderable.
 */
class SelectableCompositeRenderable : public CompositeRenderable, public SelectableRenderable {
    public:
        SelectableCompositeRenderable(GLuint name, Any payload)
        : SelectableRenderable(name, payload)
        {}
        virtual ~SelectableCompositeRenderable()
        {}
};
/**
 * @brief A selectable, leaf renderable.
 */
class SelectableLeafRenderable : public LeafRenderable, public SelectableRenderable {
    public:
        SelectableLeafRenderable(GLuint name, Any payload)
        : SelectableRenderable(name, payload)
        {}
        virtual ~SelectableLeafRenderable()
        {}
};




/**
 * @brief A transformer that pushes modifications to one of OpenGL matrix.
 *
 * Given a matrix mode and a transformation matrix, this class pushes the
 * matrix onto the stack, multiplies it by the transformation matrix,
 * renders components, and pops the matrix from the stack.
 *
 * Please note that this class transforms the current matrix by multiplication,
 * in particular, it does not replace it.
 */
class MatrixTransformerRenderable : public TransformerRenderable {
    public:
        //! @brief Proper C++ enum defining legal \link glMatrixMode() \endlink values.
        enum MatrixMode {
            MODELVIEW = GL_MODELVIEW,
            PROJECTION = GL_PROJECTION,
            TEXTURE = GL_TEXTURE,
            COLOR = GL_COLOR
        };
    protected:
        //! @brief OpenGL matrix mode to transform
        MatrixMode matrixMode;
        //! @brief Matrix that will multiply the current OpenGL matrix to transform it.
        Matrix<float,4,4> transformation;
    public:
        //! @brief Constructs a new matrix transformation.
        //! @param transformation   Transformation to apply to OpenGL matrix
        //! @param matrixMode       Specifies which OpenGL matrix mode to transform
        MatrixTransformerRenderable(Matrix<float,4,4> transformation, MatrixMode matrixMode = MODELVIEW);
        //! @brief Constructs a new matrix transformation.
        //! @param transformation   Transformation to apply to OpenGL matrix
        //! @param matrixMode       Specifies which OpenGL matrix mode to transform
        MatrixTransformerRenderable(Matrix<float,4,4>& transformation, MatrixMode matrixMode = MODELVIEW);
        //! @brief Applies a 3D cartesian coordinate system transformation.
        //! @param offset           New origin
        //! @param axisX            New X axis
        //! @param axisY            New Y axis
        //! @param matrixMode       Specifies which OpenGL matrix mode to transform
        //! @see MatrixTransformerRenderable::computeTransformationMatrix()
        MatrixTransformerRenderable(Matrix<float,4,1> offset, Matrix<float,4,1> axisX, Matrix<float,4,1> axisY, MatrixMode matrixMode = MODELVIEW);
        //! @brief Destructor.
        virtual ~MatrixTransformerRenderable();

        /**
         * @brief Calculates the 4D transformation matrix for passing
         *        from the standard orthonormal cartesian 3D coordinate system
         *        to a new 3D cartesian with given X and Y axis.
         *
         * This function is aimed at 2D plane transformation in 3D space,
         * preserving a unit length orthogonal Z component, and positive orientation.
         *
         * The constraints on the Z component makes this transformation suitable
         * for preserving a natural lightning of the transformed primitives.
         *
         * The returned matrix is defined as follows:
         * \f[
               \left(\begin{array}{cccc}
                   X_x   &   Y_x   &   Z_x   &   0   \\
                   X_y   &   Y_y   &   Z_y   &   0   \\
                   X_z   &   Y_z   &   Z_z   &   0   \\
                   0     &   0     &   0     &   1
               \end{array}\right)
           \f]
         * Where \f$ Z = \frac{X \times Y}{\Vert X \times Y \Vert} \f$.
         *
         * @param offset    New origin
         * @param axisX     New X axis
         * @param axisY     New Y axis
         */
        static Matrix<float,4,4> computeTransformationMatrix(Matrix<float,4,1> offset, Matrix<float,4,1> axisX, Matrix<float,4,1> axisY);

        //! @brief Returns the OpenGL matrix mode to transform.
        MatrixMode getMatrixMode();
        //! @brief Returns the transformation matrix.
        Matrix<float,4,4> getTransformation();

        //! @brief Pushes the configured mode matrix and transforms it by multiplication with the configured transformation matrix.
        virtual void loadTransform(GLenum renderingMode);
        //! @brief Pops the configured mode matrix back to its former value.
        virtual void unloadTransform(GLenum renderingMode);
};



/**
 * @brief Represents a texture and its different OpenGL properties.
 */
class Texture {
    public:
        //! @brief Proper C++ enum defining legal filter values for \link glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_*_FILTER, GLenum) \endlink.
        enum Filter {
            //! @brief Take nearest pixel from the texture. Leads to pixelisation.
            NEAREST = GL_NEAREST,
            //! @brief Apply a linear filter to decide the color of the outgoing fragment, looking at neighbor pixels.
            LINEAR = GL_LINEAR
        };
        //! @brief Proper C++ enum defining legal filter values for \link glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_*, GLenum) \endlink.
        enum Wrap {
            //! @brief Clamps to the border
            CLAMP = GL_CLAMP,
            //! @brief Clamps to the edges, without the border
            CLAMP_TO_EDGE = GL_CLAMP_TO_EDGE,
            //! @brief Wrap around the edges
            REPEAT = GL_REPEAT
        };
        //! @brief Special texture representing "no texture" (texture name 0 in OpenGL).
        static const Texture NO_TEXTURE;

    private:
        //! @brief Texture name
        //! \see glGenTextures()
        GLuint name;
        //! @brief Minification filter of the texture. Defaults to \link #LINEAR \endlink.
        Filter minFilter;
        //! @brief Magnification filter of the texture. Defaults to \link #LINEAR \endlink.
        Filter magFilter;
        //! @brief Wrapping for the S coordinate (equivalent to X) of the texture. Defaults to \link #REPEAT \endlink.
        Wrap wrapS;
        //! @brief Wrapping for the T coordinate (equivalent to Y) of the texture. Defaults to \link #REPEAT \endlink.
        Wrap wrapT;

    public:
        /** @brief Constructs a texture, assigning only its name.
         *
         * This constructor lets you call classic OpenGL functions to initialize the texture.
         * @param name Texture name
         */
        Texture(GLuint name);
        //! @brief Constructs a texture, configures it and uploads it to the GPU.
        //! @param name Texture name
        //! @param internalFormat Texture internal format
        //! @param width Texture width
        //! @param height Texture height
        //! @param format Texture format
        //! @param pixels Pointer to the texture's pixels
        //! @see glTexImage2D()
        Texture(GLuint name, GLint internalFormat, GLsizei width, GLsizei height, GLenum format, const void *pixels);

        //! @brief Returns the texture name
        GLuint getName() const;

        //! @brief Returns the current value of the minification filter of the texture
        Filter getMinFilter() const;
        //! @brief Sets the value of the minification filter of the texture
        void setMinFilter(Filter value);
        //! @brief Returns the current value of the magnification filter of the texture
        Filter getMagFilter() const;
        //! @brief Sets the value of the magnification filter of the texture
        void setMagFilter(Filter value);
        //! @brief Returns the current value of the wrapping for the S coordinate (equivalent to X) of the texture
        Wrap getWrapS() const;
        //! @brief Sets the value of the wrapping for the S coordinate (equivalent to X) of the texture
        void setWrapS(Wrap value);
        //! @brief Returns the current value of the wrapping for the T coordinate (equivalent to Y) of the texture
        Wrap getWrapT() const;
        //! @brief Sets the value of the wrapping for the T coordinate (equivalent to Y) of the texture
        void setWrapT(Wrap value);
};



/**
 * @brief Configurer renderable that binds the given texture to the OpenGL context.
 *
 * @see glTexParameterf()
 */
class Texturer : public ConfigurerRenderable {
    private:
        //! @brief The texture to bind to the OpenGL context.
        const Texture texture;

    public:
        //! @brief Constructs a texturer with the given texture to bind to the OpenGL context.
        Texturer(const Texture& texture);
        //! @brief Destructor.
        virtual ~Texturer();

        //! @brief Returns the used texture.
        const Texture getTexture() const;

        //! @brief Binds the configured texture to the OpenGL context.
        virtual void configure(GLenum renderingMode);
        //! @brief Unbinds any texture from the OpenGL context.
        virtual void deconfigure(GLenum renderingMode);
};

/**
 * @brief A texturer, composed renderable.
 */
class TexturerCompositeRenderable : public CompositeRenderable, public Texturer {
    public:
        TexturerCompositeRenderable(const Texture& texture)
        : Texturer(texture)
        {}
        virtual ~TexturerCompositeRenderable()
        {}
};
/**
 * @brief A texturer, leaf renderable.
 */
class TexturerLeafRenderable : public LeafRenderable, public Texturer {
    public:
        TexturerLeafRenderable(const Texture& texture)
        : Texturer(texture)
        {}
        virtual ~TexturerLeafRenderable()
        {}
};



/**
 * @brief Represents a 2D rectangle with an offset and a size.
 *
 * This does not forcefully define a rectangle to be drawn,
 * but rather a 2D offset and size, for any use.
 *
 * When used with textures it may refer to a sub/super portion
 * of a texture to be used.
 *
 * Example:
 * Remember that textures have unit width and height in \c S and \c T
 * coordinate, regardless of the actual image size and ratio.
 * Giving a size of 4 will make the texture to be tiled 4 times.
 * Using \c x=0.75, \c y=0.75, \c width=0.25 and \c height=0.25, will result
 * in the top-right most 16th squared parcel of the texture to
 * be used.
 * Remember that (S,T) coordinates are right-handed.
 *
 * \link TesseledRectangle \endlink uses it to control how the
 * texture is to be applied to the rendered rectangle,
 * like the eventual offset and the wrapping count.
 */
struct Rect {
    //! @brief X offset
    float x;
    //! @brief Y offset
    float y;
    //! @brief Width
    float width;
    //! @brief Height
    float height;
};

/**
 * @brief Renders a tesseled rectangle.
 *
 * Tesselation is useful with lightning without fragment shaders
 * since it adds intermediate steps (triangles) along each axis
 * and the lightning is performed triangle by triangle,
 * or vertex by vertex when smoothed.
 *
 * In selection mode, the rectangle is not being tesseled
 * for speed considerations.
 *
 * Without transformation (or after transformation),
 * the rectangle vertices coordinates will evolve between 0 and 1,
 * except for Z which will stay 0.
 */
class TesseledRectangle : public LeafRenderable, public MatrixTransformerRenderable {
    private:
        //! @brief Whether this rectangle may be seen from the two sides.
        bool doubleSided;
        //! @brief Number of steps to take to go along the X axis.
        unsigned int xSteps;
        //! @brief Number of steps to take to go along the Y axis.
        unsigned int ySteps;
        //! @brief Parameter for texturing the rectangle.
        Rect textureOffsetAndSize;
    protected:
        /** @brief Actual rendering function.
         *
         * Handles drawing the front and back rectangle by flipping the normals.
         * @param reverseNormal Whether or not to reverse normals, for correct lightning.
         * @param renderingMode The current value of glRenderMode().
         */
        virtual void doRender(GLenum renderingMode, bool reverseNormal);
    public:
        /** @brief Constructs a tesseled rectangle of unit length along the X and Y axis, starting at the origin.
         * @param xSteps                Number of steps to take to go along the X axis.
         * @param ySteps                Number of steps to take to go along the Y axis.
         * @param textureOffsetAndSize  Parameter for texturing the rectangle.
         * @param doubleSided           Whether this rectangle may be seen from the two sides.
         *                              Involves two drawings and flipping normals for correct lightning.
         *                              Uses back face culling for proper hiding the hidden side.
         */
        TesseledRectangle(unsigned int xSteps, unsigned int ySteps, const Rect textureOffsetAndSize, bool doubleSided = true);
        /** @brief Constructs a tesseled rectangle of unit length along the given X and Y axis, starting at the given origin.
         * @param offset                New origin
         * @param axisX                 New X axis
         * @param axisY                 New Y axis
         * @param xSteps                Number of steps to take to go along the X axis.
         * @param ySteps                Number of steps to take to go along the Y axis.
         * @param textureOffsetAndSize  Parameter for texturing the rectangle.
         * @param doubleSided           Whether this rectangle may be seen from the two sides.
         *                              Involves two drawings and flipping normals for correct lightning.
         *                              Uses back face culling for proper hiding the hidden side.
         * @see MatrixTransformerRenderable::computeTransformationMatrix
         */
        TesseledRectangle(Matrix<float,4,1> offset, Matrix<float,4,1> axisX, Matrix<float,4,1> axisY, unsigned int xSteps, unsigned int ySteps, const Rect textureOffsetAndSize, bool doubleSided = true);
        //! @brief Destructor.
        virtual ~TesseledRectangle();

        //! @brief Renders the single or double sided, tesseled, (eventually) textured rectangle.
        virtual void render(GLenum renderingMode);
};



/**
 * @brief Renders a regular polygon.
 *
 * Can be used to approximate a circle as a regular many sides polygon.
 *
 * The technique used to draw the shape consists of creating
 * triangle fans, with the first vertex at the center
 * and the others iterating through the corners.
 * Therefore, lightning may not look 100% correct.
 *
 * Without transformation (or after transformation),
 * the polygon vertices coordinates will evolve between -1 and +1,
 * except for Z which will stay 0.
 */
class RegularPolygon : public LeafRenderable, public MatrixTransformerRenderable {
    private:
        //! Number of sides of the polygon
        unsigned int sides;
    public:
        //! @brief Constructs a unit length radius, origin centered, axis aligned, regular polygon.
        //! @param sides        Number of sides of the polygon
        RegularPolygon(unsigned int sides);
        //! @brief Constructs a unit length radius, origin centered, axis aligned, regular polygon, with the given origin and axes
        //! @param centerOffset New origin.
        //! @param axisX        New X axis.
        //! @param axisY        New Y axis.
        //! @param sides        Number of sides of the polygon
        RegularPolygon(Matrix<float,4,1> centerOffset, Matrix<float,4,1> axisX, Matrix<float,4,1> axisY, unsigned int sides);
        //! @brief Destructor.
        virtual ~RegularPolygon();

        //! @brief Renders the 
        virtual void render(GLenum renderingMode);
};



#endif /*_RENDERABLE_HPP*/

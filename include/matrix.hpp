/**
 * @file matrix.hpp
 *
 * @brief Matrix manipulation library.
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

#ifndef _MATRIX_HPP
#define _MATRIX_HPP 1

#include <cassert>
#include <cmath>
#include <cstring> // memcpy
#include <iostream>

/**
 * @brief Represents a matrix.
 *
 * @tparam Value Type of the values contained in the matrix.
 * @tparam lines Number of lines.
 * @tparam cols  Number of columns.
 */
template <typename Value, unsigned int lines, unsigned int cols>
class Matrix {
    public:
        /**
         * @brief Default constructor.
         *
         * Does not initialize the values.
         */
        Matrix();
        /**
         * @brief Constructor that fills the matrix with a single value.
         *
         * @param fill_value Value to be assigned to each cell.
         * @see fill(Value)
         */
        Matrix(Value fill_value);
        /**
         * @brief Constructor that uses the given 2D array to fill the matrix.
         *
         * @param values Column-major 2-dimensionnal array of values to be copied into the matrix.
         * @see take(Value[cols][lines])
         */
        Matrix(const Value values[cols][lines]);
        /**
         * @brief Constructor that uses the given 1D array to fill the matrix.
         *
         * @param values Column-major 1-dimensionnal array of values to be copied into the matrix.
         *               Columns are laying one after the other, with \a lines consecutive values inside each.
         * @see take(Value[cols*lines])
         */
        Matrix(const Value values[cols*lines]);
        /**
         * @brief Constructor that uses the given arguments as a 1D array to fill the matrix.
         *
         * @param enoughValues List of values you would have given as an array to \link Matrix(const Value[cols*lines]) \endlink.
         *                     Please pay attention to the number of values given: exactly \a cols times \a lines will be read.
         *                     Failing to given enough values may result in unexpected behaviors,
         *                     in addition to filling the matrix with unspecified values.
         * @see Matrix(const Value[cols*lines])
         */
        Matrix(Value enoughValues, ...);
        /**
         * @brief Destructor.
         */
        virtual ~Matrix();


        /**
         * @brief Fills the entire matrix with a single value.
         *
         * @param value Value to be copied inside each cell.
         */
        void fill(Value value);
        /**
         * @brief Fills the given values inside the matrix.
         *
         * @param values Column-major 2-dimensionnal array of values to be copied into the matrix.
         */
        void take(const Value values[cols][lines]);
        /**
         * @brief Fills the given values inside the matrix.
         *
         * @param values Column-major 1-dimensionnal array of values to be copied into the matrix.
         *               Columns are laying one after the other, with \a lines consecutive values inside each.
         */
        void take(const Value values[cols*lines]);
        /**
         * @brief Calculates the L2 norm.
         *
         * This function calculates the "full" norm (with all components),
         * as opposed to \link norm() \endlink.
         * It is more likely that the latter is more used, as many 4D vectors are manipulated,
         * hence the names of the two functions.
         *
         * @return Returns the root sum of each cell's squared value, defining the L2 norm of a matrix/vector.
         */
        double normFull() const;
        /**
         * @brief Calculates the L2 norm, forgetting the last component.
         *
         * This is useful with 4D vectors when the last component is always 1,
         * in order to be responsive to 4D matrices (with the translating component).
         *
         * @return Returns the root sum of each cell's squared value, defining the L2 norm of a matrix/vector.
         */
        double norm() const;

        /**
         * @brief Calculates the product of the current matrix by another matrix.
         *
         * @param secondOperand Second operand of the matrix product.
         * @return A new matrix defined by the matrix product of \c this by \a secondOperand.
         */
        template <unsigned int finalCols>
        Matrix<Value,lines,finalCols> operator*(const Matrix<Value,cols,finalCols> &secondOperand) const;
        /**
         * @brief Calculates the product of the current matrix by a scalar value.
         *
         * @param scalar A scalar value.
         * @return A new matrix defined by the scalar multiplication of \c this by \a scalar.
         */
        Matrix<Value,lines,cols> operator*(Value scalar) const;
        /**
         * @brief Calculates the division of the current matrix by a scalar value.
         *
         * @param scalar A scalar value.
         * @return A new matrix defined by the scalar division of \c this by \a scalar.
         */
        Matrix<Value,lines,cols> operator/(Value scalar) const;
        /**
         * @brief Calculates the addition of the current matrix by a scalar value.
         *
         * @param scalar A scalar value.
         * @return A new matrix defined by the scalar addition of \c this and \a scalar.
         */
        Matrix<Value,lines,cols> operator+(Value scalar) const;
        /**
         * @brief Calculates the subtraction of the current matrix by a scalar value.
         *
         * @param scalar A scalar value.
         * @return A new matrix defined by the scalar subtraction of \c this by \a scalar.
         */
        Matrix<Value,lines,cols> operator-(Value scalar) const;

        /**
         * @brief Calculates the addition of the current matrix by another matrix.
         *
         * @param secondOperand Second operand of the matrix addition.
         * @return A new matrix defined by the matrix addition of \c this and \a secondOperand.
         */
        Matrix<Value,lines,cols> operator+(const Matrix<Value,lines,cols> &secondOperand) const;
        /**
         * @brief Calculates the subtraction of the current matrix by another matrix.
         *
         * @param secondOperand Second operand of the matrix subtraction.
         * @return A new matrix defined by the matrix subtraction of \c this by \a secondOperand.
         */
        Matrix<Value,lines,cols> operator-(const Matrix<Value,lines,cols> &secondOperand) const;

        /**
         * @brief Fetches a given cell of the matrix.
         *
         * @param line 0-based line index
         * @param col  0-based column index
         * @return A copy of the value of the cell laying at line \a line and column \a col.
         */
        Value operator()(unsigned int line, unsigned int col) const;
        /**
         * @brief Fetches a given cell of the matrix.
         *
         * @param line 0-based line index
         * @param col  0-based column index
         * @return A writeable reference to the value of the cell laying at line \a line and column \a col.
         */
        Value& operator()(unsigned int line, unsigned int col);
        /**
         * @brief Fetches a given cell of the matrix.
         *
         * Index is 0-based and the cells are ordered in column-major order.
         * In matrix having \c L lines and \c C columns,
         * the index \code c*L+l \endcode refers to the cell at line \c l and column \c c.
         *
         * @param index 0-based cell index
         * @return A copy of the value of the cell laying at line \a line and column \a col.
         */
        Value operator[](unsigned int index) const;
        /**
         * @brief Fetches a given cell of the matrix.
         *
         * Index is 0-based and the cells are ordered in column-major order.
         * In matrix having \c L lines and \c C columns,
         * the index \code c*L+l \endcode refers to the cell at line \c l and column \c c.
         *
         * @param index 0-based cell index
         * @return A writeable reference to the cell laying at line \a line and column \a col.
         */
        Value& operator[](unsigned int index);

        /**
         * @brief Public access to the inner values of the matrix.
         *
         * They are stored in column-major order.
         * Columns are laying one after the other, with \c lines consecutive values inside each.
         */
        Value values[cols*lines];
};

/**
 * @brief Helper namespace, easing the use of recurrent manipulations with \link Matrix \endlink.
 */
namespace MatrixHelper {
    /**
     * @brief Generates a 4x4, 3D rotation matrix.
     *
     * @param angle Amount of rotation to be used.
     * @param axis  Axis of the rotation, as a 4D vector.
     *              The last component should be equal to 1, and is ignored.
     * @return A 4x4 rotation matrix defined as
     *         \f[
     *                    \left(\begin{array}{cccc}
     *                        x^2 \left( 1 - c \right) + c   &   xy  \left( 1 - c \right) - zs    &   xz  \left( 1 - c \right) + ys    &   0   \\
     *                        xy  \left( 1 - c \right) + zs  &   y^2 \left( 1 - c \right) + c     &   yz  \left( 1 - c \right) - xs    &   0   \\
     *                        xz  \left( 1 - c \right) - ys  &   yz  \left( 1 - c \right) + xs    &   z^2 \left( 1 - c \right) + c     &   0   \\
     *                        0                              &   0                                &   0                                &   1
     *                    \end{array}\right)
     *                \f]
     *          Where \f$ c = cos(\textnormal{angle}) \f$, \f$ s = sin(\textnormal{angle}) \f$ and \f$ \textnormal{axis} = \left( x , y , z \right) \f$.
     *          The vector \a axis is normalized before use.
     */
    template <typename Value>
    Matrix<Value,4,4> rotation(double angle, const Matrix<Value,4,1> &axis);
    /**
     * @brief Generates a 4x4, 3D translation matrix.
     *
     * @param x Amount to translate in the X axis.
     * @param y Amount to translate in the Y axis.
     * @param z Amount to translate in the Z axis.
     * @return A 4x4 translation matrix defined as
     *         \f[
                    \left(\begin{array}{cccc}
                        0   &   0   &   0   &   x   \\
                        0   &   0   &   0   &   y   \\
                        0   &   0   &   0   &   z   \\
                        0   &   0   &   0   &   1
                    \end{array}\right)
                \f]
     */
    template <typename Value>
    Matrix<Value,4,4> translation(Value x, Value y, Value z);
    /**
     * @brief Generates a 4x4, 3D translation matrix.
     *
     * @param vector 4D translation vector.
     *               The last component should be equal to 1, and is ignored.
     * @see MatrixHelper::translation(Value x, Value y, Value z)
     */
    template <typename Value>
    Matrix<Value,4,4> translation(const Matrix<Value,4,1> &vector);
    /**
     * @brief Generates a 4x4, 3D scaling matrix.
     *
     * @param x Scaling factor in the X axis.
     * @param y Scaling factor in the Y axis.
     * @param z Scaling factor in the Z axis.
     * @return A 4x4 scaling matrix defined as
     *         \f[
                    \left(\begin{array}{cccc}
                        x   &   0   &   0   &   0   \\
                        0   &   y   &   0   &   0   \\
                        0   &   0   &   z   &   0   \\
                        0   &   0   &   0   &   1
                    \end{array}\right)
                \f]
     */
    template <typename Value>
    Matrix<Value,4,4> scaling(Value x, Value y, Value z);
    /**
     * @brief Generates a 4x4, 3D scaling matrix.
     *
     * @param vector 4D scaling vector.
     *               The last component should be equal to 1, and is ignored.
     * @see MatrixHelper::scaling(Value x, Value y, Value z)
     */
    template <typename Value>
    Matrix<Value,4,4> scaling(const Matrix<Value,4,1> &vector);
    /**
     * @brief Generates a 4x4, identity transformation matrix.
     */
    template <typename Value>
    Matrix<Value,4,4> identity();
    /**
     * @brief Pretty-prints the given matrix in the specified output stream.
     *
     * @param matrix Matrix to pretty-print
     * @param out    Output stream to use
     */
    template <typename Value, unsigned int lines, unsigned int cols>
    void print(const Matrix<Value,lines,cols> matrix, std::ostream& out = std::cout);
    /**
     * @brief Generates a unit vector, for the desired axis.
     *
     * @param axis Axis whose value should be equal to one unit
     * @return A vector like \f$ (1,0,0)^\top \f$, \f$ (0,1,0)^\top \f$ or \f$ (0,0,1)^\top \f$.
     */
    template <typename Value>
    Matrix<Value,3,1> unitRotationAxisVector(unsigned int axis);
    /**
     * @brief Generates a unit vector, for the desired axis.
     *
     * @param axis Axis whose value should be equal to one unit
     * @return A vector like \f$ (1,0,0,1)^\top \f$, \f$ (0,1,0,1)^\top \f$ or \f$ (0,0,1,1)^\top \f$.
     *         Note that the fourth component is always 1.
     * @see MatrixHelper::unitRotationAxisVector(unsigned int axis)
     */
    template <typename Value>
    Matrix<Value,4,1> unitAxisVector(unsigned int axis);
}



/**
 * @brief Calculates the vectorial product of two 4D vectors.
 *
 * A 4D vector is like a 3D vector, with an additionnal fourth component equal to 1.
 *
 * @param u The first operand
 * @param v The second operand
 * @return A 4D vector defined as
 *         \f[
                \left(\begin{array}{c}
                    u_{1,0} v_{2,0}  -  u_{2,0} v_{1,0}   \\
                    u_{2,0} v_{0,0}  -  u_{0,0} v_{2,0}   \\
                    u_{0,0} v_{1,0}  -  u_{1,0} v_{0,0}
                \end{array}\right)
           \f]
 */
template <typename Value>
Matrix<Value,4,1> operator* (Matrix<Value,4,1> u, Matrix<Value,4,1> v);



#include "matrix.tcc"

#endif /* _MATRIX_HPP */

/**
 * @file matrix.tcc
 *
 * @brief Matrix manipulation library, template code.
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
#error You should include matrix.hpp instead of this file directly
#endif

#ifndef _MATRIX_TCC
#define _MATRIX_TCC 1



#include <cstdarg>



template <typename Value, unsigned int lines, unsigned int cols>
Matrix<Value,lines,cols>::Matrix()
{
}

template <typename Value, unsigned int lines, unsigned int cols>
Matrix<Value,lines,cols>::Matrix(Value fill_value)
{
    fill(fill_value);
}

template <typename Value, unsigned int lines, unsigned int cols>
Matrix<Value,lines,cols>::Matrix(const Value values[cols][lines])
{
    take(values);
}

template <typename Value, unsigned int lines, unsigned int cols>
Matrix<Value,lines,cols>::Matrix(Value enoughValues, ...)
{
    va_list args;
    va_start(args, enoughValues);
    Value* ptr = values;
    *ptr = enoughValues;
    ptr++;
    for (unsigned int i = lines*cols-1 ; ; --i) {
        *ptr = va_arg(args, Value);
        ptr++;
        if (i == 0) break;
    }
    va_end(args);
}

template <typename Value, unsigned int lines, unsigned int cols>
Matrix<Value,lines,cols>::Matrix(const Value values[cols*lines])
{
    take(values);
}

template <typename Value, unsigned int lines, unsigned int cols>
Matrix<Value,lines,cols>::~Matrix()
{
}


template <typename Value>
Matrix<Value,4,4> MatrixHelper::rotation(double angle, const Matrix<Value,4,1> &axis)
{
    /*
     *  / x²(1−c)+c  xy(1−c)−zs xz(1−c)+ys 0 \
     * |  yx(1−c)+zs y²(1−c)+c  yz(1−c)−xs 0  |
     * |  xz(1−c)−ys yz(1−c)+xs z²(1−c)+c  0  |
     *  \     0          0          0      1 /
     * Where c=cos(angle), s=sin(angle), and (x,y,z) is normalized.
    */
    Matrix<Value,4,1> normalizedAxis = axis / axis.norm();
    normalizedAxis[3] = 1; // don't touch the last component
    Matrix<Value,4,4> rtn;
    rtn.fill(static_cast<Value>(1));
    for (unsigned int i = 0 ; i < 3 ; ++i) rtn(3,i) = rtn(i,3) = static_cast<Value>(0);
    rtn(3,3) = static_cast<Value>(1);
    double c = cos(angle);
    double s = sin(angle);
    double oneMinusC = 1-c;
    for (unsigned int i = 0 ; i < 3 ; ++i) {
        rtn(0,i) *= normalizedAxis(i,0) * oneMinusC;
        rtn(1,i) *= normalizedAxis(i,0) * oneMinusC;
        rtn(2,i) *= normalizedAxis(i,0) * oneMinusC;
        rtn(i,0) *= normalizedAxis(i,0) * oneMinusC;
        rtn(i,1) *= normalizedAxis(i,0) * oneMinusC;
        rtn(i,2) *= normalizedAxis(i,0) * oneMinusC;
        rtn(i,i) += c;
    }
    rtn(0,1) -= normalizedAxis(2,0) * s;
    rtn(1,0) += normalizedAxis(2,0) * s;
    rtn(0,2) += normalizedAxis(1,0) * s;
    rtn(2,0) -= normalizedAxis(1,0) * s;
    rtn(2,1) += normalizedAxis(0,0) * s;
    rtn(1,2) -= normalizedAxis(0,0) * s;
    return rtn;
}

template <typename Value>
Matrix<Value,4,4> MatrixHelper::translation(Value x, Value y, Value z)
{
    return MatrixHelper::translation<Value>(Matrix<Value,4,1>((Value[4]){x,y,z,1}));
}
template <typename Value>
Matrix<Value,4,4> MatrixHelper::translation(const Matrix<Value,4,1> &vector)
{
    /*
     *  / 1 0 0 x \
     * |  0 1 0 y  |
     * |  0 0 1 z  |
     *  \ 0 0 0 1 /
     */
    Matrix<Value,4,4> rtn (static_cast<Value>(0));
    for (unsigned int i = 0 ; i < 3 ; ++i) {
        rtn(i,3) = vector(i,0);
        rtn(i,i) = static_cast<Value>(1);
    }
    rtn(3,3) = static_cast<Value>(1);
    return rtn;
}
template <typename Value>
Matrix<Value,4,4> MatrixHelper::scaling(Value x, Value y, Value z)
{
    return MatrixHelper::scaling<Value>(Matrix<Value,4,1>((Value[4]){x,y,z,1}));
}
template <typename Value>
Matrix<Value,4,4> MatrixHelper::scaling(const Matrix<Value,4,1> &vector)
{
    /*
     *  / x 0 0 0 \
     * |  0 y 0 0  |
     * |  0 0 z 0  |
     *  \ 0 0 0 1 /
     */
    Matrix<Value,4,4> rtn (static_cast<Value>(0));
    for (unsigned int i = 0 ; i < 3 ; ++i) {
        rtn(i,i) = vector(i,0);
    }
    rtn(3,3) = static_cast<Value>(1);
    return rtn;
}
template <typename Value>
Matrix<Value,4,4> MatrixHelper::identity()
{
    Matrix<Value,4,4> rtn;
    rtn.fill(static_cast<Value>(0));
    for (unsigned int i = 0 ; i < 4 ; ++i) {
        rtn(i,i) = static_cast<Value>(1);
    }
    return rtn;
}

template <typename Value, unsigned int lines, unsigned int cols>
void MatrixHelper::print(const Matrix<Value,lines,cols> matrix, std::ostream& out)
{
    /* Outputs something like:
     *  / x x x x \
     * |  x x x x  |
     * |  x x x x  |
     *  \ x x x x /
     */
    for (unsigned int l = 0 ; l < lines ; ++l) {
        out << (l == 0 ? "/" : l == lines-1 ? "\\" : "|") << " ";
        for (unsigned int c = 0 ; c < cols ; ++c) {
            // Single space separation
            if (c > 0) out << " ";
            // Fixed width values
            out.width(12);
            // Space pre-filled values
            out.fill(' ');
            out << matrix(l,c);
        }
        out << " " << (l == 0 ? "\\" : l == lines-1 ? "/" : "|") << std::endl;
    }
}

template <typename Value>
Matrix<Value,4,1> MatrixHelper::unitAxisVector(unsigned int axis)
{
    assert(axis < 3);
    Matrix<Value,4,1> rtn;
    rtn.fill(static_cast<Value>(0));
    rtn(axis,0) = static_cast<Value>(1);
    // Always set the fourth component to 1
    rtn(3,0) = static_cast<Value>(1);
    return rtn;
}

template <typename Value>
Matrix<Value,3,1> MatrixHelper::unitRotationAxisVector(unsigned int axis)
{
    assert(axis < 3);
    Matrix<Value,3,1> rtn;
    rtn.fill(static_cast<Value>(0));
    rtn(axis,0) = static_cast<Value>(1);
    return rtn;
}


template <typename Value, unsigned int lines, unsigned int cols>
void Matrix<Value,lines,cols>::fill(Value value)
{
    for (unsigned int i = lines*cols-1 ; ; i--) {
        values[i] = value;
        if (i == 0) break;
    }
}

template <typename Value, unsigned int lines, unsigned int cols>
void Matrix<Value,lines,cols>::take(const Value values[cols][lines])
{
    for (unsigned int l = lines-1 ; ; --l) {
        for (unsigned int c = cols-1 ; ; --c) {
            this(l,c) = values[c][l];
            if (c == 0) break;
        }
        if (l == 0) break;
    }
}

template <typename Value, unsigned int lines, unsigned int cols>
void Matrix<Value,lines,cols>::take(const Value values[cols*lines])
{
    memcpy(this->values, values, sizeof(Value)*lines*cols);
}

template <typename Value, unsigned int lines, unsigned int cols>
double Matrix<Value,lines,cols>::normFull() const
{
    double rtn = 0;
    for (unsigned int i = lines*cols-1 ; ; i--) {
        rtn += values[i] * values[i];
        if (i == 0) break;
    }
    return sqrt(rtn);
}

template <typename Value, unsigned int lines, unsigned int cols>
double Matrix<Value,lines,cols>::norm() const
{
    double rtn = 0;
    for (unsigned int i = lines*cols-1-1 ; ; i--) {
        rtn += values[i] * values[i];
        if (i == 0) break;
    }
    return sqrt(rtn);
}


template <typename Value, unsigned int lines, unsigned int cols>
template <unsigned int finalCols>
Matrix<Value,lines,finalCols> Matrix<Value,lines,cols>::operator*(const Matrix<Value,cols,finalCols> &b) const
{
    Matrix<Value,lines,finalCols> rtn;
    rtn.fill(static_cast<Value>(0));
    for (unsigned int l = lines-1 ; ; --l) {
        for (unsigned int c = finalCols-1 ; ; --c) {
            for (unsigned int i = cols-1 ; ; --i) {
                rtn(l,c) += this->operator()(l,i) * b(i,c);
                if (i == 0) break;
            }
            if (c == 0) break;
        }
        if (l == 0) break;
    }
    return rtn;
}

template <typename Value, unsigned int lines, unsigned int cols>
Matrix<Value,lines,cols> Matrix<Value,lines,cols>::operator+(const Matrix<Value,lines,cols> &b) const
{
    Matrix<Value,lines,cols> rtn (*this);
    for (unsigned int i = lines*cols-1 ; ; --i) {
        rtn.values[i] += b.values[i];
        if (i == 0) break;
    }
    return rtn;
}

template <typename Value, unsigned int lines, unsigned int cols>
Matrix<Value,lines,cols> Matrix<Value,lines,cols>::operator-(const Matrix<Value,lines,cols> &b) const
{
    Matrix<Value,lines,cols> rtn (*this);
    for (unsigned int i = lines*cols-1 ; ; --i) {
        rtn.values[i] -= b.values[i];
        if (i == 0) break;
    }
    return rtn;
}


template <typename Value, unsigned int lines, unsigned int cols>
Matrix<Value,lines,cols> Matrix<Value,lines,cols>::operator*(Value b) const
{
    Matrix<Value,lines,cols> rtn (*this);
    for (unsigned int i = lines*cols-1 ; ; --i) {
        rtn.values[i] *= b;
        if (i == 0) break;
    }
    return rtn;
}

template <typename Value, unsigned int lines, unsigned int cols>
Matrix<Value,lines,cols> Matrix<Value,lines,cols>::operator/(Value b) const
{
    Matrix<Value,lines,cols> rtn (*this);
    for (unsigned int i = lines*cols-1 ; ; --i) {
        rtn.values[i] /= b;
        if (i == 0) break;
    }
    return rtn;
}

template <typename Value, unsigned int lines, unsigned int cols>
Matrix<Value,lines,cols> Matrix<Value,lines,cols>::operator+(Value b) const
{
    Matrix<Value,lines,cols> rtn (*this);
    for (unsigned int i = lines*cols-1 ; ; --i) {
        rtn.values[i] += b;
        if (i == 0) break;
    }
    return rtn;
}

template <typename Value, unsigned int lines, unsigned int cols>
Matrix<Value,lines,cols> Matrix<Value,lines,cols>::operator-(Value b) const
{
    Matrix<Value,lines,cols> rtn (*this);
    for (unsigned int i = lines*cols-1 ; ; --i) {
        rtn.values[i] -= b;
        if (i == 0) break;
    }
    return rtn;
}


template <typename Value, unsigned int lines, unsigned int cols>
Value Matrix<Value,lines,cols>::operator()(unsigned int line, unsigned int col) const
{
    assert(line < lines);
    assert(col  < cols );
    return this->values[col*lines+line];
}

template <typename Value, unsigned int lines, unsigned int cols>
Value& Matrix<Value,lines,cols>::operator()(unsigned int line, unsigned int col)
{
    assert(line < lines);
    assert(col  < cols );
    return this->values[col*lines+line];
}

template <typename Value, unsigned int lines, unsigned int cols>
Value Matrix<Value,lines,cols>::operator[](unsigned int index) const
{
    assert(index < lines*cols);
    return values[index];
}

template <typename Value, unsigned int lines, unsigned int cols>
Value& Matrix<Value,lines,cols>::operator[](unsigned int index)
{
    assert(index < lines*cols);
    return values[index];
}



template <typename Value>
Matrix<Value,4,1> operator* (Matrix<Value,4,1> u, Matrix<Value,4,1> v)
{
    return Matrix<Value,4,1>((Value[4]){u(1,0)*v(2,0)-u(2,0)*v(1,0),u(2,0)*v(0,0)-u(0,0)*v(2,0),u(0,0)*v(1,0)-u(1,0)*v(0,0),1});
}



#endif /* _MATRIX_TCC */
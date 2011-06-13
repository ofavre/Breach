/**
 * @file matrix_test.cpp
 *
 * @brief Unit tests for the matrix library.
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

#include "matrix.hpp"

#include <iostream>
#include <cassert>
#include <cmath>

/**
 * @brief Executes unit tests for the Matrix library.
 */
int main() {
    Matrix<int,3,1> vector3(20,200,2000);
    int vector3_values[3] = { 20, 200, 2000 };
    assert(vector3(0,0) == vector3_values[0]);
    assert(vector3(1,0) == vector3_values[1]);
    assert(vector3(2,0) == vector3_values[2]);

    Matrix<int,4,1> vector4;
    int vector4_values[4] = { 5, 50, 500, 1 };
    vector4.take(vector4_values);
    assert(vector4(0,0) == vector4_values[0]);
    assert(vector4(1,0) == vector4_values[1]);
    assert(vector4(2,0) == vector4_values[2]);
    assert(vector4(3,0) == vector4_values[3]);

    Matrix<int,4,4> translation = MatrixHelper::translation(vector3);
    assert(translation(0,0) == 1);
    assert(translation(1,1) == 1);
    assert(translation(2,2) == 1);
    assert(translation(0,1) == 0);
    assert(translation(0,2) == 0);
    assert(translation(1,0) == 0);
    assert(translation(1,2) == 0);
    assert(translation(2,0) == 0);
    assert(translation(2,1) == 0);
    assert(translation(3,0) == 0);
    assert(translation(3,1) == 0);
    assert(translation(3,2) == 0);
    assert(translation(0,3) == vector3(0,0));
    assert(translation(1,3) == vector3(1,0));
    assert(translation(2,3) == vector3(2,0));
    assert(translation(3,3) == 1);

    Matrix<int,4,1> translated_vector4 = translation * vector4;
    assert(translated_vector4(0,0) == vector4(0,0) + vector3(0,0));
    assert(translated_vector4(1,0) == vector4(1,0) + vector3(1,0));
    assert(translated_vector4(2,0) == vector4(2,0) + vector3(2,0));
    assert(translated_vector4(3,0) == 1);

    Matrix<float,4,1> vector4f;
    float vector4f_values[4] = { 10, 100, 1000, 1 };
    vector4f.take(vector4f_values);
    assert(vector4f.norm() == sqrt(10*10+100*100+1000*1000+1*1));
    Matrix<float,3,1> axis3;
    // Test rotation along the 3 axis
    {
        axis3.fill(0);
        axis3(0,0) = 2;
        Matrix<float,4,4> rotation = MatrixHelper::rotation(M_PI_2, axis3);
        Matrix<float,4,1> translated_vector4f = rotation * vector4f;
        assert(translated_vector4f(0,0) == vector4f(0,0));
        assert(translated_vector4f(1,0) == -vector4f(2,0));
        assert(translated_vector4f(2,0) == vector4f(1,0));
        assert(translated_vector4f(3,0) == 1);
    }
    {
        axis3.fill(0);
        axis3(1,0) = 20;
        Matrix<float,4,4> rotation = MatrixHelper::rotation(M_PI_2, axis3);
        Matrix<float,4,1> translated_vector4f = rotation * vector4f;
        assert(translated_vector4f(0,0) == vector4f(2,0));
        assert(translated_vector4f(1,0) == vector4f(1,0));
        assert(translated_vector4f(2,0) == -vector4f(0,0));
        assert(translated_vector4f(3,0) == 1);
    }
    {
        axis3.fill(0);
        axis3(2,0) = 200;
        Matrix<float,4,4> rotation = MatrixHelper::rotation(M_PI_2, axis3);
        Matrix<float,4,1> translated_vector4f = rotation * vector4f;
        assert(translated_vector4f(0,0) == -vector4f(1,0));
        assert(translated_vector4f(1,0) == vector4f(0,0));
        assert(translated_vector4f(2,0) == vector4f(2,0));
        assert(translated_vector4f(3,0) == 1);
    }
    {
        // Full turn around non axis-aligned axis
        axis3.fill(1);
        Matrix<float,4,4> rotation = MatrixHelper::rotation(2*M_PI, axis3);
        Matrix<float,4,1> translated_vector4f = rotation * vector4f;
        assert(translated_vector4f(0,0) == vector4f(0,0));
        assert(translated_vector4f(1,0) == vector4f(1,0));
        assert(translated_vector4f(2,0) == vector4f(2,0));
        assert(translated_vector4f(3,0) == 1);
    }

    return 0;
}

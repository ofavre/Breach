/**
 * @file any_test.cpp
 *
 * @brief Unit tests for the any pointer facility.
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

#include <cassert>

#include "any.hpp"



class BaseA {
    public:
        BaseA() {}
        virtual ~BaseA() {}
};
class BaseB {
    public:
        BaseB() {}
        virtual ~BaseB() {}
};

class DerivA : public BaseA {
    public:
        DerivA() {}
        virtual ~DerivA() {}
};
class DerivB : public BaseB {
    public:
        DerivB() {}
        virtual ~DerivB() {}
};

int main() {
    BaseA ba;
    BaseB bb;
    DerivA da;
    DerivB db;

    Any any;
    assert(!any.isSet());

    any.set(ba);
    assert(any.isSet());
    assert(any.get<BaseA>() == &ba);
    assert(any.get<BaseB>() == NULL);
    assert(any.get<DerivA>() == NULL);
    assert(any.get<DerivB>() == NULL);

    any.clear();
    assert(!any.isSet());
    assert(any.get<BaseA>() == NULL);
    assert(any.get<BaseB>() == NULL);
    assert(any.get<DerivA>() == NULL);
    assert(any.get<DerivB>() == NULL);

    any = bb;
    assert(any.isSet());
    assert(any.get<BaseA>() == NULL);
    assert(any.get<BaseB>() == &bb);
    assert(any.get<DerivA>() == NULL);
    assert(any.get<DerivB>() == NULL);

    any = da;
    assert(any.isSet());
    assert(any.get<BaseA>() == NULL);
    assert(any.get<BaseB>() == NULL);
    assert(any.get<DerivA>() == &da);
    assert(any.get<DerivB>() == NULL);

    any = db;
    assert(any.isSet());
    assert(any.get<BaseA>() == NULL);
    assert(any.get<BaseB>() == NULL);
    assert(any.get<DerivA>() == NULL);
    assert(any.get<DerivB>() == &db);

    Any any2 = any;
    assert(any.isSet());
    assert(any.get<BaseA>() == NULL);
    assert(any.get<BaseB>() == NULL);
    assert(any.get<DerivA>() == NULL);
    assert(any.get<DerivB>() == &db);

    Any any3(any2);
    assert(any.isSet());
    assert(any.get<BaseA>() == NULL);
    assert(any.get<BaseB>() == NULL);
    assert(any.get<DerivA>() == NULL);
    assert(any.get<DerivB>() == &db);

    return 0;
}
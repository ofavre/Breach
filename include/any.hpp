/**
 * @file any.hpp
 *
 * @brief Typesafe pointer to any type.
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

#ifndef _ANY_HPP
#define _ANY_HPP 1



#include <typeinfo>


class AnyHandle {
    public:
        virtual const std::type_info& type() = 0;
        virtual AnyHandle* copy() = 0;
};



template <class T>
class Handle : public AnyHandle {
    private:
        T& data;

    public:
        virtual const std::type_info& type();
        virtual AnyHandle* copy();

        Handle(T& value);
        T& get();
};



class Any {
    private:
        AnyHandle* handle;
    public:
        inline Any();
        inline Any(Any& tocopy);
        inline ~Any();

        template <class T>
        T& operator =(T& value);

        template <class T>
        void set(T& value);

        template <class T>
        T* get();

        inline void clear();

        inline bool isSet();
};



#include "any.tcc"

#endif /* _ANY_HPP */

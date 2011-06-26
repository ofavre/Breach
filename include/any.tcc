#ifndef _ANY_HPP
#error You should include any.hpp instead of this file directly
#endif

#ifndef _ANY_TCC
#define _ANY_TCC 1



#include <cstdlib> // NULL


template <class T>
Handle<T>::Handle(T& value)
: data(value)
{
}

template <class T>
T& Handle<T>::get()
{
    return data;
}

template <class T>
const std::type_info& Handle<T>::type()
{
    return typeid(T);
}

template <class T>
AnyHandle* Handle<T>::copy()
{
    return new Handle<T>(data);
}



Any::Any()
: handle(NULL)
{
}

Any::Any(Any& tocopy)
: handle(tocopy.handle == NULL ? NULL : tocopy.handle->copy())
{
}

Any::~Any()
{
    if (handle != NULL) {
        delete handle;
        handle = NULL;
    }
}

template <class T>
T& Any::operator =(T& value)
{
    set(value);
    return value;
}

template <class T>
Any& Any::set(T& value)
{
    if (handle != NULL) {
        delete handle;
    }
    handle = new Handle<T>(value);
    return *this;
}

template <class T>
T* Any::get()
{
    if (handle == NULL) {
        return NULL;
    }
    if (typeid(T) == handle->type()) {
        return &static_cast<Handle<T>*>(handle)->get();
    } else {
        return NULL;
    }
}

Any& Any::clear()
{
    if (handle != NULL) {
        delete handle;
        handle = NULL;
    }
    return *this;
}

bool Any::isSet()
{
    return handle != NULL;
}



#endif /* _ANY_TCC */

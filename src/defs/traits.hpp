#pragma once

template <typename> struct Types;

template <typename R, typename C, typename A, typename... Args>
struct Types<R(C::*)(A, Args...)>
{
    using returnType = R;
    using firstArgType = A;
};

template <typename T> using first_agument_t = typename Types<T>::firstArgType;
template <typename T> using return_t = typename Types<T>::returnType;

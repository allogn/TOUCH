/*

        Ecole Polytechnique Federale de Lausanne
        Brain Mind Institute,
        Blue Brain Project
        (c) 2008. All rights reserved.

        Responsible authors:    Juan Hernando Vieites

*/

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef BBP_CONST_MEMBER_FUNCTION_H
#define BBP_CONST_MEMBER_FUNCTION_H

namespace bbp
{

/** \cond SHOW_IN_DEVELOPER_REFERENCE */

//-----------------------------------------------------------------------------

template <typename T, 
          typename Return,
          Return (T::*Function)() const>
class const_member_function_t
{
public:
    Return operator()(const T &t) const
    {
        return (t.*Function)();
    }
};

//-----------------------------------------------------------------------------

template <typename T, 
          typename Return,
          Return (T::*Function)()>
class member_function_t
{
public:
    Return operator()(T &t) const
    {
        return (t.*Function)();
    }
};

//-----------------------------------------------------------------------------

template <typename T, 
          typename Return,
          typename Arg1,
          Return (T::*Function)(Arg1) const>
class const_member_function_1_t
{
public:
    Return operator()(const T &t, Arg1 arg1) const
    {
        return (t.*Function)(arg1);
    }
};

//-----------------------------------------------------------------------------

template <typename T, 
          typename Return,
          typename Arg1,
          Return (T::*Function)(Arg1)>
class member_function_1_t
{
public:
    Return operator()(T &t, Arg1 arg1) const
    {
        return (t.*Function)(arg1);
    }
};


/** \endcond SHOW_IN_DEVELOPER_REFERENCE */

}
#endif

//  Boost Type-Traits Library, undecorate.hpp header file  -------------------//

//  Copyright 2012 Daryle Walker.
//  Distributed under the Boost Software License, Version 1.0.  (See the
//  accompanying file LICENSE_1_0.txt or a copy at
//  <http://www.boost.org/LICENSE_1_0.txt>.)

//  See <http://www.boost.org/libs/type_traits/> for the library's home page.

/** \file
    \brief  Type-trait class template to strip all pointer decorators.

    \author  Daryle Walker

    \version  0.5

    \copyright  Boost Software License, version 1.0

    Contains the declaration and definitions for `undecorate`, which reaches
    a given type's base pointed-to type by continuously stripping the type's
    front-loaded reference, cv-qualification, and pointer markers until it
    cannot be done any more.  The result will be cv-unqualified non-pointer
    type.  (It does include arrays-of-CV-pointer, though.)
 */

#ifndef BOOST_TYPE_TRAITS_UNDECORATE_HPP
#define BOOST_TYPE_TRAITS_UNDECORATE_HPP


namespace boost
{


//  Declaration & primary definition of decorator-stripping meta-function  ---//

//! Meta-function: type -> type without any *, const, volatile, &, and/or &&
/** With the given type, strip any top-level reference marker and/or
    cv-qualification, then remove any pointer indicators (possibly with their
    own cv-qualifications) until no more can be done, and return that
    ultimately-pointed-to type.  Note that array types are not stripped, so
    the process will stop at any array types encountered, even if the element
    type (after removing all extents) is a pointer type.  (This is because the
    original intent was to aid converting data-pointers to pointer-to-CV-void
    objects, which array types (or any other non-pointer type) can't do.)

    The result will be an inner type-alias called `type`.

    \tparam T  The type to be (multiply) stripped.

    \todo  Make a test suite for this meta-function.
 */
template < typename T >
struct undecorate
{
    typedef T  type;
};


//  Decorator-stripping meta-function partial specialization definitions  ----//

//! Specialize `undecorate` when the type leads with an r-value reference.
template < typename T >
struct undecorate<T &&>
{
    typedef typename undecorate<T>::type  type;
};

//! Specialize `undecorate` when the type leads with a l-value reference.
template < typename T >
struct undecorate<T &>
{
    typedef typename undecorate<T>::type  type;
};

//! Specialize `undecorate` when the type leads with both cv-qualifications.
template < typename T >
struct undecorate<T const volatile>
{
    typedef typename undecorate<T>::type  type;
};

//! Specialize `undecorate` when the type leads with a volatile qualification.
template < typename T >
struct undecorate<T volatile>
{
    typedef typename undecorate<T>::type  type;
};

//! Specialize `undecorate` when the type leads with a const qualification.
template < typename T >
struct undecorate<T const>
{
    typedef typename undecorate<T>::type  type;
};

//! Specialize `undecorate` when the type leads with a pointer indicator.
template < typename T >
struct undecorate<T *>
{
    typedef typename undecorate<T>::type  type;
};


}  // namespace boost


#endif  // BOOST_TYPE_TRAITS_UNDECORATE_HPP

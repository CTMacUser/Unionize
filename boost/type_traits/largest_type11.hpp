//  Boost Type-Traits Library, largest_type11.hpp header file  ---------------//

//  Copyright 2012 Daryle Walker.
//  Distributed under the Boost Software License, Version 1.0.  (See the
//  accompanying file LICENSE_1_0.txt or a copy at
//  <http://www.boost.org/LICENSE_1_0.txt>.)

//  See <http://www.boost.org/libs/type_traits/> for the library's home page.

/** \file
    \brief  Type-trait class template to find the largest type of a set.

    \author  Daryle Walker

    \version  0.5

    \copyright  Boost Software License, version 1.0

    Contains the declaration and definitions for `largest_type`, which returns
    the first-found type of a given variadic type-list that has the largest
    `sizeof` value.
 */

#ifndef BOOST_TYPE_TRAITS_LARGEST_TYPE11_HPP
#define BOOST_TYPE_TRAITS_LARGEST_TYPE11_HPP

#include <type_traits>


namespace boost
{


//  Forward declaration  -----------------------------------------------------//

//! Meta-function: list of types -> first type with largest sizeof value
/** Given several types submitted through a variadic type-based template
    parameter, provide a type-alias to the type that is the largest.  The
    largest is determined by the value returned by `sizeof`.  If there is a tie
    for largest, the first type listed is returned.

    The result will be an inner type-alias called `type`.

    \tparam First  The first type to be tested.
    \tparam Other  Any other type(s) to be compared in the tests.  May be empty.

    \todo  Make a test suite for this meta-function.
 */
template < typename First, typename ...Other >
struct largest_type;


//  Size-maximum metadata template partial specialization definitions  -------//

//! Specialize `largest_type` for the degenerate base case.
template < typename T >
struct largest_type<T>
{
    typedef T  type;
};

//! Specialize `largest_type` for the main base case.
template < typename T, typename U >
struct largest_type<T, U>
{
    typedef typename std::conditional<
        (sizeof(T) < sizeof(U)), U, T
    >::type  type;
};

//! Specialize `largest_type` for the recursive case.
template < typename T, typename U, typename V, typename ...W >
struct largest_type<T, U, V, W...>
{
    typedef typename largest_type<
        T, typename largest_type<U, V, W...>::type
    >::type  type;
};


}  // namespace boost


#endif  // BOOST_TYPE_TRAITS_LARGEST_TYPE11_HPP

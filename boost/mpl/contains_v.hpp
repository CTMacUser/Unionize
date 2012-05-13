//  Boost MPL, contains_v.hpp header file  -----------------------------------//

//  Copyright 2012 Daryle Walker.
//  Distributed under the Boost Software License, Version 1.0.  (See the
//  accompanying file LICENSE_1_0.txt or a copy at
//  <http://www.boost.org/LICENSE_1_0.txt>.)

//  See <http://www.boost.org/libs/mpl/> for the library's home page.

/** \file
    \brief  Check if a type is in a variadic type list

    \author  Daryle Walker

    \version  0.5

    \copyright  Boost Software License, version 1.0

    Contains the declaration and definitions for `contains_v`, a version of
    `boost::mpl::contains` adapted for C++11's variadic type lists.  If the
    first given type is duplicated amoung the later ones, a meta-function
    constant of `true` is revealed, `false` otherwise.
 */

#ifndef BOOST_MPL_CONTAINS_V_HPP
#define BOOST_MPL_CONTAINS_V_HPP

#include <type_traits>


namespace boost
{
namespace mpl
{


//  Forward declarations  ----------------------------------------------------//

//! Meta-function: a type and list of types -> whether the type is in the list
/** Given one type followed by several types submitted through a variadic
    type-based template parameter, provide a compile-time constant checking if
    the first type is one of the types in the following list.

    The result is that the class inherits from a Boolean-valued Meta-function
    Integral-constant.  The constant is `true` if `Target` is in `Corpus`,
    `false` otherwise.

    \tparam Target  The type to look for.
    \tparam Corpus  The list of types to search for `Target` in.  May be empty.

    \todo  Make a test suite for this meta-function.
 */
template < typename Target, typename ...Corpus >
struct contains_v;


//  Member-containment metadata template partial specialization definitions  -//

//! Specialize `contains_v` for the base case.
template < typename T >
struct contains_v<T>
    : std::false_type
{ };

//! Specialize `contains_v` for the recursive case.
template < typename T, typename U, typename ...V >
struct contains_v<T, U, V...>
    : std::conditional<
          std::is_same<T, U>::value,
          std::true_type,
          typename contains_v<T, V...>::type
      >::type
{ };


}  // namespace mpl
}  // namespace boost


#endif  // BOOST_MPL_CONTAINS_V_HPP

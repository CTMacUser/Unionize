//  Boost MPL, type_at_v.hpp header file  ------------------------------------//

//  Copyright 2012 Daryle Walker.
//  Distributed under the Boost Software License, Version 1.0.  (See the
//  accompanying file LICENSE_1_0.txt or a copy at
//  <http://www.boost.org/LICENSE_1_0.txt>.)

//  See <http://www.boost.org/libs/mpl/> for the library's home page.

/** \file
    \brief  Return the type at a certain index in a list

    \author  Daryle Walker

    \version  0.5

    \copyright  Boost Software License, version 1.0

    Contains the declaration and definitions for `type_at_v`, a version of
    `boost::mpl::at` adapted for C++11's variadic type lists.  With a given
    index value and a variadic type list, reveal the (index + 1)'th type from
    that list as a type-alias.
 */

#ifndef BOOST_MPL_TYPE_AT_V_HPP
#define BOOST_MPL_TYPE_AT_V_HPP

#include <cstddef>


namespace boost
{
namespace mpl
{


//  Forward declarations  ----------------------------------------------------//

//! Meta-function: an index \& type-list -> the type at that place in the list
/** Given a (zero-based) index value, followed by several types submitted
    throught a variadic type-based template parameter, provide a type-alias for
    type at the indicated position.

    The result is an inner type-alias called `type`.  If the type-list is too
    short to support the given index, then this class template specialization
    will remain undefined, probably leading to a compile-time error.

    \pre  0 \<= `Index` \< sizeof...(`Types`)

    \tparam Index  Which type to count for, zero-based.
    \tparam Types  The list of types to search.  May be empty.

    \todo  Make a test suite for this meta-function.
 */
template < std::size_t Index, typename ...Types >
struct type_at_v;


//  Type-list indexing metadata template partial specialization definitions  -//

//! Specialize `type_at_v` for the base case.
template < typename Head, typename ...Tail >
struct type_at_v<0, Head, Tail...>
{
    typedef Head  type;
};

//! Specialize `type_at_v` for the recursive case.
template < std::size_t Index, typename Head, typename ...Tail >
struct type_at_v<Index, Head, Tail...>
{
    typedef typename type_at_v<Index - 1, Tail...>::type  type;
};


}  // namespace mpl
}  // namespace boost


#endif  // BOOST_MPL_TYPE_AT_V_HPP

//  Boost Unions Library, variant_traits.hpp header file  --------------------//

//  Copyright 2012 Daryle Walker.
//  Distributed under the Boost Software License, Version 1.0.  (See the
//  accompanying file LICENSE_1_0.txt or a copy at
//  <http://www.boost.org/LICENSE_1_0.txt>.)

//  See <http://www.boost.org/libs/unions/> for the library's home page.

/** \file
    \brief  Base declarations of traits classes for extended-union types.

    \author  Daryle Walker

    \version  0.5

    \copyright  Boost Software License, version 1.0

    Contains the declarations for `variant_element` and `variant_size`,
    analogues of `std::tuple_element` and `std::tuple_size`.  There are no
    specializations for any extended-union types; the creators of those types,
    or interested users, are expected to write those.  There are partial
    specializations for cv-qualified extended-union types, so the users only
    have to make specializations for thier directly-created types.
 */

#ifndef BOOST_UNIONS_VARIANT_TRAITS_HPP
#define BOOST_UNIONS_VARIANT_TRAITS_HPP

#include <cstddef>
#include <type_traits>


namespace boost
{
namespace unions
{


//  Forward declarations  ----------------------------------------------------//

//! Meta-function: index & union-based class -> variant type in that class
/** Given a union-based class where its variant types can be formed into a
    list, present the variant type at the given (zero-based) index of the list.

    Each union-based class is expected to provide (partial) specializations of
    this class template in order to return the answers.  It's up to each class
    to determine its policy for types that appear in the list more than once.
    The meta-function is expected to have a single member, called \"type\",
    that is a type-alias of the appropriate variant type.  The internal `type`
    shouldn't be defined when the `Index` is out of bounds.

    \tparam Index        The zero-based index of which variant type to return.
    \tparam VariantType  The union-based class to be inspected.
 */
template < std::size_t Index, class VariantType >
struct variant_element;  // undefined

//! Meta-function: union-based class -> count of its variant types
/** Given a union-based class, present the number of variant types it contains.
    The valid indices for #variant_element should be less than this number.

    Each union-based class is expected to provide (partial) specializations of
    this class template in order to return the answers.  It's up to each class
    to determine its policy for types that are variants more than once.  The
    meta-function is expected to match the interface of Meta-function
    Integer-constants, i.e. inheirit from
    `std\::integral_constant\<std\::size_t, XXX\>`, where `XXX` is the desired
    value.

    \tparam VariantType  The union-based class to be inspected.
 */
template < class VariantType >
struct variant_size;  // undefined


//  Union metadata template partial specialization definitions  --------------//

//! Specialize `variant_element` to propagate a union's cv-qualification.
template < std::size_t Index, class VariantType >
struct variant_element<Index, const volatile VariantType>
{
    typedef typename std::add_cv<
        typename variant_element< Index, VariantType >::type
    >::type  type;
};

//! Specialize `variant_element` to propagate a union's const-qualification.
template < std::size_t Index, class VariantType >
struct variant_element<Index, const VariantType>
{
    typedef typename std::add_const<
        typename variant_element< Index, VariantType >::type
    >::type  type;
};

//! Specialize `variant_element` to propagate a union's volatile-qualification.
template < std::size_t Index, class VariantType >
struct variant_element<Index, volatile VariantType>
{
    typedef typename std::add_volatile<
        typename variant_element< Index, VariantType >::type
    >::type  type;
};

//! Specialize of `variant_size` to ignore a union's cv-qualification.
template < class VariantType >
struct variant_size<const volatile VariantType>
    : variant_size<VariantType>::type
{ };

//! Specialize of `variant_size` to ignore a union's const-qualification.
template < class VariantType >
struct variant_size<const VariantType>
    : variant_size<VariantType>::type
{ };

//! Specialize of `variant_size` to ignore a union's volatile-qualification.
template < class VariantType >
struct variant_size<volatile VariantType>
    : variant_size<VariantType>::type
{ };


}  // namespace unions
}  // namespace boost


#endif  // BOOST_UNIONS_VARIANT_TRAITS_HPP

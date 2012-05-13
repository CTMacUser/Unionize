//  Boost Unions Library, super_union.hpp header file  -----------------------//

//  Copyright 2012 Daryle Walker.
//  Distributed under the Boost Software License, Version 1.0.  (See the
//  accompanying file LICENSE_1_0.txt or a copy at
//  <http://www.boost.org/LICENSE_1_0.txt>.)

//  See <http://www.boost.org/libs/unions/> for the library's home page.

/** \file
    \brief  A union with its member types specified in the template parameters.

    \author  Daryle Walker

    \version  0.5

    \copyright  Boost Software License, version 1.0

    Contains the declaration and definitions of `super_union`, a union
    class-template that gets the types of its variant members through a template
    parameter list.  Instead of using the implementation-defined member names to
    get at a variant member, custom access function templates are provided.
    There are also specializations of some traits classes for `super_union`.
 */

#ifndef BOOST_UNIONS_SUPER_UNION_HPP
#define BOOST_UNIONS_SUPER_UNION_HPP

#include "boost/unions/variant_traits.hpp"

#include <array>
#include <cstddef>
#include <typeinfo>
#include <type_traits>
#include <utility>


namespace boost
{
namespace unions
{


//  Forward declarations  ----------------------------------------------------//

//! Union-type with its variant members addressed by type
/** Provides a union-based class-type where its variant members have their
    member names hidden, and are solely addressed via their type.  The nature
    of a union makes (intentionally) adding multiple (non-static) members of
    the same type useless, so why not use the type as an index.  The types for
    the union are described by a (variadic) list of template parameters.

    Remember the (C++11) rules for unions:

    - Reference types cannot be used for (non-static) members.
    - For each of the six special member functions (default constructor, copy
      constructor, move constructor, destructor, and the copy- and
      move-assignment operators), a particular special member function is
      defined only if it both is not deleted and is trivial for **every**
      variant type.  Otherwise, that special member function is deleted unless
      it's user-provided.
    - Since this union will not provide any user-declared constructors, nor will
      it use member initializers, you can use aggregate initialization on
      objects of this type.  (You can simulate default construction, even if the
      offical one is deleted, as long as the first listed variant type is
      default-constructible.)

    Also, any class type can contain a pointer (or multi-level pointer) to its
    own type as a member.  You cannot do that here because any reference to
    `super_union` within its own template parameter list would involve infinite
    recursion.  To get around this, use a pointer-to-(an appropriately
    cv-qualified)-`void` as one of the variant types.  If that is already one
    of the variant types, then you need to carry an external flag indicating
    which usage.

    \tparam Types  The types to be included in the union.  It may be any
                   nonnegative number in length (supported by the compiler).
                   No type listed may be a reference, since references alias
                   other objects and are not objects themselves.  Use a pointer
                   type if you need to connect to another object.  Repeats
                   are not tracked.
 */
template < typename ...Types >
union super_union;


//  Type-tagged union template specialization definitions  -------------------//

//! Specialization of `super_union` when the type list is empty
/** The base case contains no variant members.  It has degenerate versions of
    status-indicating members.  You can create objects of this type with
    default construction and/or an empty aggregate initialization list.
 */
template < >
union super_union<>
{
    //! Returns a list of the union's variant members' types.
    static  auto variant_types() -> std::array<std::type_info const *, 0>
    { return { {} }; }
};

//! Specialization of `super_union` when the type list is not empty
/** The recursive case directly contains its first variant member, and uses a
    sibling specialization for the other variant members.  There are several
    helper non-variant members for revealing information about the
    `super_union`.

    \tparam Head  The first variant type listed, which is directly contained
                  in this union.  It cannot be a reference.
    \tparam Tail  The other variant types listed, which are indirectly
                  contained.  This variadic list may be empty.  No type listed
                  can be a reference.  Repeats, either with `Head` and/or within
                  itself, are not tracked.
 */
template < typename Head, typename ...Tail >
union super_union<Head, Tail...>
{
    //! Returns a list of the union's variant members' types.
    static
    auto  variant_types()
      -> std::array<std::type_info const *, 1 + sizeof...(Tail)>
    { return { {&typeid(Head), &typeid(Tail)...} }; }

    Head                  data;
    super_union<Tail...>  rest;
};


//  Type-tagged union metadata template specialization definitions  ----------//

//! Specialization of `variant_element` for `super_union`s, base case.
template < typename Head, typename ...Tail >
struct variant_element< 0, super_union<Head, Tail...> >
{
    typedef Head  type;
};

//! Specialization of `variant_element` for `super_union`s, recursive case.
template < std::size_t Index, typename Head, typename ...Tail >
struct variant_element< Index, super_union<Head, Tail...> >
{
    typedef typename variant_element< Index - 1, super_union<Tail...> >::type
      type;
};  // Note that any repeats are included!

//! Specialization of `variant_size` for `super_union`s.
template < typename ...Types >
struct variant_size< super_union<Types...> >
    : std::integral_constant<std::size_t, sizeof...(Types)>
{ };  // Note that all repeats are included!


//  Type-tagged union template data extraction functions  --------------------//

//! \cond
namespace detail
{
    template < typename T, typename Head, typename ...Tail >
    auto  gett_impl( super_union<Head, Tail...> &su, std::true_type ) -> T &
    { return su.data; }

    template < typename T, typename Head, typename Middle, typename ...Tail >
    auto  gett_impl( super_union<Head, Middle, Tail...> &su, std::false_type )
      -> T &
    { return gett_impl<T>(su.rest, typename std::is_same<T, Middle>::type{}); }

    template < typename T, typename Head, typename ...Tail >
    auto  gett_impl( super_union<Head, Tail...> const &su, std::true_type )
      -> T const &
    { return su.data; }

    template < typename T, typename Head, typename Middle, typename ...Tail >
    auto  gett_impl( super_union<Head, Middle, Tail...> const &su,
     std::false_type ) -> T const &
    { return gett_impl<T>(su.rest, typename std::is_same<T, Middle>::type{}); }

    template < typename T, typename Head, typename ...Tail >
    auto  gett_impl( super_union<Head, Tail...> &&su, std::true_type ) -> T &&
    { return std::move( su.data ); }

    template < typename T, typename Head, typename Middle, typename ...Tail >
    auto  gett_impl( super_union<Head, Middle, Tail...> &&su, std::false_type )
      -> T &&
    {
        return gett_impl<T>( std::move(su.rest),
         typename std::is_same<T, Middle>::type{} );
    }

    template < std::size_t Index, typename ...Types >
    auto  get_impl( super_union<Types...> &su, std::false_type )
      -> typename variant_element<Index, super_union<Types...>>::type &
    { return su.data; }

    template < std::size_t Index, typename ...Types >
    auto  get_impl( super_union<Types...> &su, std::true_type )
      -> typename variant_element<Index, super_union<Types...>>::type &
    {
        return get_impl<Index - 1>( su.rest,
         std::integral_constant<bool, (Index > 1)>{} );
    }

    template < std::size_t Index, typename ...Types >
    auto  get_impl( super_union<Types...> const &su, std::false_type )
      -> typename variant_element<Index, super_union<Types...>>::type const &
    { return su.data; }

    template < std::size_t Index, typename ...Types >
    auto  get_impl( super_union<Types...> const &su, std::true_type )
      -> typename variant_element<Index, super_union<Types...>>::type const &
    {
        return get_impl<Index - 1>( su.rest,
         std::integral_constant<bool, (Index > 1)>{} );
    }

    template < std::size_t Index, typename ...Types >
    auto  get_impl( super_union<Types...> &&su, std::false_type )
      -> typename variant_element<Index, super_union<Types...>>::type &&
    { return std::move( su.data ); }

    template < std::size_t Index, typename ...Types >
    auto  get_impl( super_union<Types...> &&su, std::true_type )
      -> typename variant_element<Index, super_union<Types...>>::type &&
    {
        return get_impl<Index - 1>( std::move(su.rest),
         std::integral_constant<bool, (Index > 1)>{} );
    }
}
//! \endcond

//! Extract the variant member of the given type from the given `super_union`.
/** If `T` is not one of the variant types of the `super_union`, then a
    compile-time error will be eventually generated.  If `T` is an allowable
    type, but not the current active variant member, then undefined behavior
    may result depending on the subsequent use of the access, especially if
    `T` and/or the current type are not POD.

    \pre  `T` is the type of at least one of the variant members of `su`'s type.

    \tparam T     The type of the desired variant member.  Must be explicitly
                  provided.
    \tparam Head  The first variant type listed in `su`'s type.  Should not be
                  explicitly provided.
    \tparam Tail  The other variant types listed in `su`'s type.  This variadic
                  list may be empty.  Should not be explicitly provided.

    \param[in] su  The `super_union` object to be accessed.

    \throws  Nothing.  But this function is not marked `noexcept` because
             undefined behavior may be induced if `T` is the wrong choice.

    \returns  A reference to the variant member of type `T`.  The reference
              has the same cv-qualifications and l-versus-r state as `su`.  If
              the variant type list for `su` repeats `T`, then the first one
              found is returned.  (They should be equivalent, anyway.)
 */
template < typename T, typename Head, typename ...Tail >
auto  gett( super_union<Head, Tail...> &su ) -> T &
{ return detail::gett_impl<T>(su, typename std::is_same<T, Head>::type{}); }

//! \overload
template < typename T, typename Head, typename ...Tail >
auto  gett( super_union<Head, Tail...> const &su ) -> T const &
{ return detail::gett_impl<T>(su, typename std::is_same<T, Head>::type{}); }

//! \overload
template < typename T, typename Head, typename ...Tail >
auto  gett( super_union<Head, Tail...> &&su ) -> T &&
{
    return detail::gett_impl<T>( std::forward<super_union<Head, Tail...>>(su),
     typename std::is_same<T, Head>::type{} );
}

//! Extract variant member with the given index from the given `super_union`.
/** If `Index` is out of range, then a compile-time error will be eventually
    generated.  If the `Index` is valid, but the corresponding variant member
    is not the current active one, then undefined behavior may result depending
    on the subsequent use of the access, especially if those variants differ in
    type and at least one of them is not POD.

    \pre  `0 \<= Index \< #variant_size\<decltype(su)\>\::value`

    \tparam Index  The index into the list of variant types of `su`'s type.
                   Must be explicitly provided.  The index is zero-based.
    \tparam Types  The variant types listed in `su`'s type.  Should not be
                   explicitly provided.  This variadic list may be empty.

    \param[in] su  The `super_union` object to be accessed.

    \throws  Nothing.  But this function is not marked `noexcept` because
             undefined behavior may be induced if `Index` is the wrong choice.

    \returns  A reference to the variant member counted to `Index`.  The
              reference has the same cv-qualifications and l-versus-r state as
              `su`.  Even if the variant type list for `su` repeats the variant
              type at `Index`, the specified one is returned.  (They should be
              equivalent, anyway.)
 */
template < std::size_t Index, typename ...Types >
auto  get( super_union<Types...> &su )
  -> typename variant_element<Index, super_union<Types...>>::type &
{
    return detail::get_impl<Index>(su, std::integral_constant<bool, !!Index>{});
}

//! \overload
template < std::size_t Index, typename ...Types >
auto  get( super_union<Types...> const &su )
  -> typename variant_element<Index, super_union<Types...>>::type const &
{
    return detail::get_impl<Index>(su, std::integral_constant<bool, !!Index>{});
}

//! \overload
template < std::size_t Index, typename ...Types >
auto  get( super_union<Types...> &&su )
  -> typename variant_element<Index, super_union<Types...>>::type &&
{
    return detail::get_impl<Index>( std::forward<super_union<Types...>>(su),
     std::integral_constant<bool, !!Index>{} );
}


}  // namespace unions
}  // namespace boost


#endif  // BOOST_UNIONS_SUPER_UNION_HPP

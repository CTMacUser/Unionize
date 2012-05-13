//  Boost Utility Library, apply11.hpp header file  --------------------------//

//  Copyright 2012 Daryle Walker.
//  Distributed under the Boost Software License, Version 1.0.  (See the
//  accompanying file LICENSE_1_0.txt or a copy at
//  <http://www.boost.org/LICENSE_1_0.txt>.)

//  See <http://www.boost.org/libs/utility/> for the library's home page.

/** \file
    \brief  A function that calls a given function with given arguments.

    \author  Daryle Walker

    \version  0.5

    \copyright  Boost Software License, version 1.0

    Contains the definition of the `apply` function template.  This function
    takes a function object, followed by its arguments (if any), then calls
    said function with said arguments.  The code for doing this usually causes
    a compile-time error if the function object doesn't actually support the
    function-call syntax, or it does but not with the given arguments.  The
    SFINAE technique is used to prevent this case from causing a compile-time
    error; it becomes a no-op instead.
 */

#ifndef BOOST_UTILITY_APPLY11_HPP
#define BOOST_UTILITY_APPLY11_HPP

#include <typeinfo>
#include <utility>


namespace boost
{


//  SFINAE-aided application caller function  --------------------------------//

//! \cond
namespace detail
{
    template < typename Func >
    void  help_apply( std::type_info const *&return_type, Func &&f, ... )
    { return_type = nullptr; }

    template < typename Func, typename ...Args >
    auto  help_apply(std::type_info const *&return_type, Func&& f, Args&& ...a)
      -> decltype( std::forward<Func>(f)(std::forward<Args>( a )...) )
    {
        return return_type = &typeid( decltype(std::forward<Func>( f )(
         std::forward<Args>(a)... )) ), std::forward<Func>( f )(
         std::forward<Args>(a)... );
    }
}
//! \endcond

//! Call a function (with arguments) only if there's a match.
/** If `f` is a function, function pointer, or class object with an application
    operator, call that code with the given arguments `a`.  Otherwise, do
    nothing.  The program keeps running if nothing happens, using SFINAE to
    block compile-time errors when `f` doesn't support the function-call syntax
    at all or doesn't support it with the given `a`.

    \tparam Func  The type of `f`.  Should not be explicitly given.
    \tparam Args  The type(s) of `a`.  Should not be explicitly given.

    \param[in] f  The (supposed) function to be called.
    \param[in] a  The arguments, if any, to use when calling `f`.

    \throws  Anything `f` may throw when using `a`; if `f` and `a` are not
             compatible, then nothing is thrown.

    \returns  The address of an immutable `std\::type_info` object describing
              the return type when `f` uses `a`.  If `f` and `a` are
              incompatible, then `nullptr` is returned.  If you're comparing
              with another type, check the return value for `NULL` first, then
              compare the *dereferenced* pointer to the target type-info object.
              (A type may have multiple type-info objects, so two objects with
              differing addresses may equate to the same type.)

    \todo  Make a test suite for this function.
 */
template < typename Func, typename ...Args >
auto  apply( Func&& f, Args&& ...a ) -> std::type_info const *
{
    std::type_info const  *result;

    detail::help_apply( result, std::forward<Func>(f),
     std::forward<Args>(a)... );
    return result;
}


}  // namespace boost


#endif  // BOOST_UTILITY_APPLY11_HPP

//  Boost Unions Library, tagged_union.hpp header file  ----------------------//

//  Copyright 2012 Daryle Walker.
//  Distributed under the Boost Software License, Version 1.0.  (See the
//  accompanying file LICENSE_1_0.txt or a copy at
//  <http://www.boost.org/LICENSE_1_0.txt>.)

//  See <http://www.boost.org/libs/unions/> for the library's home page.

/** \file
    \brief  A tagged-union with its member types specified in the template
            parameters.

    \author  Daryle Walker

    \version  0.5

    \copyright  Boost Software License, version 1.0

    Contains the declaration and definitions of `tagged_union`, a union
    class-template that gets the types of its variant members through a template
    parameter list, and keeps track of which variant is active.  Instead of
    using the implementation-defined member names to get at a variant member,
    custom access function templates are provided.  The union can be initialized
    with objects of its variant types directly (and indirectly assigned).  There
    are also specializations of some traits classes for `tagged_union`.
 */

#ifndef BOOST_UNIONS_TAGGED_UNION_HPP
#define BOOST_UNIONS_TAGGED_UNION_HPP

#include "boost/mpl/contains_v.hpp"
#include "boost/mpl/type_at_v.hpp"
#include "boost/type_traits/largest_type11.hpp"
#include "boost/type_traits/undecorate.hpp"
#include "boost/unions/variant_traits.hpp"
#include "boost/utility/apply11.hpp"
#include <boost/variant/get.hpp>

#include <array>
#include <cstddef>
#include <cstring>
#include <new>
#include <typeinfo>
#include <type_traits>
#include <utility>


namespace boost
{
namespace unions
{


//  Implementation details  --------------------------------------------------//

//! \cond
namespace detail
{
    template < typename Func, typename ...Args >
    void  visit_via_ref( Func&&, std::type_info const &, void *, Args&& ... )
    { }

    template <typename Head, typename ...Tail, typename Func, typename ...Args>
    void  visit_via_ref( Func&& visitor, std::type_info const &type, void *data,
     Args&& ...args )
    {
        if ( type == typeid(Head) )
            apply( std::forward<Func>(visitor), *static_cast<Head *>(data),
             std::forward<Args>(args)... );
        else
            visit_via_ref<Tail...>( std::forward<Func>(visitor), type, data,
             std::forward<Args>(args)... );
    }

    template < typename Func, typename ...Args >
    void  visit_via_ref(Func&&, std::type_info const &, void const *, Args&&...)
    { }

    template <typename Head, typename ...Tail, typename Func, typename ...Args>
    void  visit_via_ref( Func&& visitor, std::type_info const &type, void const
     *data, Args&& ...args )
    {
        if ( type == typeid(Head) )
            apply( std::forward<Func>(visitor), *static_cast<Head const *>(data)
             , std::forward<Args>(args)... );
        else
            visit_via_ref<Tail...>( std::forward<Func>(visitor), type, data,
             std::forward<Args>(args)... );
    }

    template < typename Func, typename ...Args >
    void  visit_via_rref( Func&&, std::type_info const &, void *, Args&& ... )
    { }

    template <typename Head, typename ...Tail, typename Func, typename ...Args>
    void  visit_via_rref( Func&& visitor, std::type_info const &type, void *data
     , Args&& ...args )
    {
        if ( type == typeid(Head) )
            apply( std::forward<Func>(visitor), std::move(*static_cast<Head *>(
             data )), std::forward<Args>(args)... );
        else
            visit_via_rref<Tail...>( std::forward<Func>(visitor), type, data,
             std::forward<Args>(args)... );
    }

    template < typename Func, typename ...Args >
    void  visit_via_ptr( Func&&, std::type_info const &, void *, Args&& ... )
    { }

    template <typename Head, typename ...Tail, typename Func, typename ...Args>
    void  visit_via_ptr( Func&& visitor, std::type_info const &type, void *data,
     Args&& ...args )
    {
        if ( type == typeid(Head) )
            apply( std::forward<Func>(visitor), static_cast<Head *>(data),
             std::forward<Args>(args)... );
        else
            visit_via_ptr<Tail...>( std::forward<Func>(visitor), type, data,
             std::forward<Args>(args)... );
    }

    template < typename Func, typename ...Args >
    void  visit_via_ptr(Func&&, std::type_info const &, void const *, Args&&...)
    { }

    template <typename Head, typename ...Tail, typename Func, typename ...Args>
    void  visit_via_ptr( Func&& visitor, std::type_info const &type, void const
     *data, Args&& ...args )
    {
        if ( type == typeid(Head) )
            apply( std::forward<Func>(visitor), static_cast<Head const *>(data),
             std::forward<Args>(args)... );
        else
            visit_via_ptr<Tail...>( std::forward<Func>(visitor), type, data,
             std::forward<Args>(args)... );
    }

    inline
    void  memswap( void *a, void *b, std::size_t s )
    {
        auto  aa = static_cast<unsigned char *>( a ),
              bb = static_cast<unsigned char *>( b );

        while ( s-- )
            std::swap( *aa++, *bb++ );
    }

    struct destroyer
    {
        template < typename T >
        void  operator ()( T *about_to_die ) const
        { about_to_die->~T(); }
    };

    struct copy_constructor
    {
        template < typename T >
        void  operator ()( T const &source, void *destination ) const
        { ::new (destination) T{ source }; }
    };

    struct move_constructor
    {
        template < typename T >
        void  operator ()( T &&source, void *destination ) const
        { ::new (destination) T{ std::move(source) }; }
    };

    struct copy_assigner
    {
        template < typename T >
        T &  operator ()( T const &source, void *destination ) const
        { return *static_cast<T *>(destination) = source; }
    };

    struct move_assigner
    {
        template < typename T >
        T &  operator ()( T &&source, void *destination ) const
        { return *static_cast<T *>(destination) = std::move(source); }
    };
}
//! \endcond


//  Tracked type-tagged union template definition  ---------------------------//

//! Union-type with its tracked variant members addressed by type
/** Provides a union-based class-type where its variant members have their
    member names hidden, and are solely addressed via their type.  The nature
    of a union makes (intentionally) adding multiple (non-static) members of
    the same type useless, so why not use the type as an index.  The types for
    the union are described by a (variadic) list of template parameters.
    Objects track which variant type is active, and so flag errors when the
    wrong type is selected.  Objects can be initialized or assigned from
    objects/values of any of the variant types.

    Class-types can use a pointer (or multi-level pointer) to its own type as
    members.  This type cannot do that since the variant types are listed in
    the template header.  (You cannot describe the type in itself without
    invoking infinite recursion.)  As a workaround, this type can store pointers
    to itself without needing to specify them in `Types`.  Of course, you still
    can't list non-pointer types that refer to this type (i.e. class-type
    templates that would use this type as a parameter) as variant members.

    \tparam Types  The types to be included in the union.  It may be empty.
                   Neither reference and/or cv-qualified types may be used.
                   Repeats are not flagged.

    \todo  Add filter to ban reference or cv-qualified types.
    \todo  Should we require all types to have a copy-ctr, move-ctr, dtr,
           copy-assign, and move-assign?
 */
template < typename ...Types >
class tagged_union
{
public:
    //! Returns a list of the union's variant members' types.
    static
    auto  variant_types() noexcept
      -> std::array<std::type_info const *, sizeof...(Types)>
    { return { {&typeid(Types)...} }; }

    //! Default-construction, with no data
    tagged_union() noexcept
        : what_{}
        , which_{ nullptr }
        , self_{ false }
    { }

    //! Construction by copy-constructing from a variant type
    template <
        typename T,
        class EnableIf = typename std::enable_if<
            mpl::contains_v<T, Types...>::value
        >::type
    >
    tagged_union( T const &that )
        noexcept( std::is_nothrow_copy_constructible<T>::value )
        : what_{}, which_{ &typeid(T) }, self_{ false }
    { ::new (&this->what_) T{ that }; }
    //! Construction by move-constructing from a variant type
    template <
        typename T,
        class EnableIf = typename std::enable_if<
            mpl::contains_v<T, Types...>::value
        >::type
    >
    tagged_union( T &&that )
        noexcept( std::is_nothrow_move_constructible<T>::value )
        : what_{}, which_{ &typeid(T) }, self_{ false }
    { ::new (&this->what_) T{ std::move(that) }; }
    //! Construction by copying a (multi-level) pointer to self
    template <
        typename T,
        class EnableIf = typename std::enable_if<
            std::is_same<
                tagged_union,
                typename undecorate<T>::type
            >::value
        >::type
    >
    tagged_union( T const volatile *that ) noexcept
        : what_{}, which_{ &typeid(T const volatile *) }, self_{ true }
    { ::new (&this->what_) void const volatile *{ that }; }
    //! \overload
    template <
        typename T,
        class EnableIf = typename std::enable_if<
            std::is_same<
                tagged_union,
                typename undecorate<T>::type
            >::value
        >::type
    >
    tagged_union( T volatile *that ) noexcept
        : what_{}, which_{ &typeid(T volatile *) }, self_{ true }
    { ::new (&this->what_) void volatile *{ that }; }
    //! \overload
    template <
        typename T,
        class EnableIf = typename std::enable_if<
            std::is_same<
                tagged_union,
                typename undecorate<T>::type
            >::value
        >::type
    >
    tagged_union( T const *that ) noexcept
        : what_{}, which_{ &typeid(T const *) }, self_{ true }
    { ::new (&this->what_) void const *{ that }; }
    //! \overload
    template <
        typename T,
        class EnableIf = typename std::enable_if<
            std::is_same<
                tagged_union,
                typename undecorate<T>::type
            >::value
        >::type
    >
    tagged_union( T *that ) noexcept
        : what_{}, which_{ &typeid(T *) }, self_{ true }
    { ::new (&this->what_) void *{ that }; }

    //! Copy-constructor
    tagged_union( tagged_union const &that )
        : what_{}, which_{ that.which_ }, self_{ that.self_ }
    {
        if ( that.self_ )
            // Pointer-to-self is POD -> can use bitwise copying
            std::memcpy( &this->what_, &that.what_, sizeof(that.what_) );
        else if ( that.which_ )
            detail::visit_via_ref<Types...>( detail::copy_constructor{},
             *that.which_, &that.what_, static_cast<void *>(&this->what_) );
    }
    //! Move-constructor
    tagged_union( tagged_union &&that )
        : what_{}, which_{ that.which_ }, self_{ that.self_ }
    {
        if ( that.self_ )
            // Pointer-to-self is POD -> can use bitwise copying
            std::memmove( &this->what_, &that.what_, sizeof(that.what_) );
        else if ( that.which_ )
            detail::visit_via_rref<Types...>( detail::move_constructor{},
             *that.which_, &that.what_, static_cast<void *>(&this->what_) );
    }
    //! Destructor
    ~tagged_union()
    {
        if ( this->which_ && !this->self_ )
            detail::visit_via_ptr<Types...>( detail::destroyer{}, *this->which_,
             &this->what_ );
        // If "which_" is NULL, then there's no object to destroy.
        // If "self_" is TRUE, pointer-to-self is POD, so no need for dtr-call.
    }

    //! Copy-assignment
    tagged_union &  operator =( tagged_union const &that )
    {
		// Save the old data
		unsigned char  old_what[ sizeof(this->what_) ];
		auto           old_which = this->which_;
		auto           old_self = this->self_;

		std::memcpy( &old_what, &this->what_, sizeof(old_what) );

        // Look for the appropriate case
        if ( this->which_ && !this->self_ )
        {
            if ( that.which_ && !that.self_ )
            {
                if ( *this->which_ == *that.which_ )
                {
                    // The source and destination objects are of the same
                    // (non-pointer-to-self) type.  Let's use its
                    // copy-assignment operator.
                    detail::visit_via_ref<Types...>( detail::copy_assigner{},
                     *that.which_, &that.what_,
                     static_cast<void *>(&this->what_) );
                }
                else
                {
                    // The source and destination objects are of different
                    // non-pointer-to-self types.  We have to destroy the
                    // old data and copy-construct the new.  But we're going
                    // to place the new data first, then temporarily put the
                    // old data back in to delete it.
                    detail::visit_via_ref<Types...>( detail::copy_constructor{},
                     *that.which_, &that.what_,
                     static_cast<void *>(&this->what_) );

                    detail::memswap(&this->what_, &old_what, sizeof(old_what));
                    try {
                        detail::visit_via_ptr<Types...>( detail::destroyer{},
                         *old_which, &this->what_ );
                    } catch ( ... ) {
                        // We can't really deal with the old data's destructor
                        // throwing, especially since the new data's constructor
                        // succeeded.  So we pretend it didn't happen.
                    }
                    detail::memswap(&this->what_, &old_what, sizeof(old_what));
                    this->which_ = that.which_;
                    this->self_ = that.self_;
                }
            }
            else
            {
                // We're receiving either nothing or a pointer-to-self POD.
                // Neither case can throw, so let's get rid of the old data
                // first.  We can reuse the destructor's code.
                this->~tagged_union();
                std::memcpy( &this->what_, &that.what_, sizeof(that.what_) );
                this->which_ = that.which_;
                this->self_ = that.self_;
            }
        }
        else
        {
            // We're adding data on top of either nothing or a pointer-to-self
            // POD.  When it's nothing, we're doing what the copy constructor
            // does, so let's reuse the code.  When it's a pointer-to-self, we
            // can ignore what's there since its destruction is trivial.
            try {
                ::new ( static_cast<void *>(this) ) tagged_union{ that };
            } catch ( ... ) {
                std::memcpy( &this->what_, &old_what, sizeof(old_what) );
                this->which_ = old_which;
                this->self_ = old_self;
                throw;
            }
        }

        return *this;
    }
    //! Move-assignment
    tagged_union &  operator =( tagged_union &&that )
    {
        // The only case where a move is not always a copy is when the source
        // and destination store the same non-pointer-to-self type.
        if ( this->which_ && that.which_ && (*this->which_ == *that.which_)
         && !this->self_ )
        {
            // Let's use the type's move-assignment operator.
            detail::visit_via_rref<Types...>( detail::move_assigner{},
             *that.which_, &that.what_, static_cast<void *>(&this->what_) );
            return *this;
        }
        else
        {
            return this->operator =( const_cast<tagged_union const &>(that) );
        }
    }

    //! Return the address of the stored data, type-less, and NULL if none.
    auto  data() noexcept -> void *
    { return this->which_ ? static_cast<void *>(&this->what_) : nullptr; }
    //! \overload
    auto  data() const noexcept -> void const *
    { return const_cast<tagged_union *>(this)->data(); }
    //! \overload
    auto  data() volatile noexcept -> void volatile *
    { return const_cast<tagged_union *>(this)->data(); }
    //! \overload
    auto  data() const volatile noexcept -> void const volatile *
    { return const_cast<tagged_union *>(this)->data(); }

    //! Check the type of the object being stored, NULL if none
    auto  stored_type() const noexcept -> std::type_info const *
    { return this->which_; }
    //! Check if current object is a (multi-level) pointer-to-self type
    bool  storing_pointer_to_self() const noexcept
    { return this->self_; }

protected:
    //! Check for self-consistency
    bool  invariant() const
    {
        // "what_" can't be confirmed, but we can check that "self_" always
        // implies non-null "which_".  Also, the "which_" when "self_" is
        // True can't be in "variant_types()".
        auto const  types = tagged_union::variant_types();

        // "self_" shouldn't be on if "which_" isn't
        if ( this->self_ && !this->which_ )
            return false;
        // no object is stored
        if ( !this->which_ )
            return true;
        for ( auto const *t : types )
            // "self_" indicates when a type NOT in "variant_types()" is stored
            if ( *t == *this->which_ )
                return !this->self_;
        // "self_" being on is the only time an active type shouldn't be found
        // in "variant_types()".
        return this->self_;
    }

private:
    typename std::aligned_storage<  // Had aligned_union, but no GCC-4.7 support
        sizeof( typename largest_type<
            Types..., void *, void const *,
            void volatile *, void const volatile *
        >::type )
    >::type                 what_;   // storage for variant object
    std::type_info const *  which_;  // NULL for no object, else "what_"'s type
    bool                    self_;   // TRUE if "which_" is a pointer-to-self
};


//  Tracked type-tagged union metadata template specialization definitions  --//

//! Specialization of `variant_element` for `tagged_union`s.
template < std::size_t Index, typename ...Types >
struct variant_element<Index, tagged_union<Types...>>
{
    typedef typename mpl::type_at_v<Index, Types...>::type  type;
};  // Note that any repeats are included!

//! Specialization of `variant_size` for `tagged_union`s.
template < typename ...Types >
struct variant_size<tagged_union<Types...>>
    : std::integral_constant<std::size_t, sizeof...(Types)>
{ };  // Note that all repeats are included!


//  Tracked type-tagged union template data extraction functions  ------------//

//! Extract the variant member of the given type from the given `tagged_union`.
/** Besides the types explicitly given in `Types`, a type that is a pointer to
    cv-qualified `tagged_union` or a cv-qualified (multi-level) pointer to that
    pointer (e.g. `tagged_union\<Types...\>* const *`) may be used for `T`.

    \tparam T      The type of the desired variant member.  Must be explicitly
                   provided.
    \tparam Types  The variant types listed in `tu`'s type.  Should not be
                   explicitly provided.  This variadic list may be empty.

    \param[in] tu  The `tagged_union` object to be accessed.

    \throws  Nothing for the pointer versions.  If the wrong type is requested
             in the reference versions, `boost::bad_get` is thrown.

    \returns  `NULL` if `tu` is `NULL` or if the stored variant isn't of type
              `T`.  A pointer to the stored variant object otherwise.  (For the
              reference versions, a reference to the stored variant object is
              returned when the type request is right.)
 */
template < typename T, typename ...Types >
auto  gett( tagged_union<Types...> *tu ) -> T *
{
    return static_cast<T *>( (tu && tu->stored_type() && ( *tu->stored_type()
     == typeid(T) )) ? tu->data() : nullptr );
}

//! \overload
template < typename T, typename ...Types >
auto  gett( tagged_union<Types...> const *tu ) -> T const *
{
    return static_cast<T const *>( (tu && tu->stored_type() &&
     ( *tu->stored_type() == typeid(T) )) ? tu->data() : nullptr );
}

//! \overload
template < typename T, typename ...Types >
auto  gett( tagged_union<Types...> &tu ) -> T &
{
    if ( auto const  p = gett<T>(&tu) )
        return *p;
    else 
        throw bad_get{};
}

//! \overload
template < typename T, typename ...Types >
auto  gett( tagged_union<Types...> const &tu ) -> T const &
{
    if ( auto const  p = gett<T>(&tu) )
        return *p;
    throw bad_get{};
}

//! \overload
template < typename T, typename ...Types >
auto  gett( tagged_union<Types...> &&tu ) -> T &&
{
    return std::move( gett<T>(tu) );
}

//! Extract the variant member of the given index from the given `tagged_union`.
/** This accessor cannot be used to get the secret variants of type
    `tagged_union\<Types...\> CV *` (or cv-qualified pointers to those).  If
    `Index` is out of range, then a compile-time error will be eventually
    generated.

    \pre  `0 \<= Index \< #variant_size\<decltype(tu)\>\::value`

    \tparam Index  The type of the desired variant member.  Must be explicitly
                   provided.
    \tparam Types  The variant types listed in `tu`'s type.  Should not be
                   explicitly provided.  This variadic list may be empty.

    \param[in] tu  The `tagged_union` object to be accessed.

    \throws  Nothing for the pointer versions.  If the wrong type is requested
             in the reference versions, `boost::bad_get` is thrown.

    \returns  `NULL` if `tu` is `NULL` or if the stored variant isn't of the
              requested type.  A pointer to the stored variant object otherwise.
              (For the reference versions, a reference to the stored variant
              object is returned when the type request is right.)
 */
template < std::size_t Index, typename ...Types >
auto  get( tagged_union<Types...> *tu )
 -> typename variant_element<Index, tagged_union<Types...>>::type *
{
    return gett<typename variant_element<Index, tagged_union<Types...>>::type>(
     tu );
}

//! \overload
template < std::size_t Index, typename ...Types >
auto  get( tagged_union<Types...> const *tu )
 -> typename variant_element<Index, tagged_union<Types...>>::type const *
{
    return gett<typename variant_element<Index, tagged_union<Types...>>::type>(
     tu );
}

//! \overload
template < std::size_t Index, typename ...Types >
auto  get( tagged_union<Types...> &tu )
 -> typename variant_element<Index, tagged_union<Types...>>::type &
{
    return gett<typename variant_element<Index, tagged_union<Types...>>::type>(
     tu );
}

//! \overload
template < std::size_t Index, typename ...Types >
auto  get( tagged_union<Types...> const &tu )
 -> typename variant_element<Index, tagged_union<Types...>>::type const &
{
    return gett<typename variant_element<Index, tagged_union<Types...>>::type>(
     tu );
}

//! \overload
template < std::size_t Index, typename ...Types >
auto  get( tagged_union<Types...> &&tu )
 -> typename variant_element<Index, tagged_union<Types...>>::type &&
{
    return gett<typename variant_element<Index, tagged_union<Types...>>::type>(
     std::forward<tagged_union<Types...>>(tu) );
}


}  // namespace unions
}  // namespace boost


#endif  // BOOST_UNIONS_TAGGED_UNION_HPP

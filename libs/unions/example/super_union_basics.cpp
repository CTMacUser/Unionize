//  Boost Unions Library, basic super_union example program file  ------------//

//  Copyright 2012 Daryle Walker.  Use, modification, and distribution are
//  subject to the Boost Software License, Version 1.0.  (See accompanying file
//  LICENSE_1_0.txt or a copy at <http://www.boost.org/LICENSE_1_0.txt>.)

//  See <http://www.boost.org/libs/unions/> for the library's home page.

#include "boost/unions/super_union.hpp"  // for boost::unions::super_union

#include <array>     // for std::array
#include <iostream>  // for std::cout
#include <ostream>   // for std::basic_ostream, endl
#include <typeinfo>  // for std::type_info


// The union we'll we working with...
typedef boost::unions::super_union<int, double>  intdouble_t;

// ...And the access functions needed.
using boost::unions::gett;
using boost::unions::get;


// Generators
intdouble_t  make_int()
{
    intdouble_t  result;

    gett<int>( result ) = -2;
    return result;
}

intdouble_t  make_double()
{
    intdouble_t  result;

    gett<double>( result ) = 3.75;
    return result;
}

// Reporters
template < typename Ch, class Tr, typename InIter >
std::basic_ostream<Ch, Tr> &
print_type_list( std::basic_ostream<Ch, Tr> &o, InIter b, InIter e )
{
    o << "Listed types: ";
    if ( b == e )
    {
        o << "(None)";
    }
    else
    {
        o << (**b).name();
        while ( ++b != e )
        {
            o << ", " << (*b)->name();
        }
    }
    return o << std::endl;
}


// Main program
int  main()
{
    using std::cout;
    using std::endl;

    intdouble_t        test1 = make_double();
    intdouble_t const &test2 = test1;

    cout << "The number: " << gett<double>(test1) << '\n';
    cout << "And again: " << gett<double>(test2) << '\n';
    test1 = make_int();
    cout << "Changed number: " << gett<int>(test1) << '\n';
    cout << "Re-printing: " << gett<int>(test2) << '\n';
    cout << "Printing immediate: " << gett<double>(make_double()) << '\n';
    cout << "And another: " << gett<int>(make_int()) << endl;

    get<0>(test1) = +5;
    cout << "Changed number through index: " << get<0>(test1) << '\n';
    cout << "Print as constant: " << get<0>(test2) << '\n';
    test1 = make_double();
    get<1>(test1) = -7.1;
    cout << "Indexed after another change: " << get<1>(test1) << '\n';
    cout << "And another constant print: " << get<1>(test2) << '\n';
    cout << "Print immediate through index: " << get<1>(make_double()) << '\n';
    cout << "And another time: " << get<0>(make_int()) << endl;

    boost::unions::super_union<>  test3{};
    auto const                    types1 = intdouble_t::variant_types();
    auto const                    types2 = decltype(test3)::variant_types();

    print_type_list( cout, types1.begin(), types1.end() );
    print_type_list( cout, types2.begin(), types2.end() );

    return 0;
}

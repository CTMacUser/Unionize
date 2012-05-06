//  Boost Unions Library, basic super_union example program file  ------------//

//  Copyright 2012 Daryle Walker.  Use, modification, and distribution are
//  subject to the Boost Software License, Version 1.0.  (See accompanying file
//  LICENSE_1_0.txt or a copy at <http://www.boost.org/LICENSE_1_0.txt>.)

//  See <http://www.boost.org/libs/unions/> for the library's home page.

#include "boost/unions/super_union.hpp"  // for boost::unions::super_union

#include <iostream>  // for std::cout
#include <ostream>   // for std::endl


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

    return 0;
}

//  Boost Unions Library, super_union compile-time-fail test file  -----------//

//  Copyright 2012 Daryle Walker.  Use, modification, and distribution are
//  subject to the Boost Software License, Version 1.0.  (See accompanying file
//  LICENSE_1_0.txt or a copy at <http://www.boost.org/LICENSE_1_0.txt>.)

//  See <http://www.boost.org/libs/unions/> for the library's home page.

#include "boost/unions/super_union.hpp"  // for boost::unions::super_union


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

    get<1>( result ) = 3.75;
    return result;
}


// Main program
int  main()
{
    intdouble_t        test1 = make_double();
    intdouble_t const &test2 = test1;

    auto const  t1 = gett<short>( test1 );       // should fail
    auto const  t2 = gett<long>( test2 );        // should fail
    auto const  t3 = gett<float>( make_int() );  // should fail

    test1 = make_int();

    auto const  t4 = get<2>( test1 );            // should fail
    auto const  t5 = get<6>( test2 );            // should fail
    auto const  t6 = get<9>( make_double() );    // should fail

    return 0;
}

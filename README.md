Unionize
========

Extended union types in C++11

This C++ library explores advanced definitions of union-types, using C++11
features.  As such, your compiler (and standard library) needs to be set to
run in C++11 mode.  The library has (so far):

-  `super_union`, a `union` that has its variant member types listed as
   template parameters.  You can ignore having to recall the name (or name
   path) for each variant member and just address them by type.  There is
   an option to address them via type index.
-  `tagged_union`, a `union` that works like `super_union` except it also
   keeps track of which variant member is current.  Addressing members must
   be done through the custom access functions.
-  `variant_size` and `variant_element`, analogs to the meta-functions
   `std::tuple_size` and `std::tuple_element` that support the `std::tuple`
   (and `std::pair` and `std::array`) class templates.  These class templates
   are specialized for the provided advanced union types in this library, and
   should be specialized for any union-like types you make.

The format of the library is like a prototype Boost library submission, which
it may become.  As such, the code is under the Boost license.

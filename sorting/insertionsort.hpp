//
// dsa is a utility library of data structures and algorithms built with C++11.
// This file (insertionsort.hpp) is part of the dsa project.
//
// A description of the insertionsort algorithm can be found here:
//
//      https://en.wikipedia.org/wiki/Insertion_sort
//
// author: Dalton Woodard
// contact: daltonmwoodard@gmail.com
// repository: https://github.com/daltonwoodard/sorting.git
// license:
//
// Copyright (c) 2016 Dalton Woodard. See the COPYRIGHT.md file at the top-level
// directory or at the listed source repository for details.
//
//      Licensed under the Apache License. Version 2.0:
//          https://www.apache.org/licenses/LICENSE-2.0
//      or the MIT License:
//          https://opensource.org/licenses/MIT
//      at the licensee's option. This file may not be copied, modified, or
//      distributed except according to those terms.
//

#ifndef DSA_SORTING_INSERTIONSORT_HPP
#define DSA_SORTING_INSERTIONSORT_HPP

#include <functional>   // std::less
#include <iterator>     // std::iterator_traits
#include <utility>      // std::swap


namespace dsa
{
    /*
     * insertionsort sorting algorithm
     *
     * A description of the insertionsort algorithm can be found here:
     *
     *      https://en.wikipedia.org/wiki/Insertion_sort
     *
     *  Description
     *  -----------
     *
     *  insertionsort is a stable sorting algorithm with O(1) space complexity,
     *  O(n) best case and O(n^2) average and worse case time complexities . It
     *  is suitable for sorting small sequences.
     *
     *  Template Parameters
     *  -------------------
     *  - RandomIt: an iterator type modeling RandomAccessIterator
     *  - Compare: a function type with signature comparable to
     *
     *      bool cmp (T1 const &, T2 const &)
     *
     *    where T1 and T2 are such that an object of type RandomIt can be
     *    dereferenced and then implicitly converted to both types.
     *
     *  Parameters
     *  ----------
     *  - RandomIt first: an iterator to the first element in the range to sort
     *  - RandomIt last: an iterator to one past the end of the range to sort
     *  - Compare comp: the comparison function object to be used in sorting
     */
    template <typename RandomIt, typename Compare>
    void insertionsort (RandomIt first, RandomIt last, Compare comp)
    {
        auto const index {first};
        auto const length {static_cast <std::size_t> (last - first)};

        for (std::size_t i = 1; i < length; ++i) {
            for (auto j = i; j > 0 && comp (index [j], index [j -1]); --j) {
                std::swap (index [j], index [j -1]);
            }
        }
    }

    template <typename RandomIt>
    void insertionsort (RandomIt first, RandomIt last)
    {
        using value_type = typename std::iterator_traits <RandomIt>::value_type;
        insertionsort (first, last, std::less <value_type> {});
    }
}   // namespace dsa

#endif  // #ifndef DSA_SORTING_BLOCKSORT_HPP

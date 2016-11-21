//
// dsa is a utility library of data structures and algorithms built with C++11.
// This file (shellsort.hpp) is part of the dsa project.
//
// A description of the shellsort algorithm can be found here:
//
//      https://en.wikipedia.org/wiki/Shellsort
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

#ifndef DSA_SORTING_SHELLSORT_HPP
#define DSA_SORTING_SHELLSORT_HPP

#include <array>        // std::array
#include <cstddef>      // std::size_t
#include <functional>   // std::less
#include <iterator>     // std::iterator_traits
#include <utility>      // std::swap


namespace dsa
{
namespace
{
    static constexpr std::array <std::size_t, 8>
        curia01_seq {{701, 301, 132, 57, 23, 10, 4, 1}};

    static constexpr std::array <std::size_t, 9>
        tokuda92_seq {{1182, 525, 233, 103, 46, 20, 9, 4, 1}};

    static constexpr std::array <std::size_t, 6>
        sedgewick86_seq {{1073, 281, 77, 23, 8, 1}};
}   // anonymous namespace

    enum class shellseq
    {
        curia01     = 0,  // Curia 2001
        tokuda92    = 1,  // Tokuda 1992
        sedgewick86 = 2,  // Sedgewick 1986
    };

    /*
     * shellsort sorting algorithm
     *
     * A description of the shellsort algorithm can be found here:
     *
     *      https://en.wikipedia.org/wiki/Shellsort
     *
     *  Description
     *  -----------
     *
     *  shellsort is a non-stable sorting algorithm with O(1) space complexity,
     *  O(n·log n) best case, O(n·log^2 n) average case, and a (sometimes) unknown
     *  worst case time complexity (depending on chosen gap-sequence). It is
     *  suitable for sorting small sequences and generally performs better than
     *  a standard insertion sort.
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
     *  - enum shellseq gapseq: the gap sequence to use in sorting
     *      [default: curia01]
     */
    template <typename RandomIt, typename Compare>
    void shellsort (
        RandomIt first, RandomIt last, Compare comp,
        enum shellseq seq = shellseq::curia01
    )
    {
        auto const index {first};
        auto const length {static_cast <std::size_t> (last - first)};
        auto const inner_loop {
            [index, length, comp] (std::size_t g) {
                for (std::size_t i = g; i < length; ++i) {
                    auto const tmp {index [i]};

                    std::size_t j;
                    for (j = i; j >= g && comp (tmp, index [j - g]); j -= g) {
                        index [j] = index [j - g];
                    }

                    index [j] = tmp;
                }
            }
        };

        switch (seq) {
            case shellseq::curia01:
                for (std::size_t g : curia01_seq) {
                    inner_loop (g);
                }
                break;
            case shellseq::tokuda92:
                for (std::size_t g : tokuda92_seq) {
                    inner_loop (g);
                }
                break;
            case shellseq::sedgewick86:
                for (std::size_t g : sedgewick86_seq) {
                    inner_loop (g);
                }
                break;
        }
    }

    template <typename RandomIt>
    void shellsort (
        RandomIt first, RandomIt last, enum shellseq seq = shellseq::curia01
    )
    {
        using value_type = typename std::iterator_traits <RandomIt>::value_type;
        shellsort (first, last, std::less <value_type> {}, seq);
    }
}   // namespace dsa

#endif  // #ifndef DSA_SORTING_BLOCKSORT_HPP

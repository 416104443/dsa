//
// dsa is a utility library of data structures and algorithms built with C++11.
// This file (blocksort.hpp) is part of the dsa project.
//
// A description of the blocksort algorithm can be found here:
//
//      https://en.wikipedia.org/wiki/Block_sort
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

#ifndef DSA_SORTING_BLOCKSORT_HPP
#define DSA_SORTING_BLOCKSORT_HPP

#include <algorithm>    // std::rotate, std::inplace_merge
#include <array>        // std::array
#include <cstddef>      // std::size_t
#include <functional>   // std::less
#include <iterator>     // std::iterator_traits


namespace dsa
{
namespace
{
    /* specialized shellsort variant for sorting ranges of length 16-31 */
    static constexpr std::array <std::size_t, 4>
        curia_gap_sequence {{23, 10, 4, 1}};

    template <typename RandomIt, typename Compare>
    static void shellsort (RandomIt first, RandomIt last, Compare comp)
    {
        auto const index {first};
        auto const length {static_cast <std::size_t> (last - first)};

        for (std::size_t g : curia_gap_sequence) {
            for (std::size_t i = g; i < length; ++i) {
                auto const tmp {index [i]};

                std::size_t j;
                for (j = i; j >= g && comp (tmp, index [j - g]); j -= g) {
                    index [j] = index [j - g];
                }

                index [j] = tmp;
            }
        }
    }
}   // anonymous namespace

    /*
     * blocksort sorting algorithm
     *
     * A description of the blocksort algorithm can be found here:
     *
     *      https://en.wikipedia.org/wiki/Block_sort
     *
     *  Description
     *  -----------
     *
     *  blocksort is a stable sorting algorithm with O(1) space complexity, O(n)
     *  best case and O(n·log n) average and worse case time complexities. It is
     *  an adaptive, hybrid algorithm combining merge operations and insertion
     *  sorts (in our implementation the insertion sort is replaced by a
     *  shellsort). It is suitable for cases when a standard merge sort has
     *  favorable performance characteristics, but additional space for a work
     *  buffer is not available.
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
    void blocksort (RandomIt first, RandomIt last, Compare comp)
    {
        auto const rangelen {static_cast <std::uint64_t> (last - first)};

        /* previous power of 2 below rangelen */
        auto rl = rangelen;
        std::size_t const rl_pow2 = (
            rl |= rl >> 1,
            rl |= rl >> 2,
            rl |= rl >> 4,
            rl |= rl >> 8,
            rl |= rl >> 16,
            rl |= rl >> 32,
            rl - (rl >> 1)
        );

        /*
         * scale is a multiplier between 1.0 and 2.0 determining the length of
         * the blocks to be worked on in the first portion of the algorithm
         */
        auto const scale {
            static_cast <double> (rangelen) /
                static_cast <double> (rl_pow2)
        };

        /* sort blocks (of length 16-31, depending on scale) */
        for (std::size_t b = 0; b < rl_pow2; b += 16) {
            auto const block_start {
                first + static_cast <std::size_t> (b * scale)
            };
            auto const block_end {
                first + static_cast <std::size_t> ((b + 16) * scale)
            };

            shellsort (block_start, block_end, comp);
        }

        /* rotate and merge increasingly long ranges to finish sort */
        for (std::size_t len = 16; len < rl_pow2; len *= 2) {
            for (std::size_t merge = 0; merge < rl_pow2; merge += len * 2) {
                auto const range_start {
                    first + static_cast <std::size_t> (merge * scale)
                };
                auto const range_mid {
                    first + static_cast <std::size_t> ((merge + len) * scale)
                };
                auto const range_end {
                    first + static_cast <std::size_t> (
                        (merge + (len * 2)) * scale
                    )
                };

                if (comp (*(range_end - 1), *range_start)) {
                    std::rotate (range_start, range_mid, range_end);
                } else if (comp (*range_mid, *(range_mid - 1))) {
                    std::inplace_merge (
                        range_start, range_mid, range_end, comp
                    );
                }
            }
        }
    }

    template <typename RandomIt>
    void blocksort (RandomIt first, RandomIt last)
    {
        using value_type = typename std::iterator_traits <RandomIt>::value_type;
        blocksort (first, last, std::less <value_type> {});
    }
}   // namespace dsa

#endif  // #ifndef DSA_SORTING_BLOCKSORT_HPP

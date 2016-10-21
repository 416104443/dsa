//
// dsa is a utility library of data structures and algorithms built with C++11.
// This file (bst.hpp) is part of the dsa project.
//
// bstree; a generic binary search-tree (with unique keys) implementation for C++11
// or later.
//
// A description of a binary search tree can be found here:
//
//      https://en.wikipedia.org/wiki/Binary_search_tree
//
// author: Dalton Woodard
// contact: daltonmwoodard@gmail.com
// repository: https://github.com/daltonwoodard/bstree.hpp
// license:
//
// Copyright (c) 2016 DaltonWoodard. See the COPYRIGHT.md file at the top-level
// directory or at the listed source repository for details.
//
//      Licensed under the Apache License. Version 2.0:
//          https://www.apache.org/licenses/LICENSE-2.0
//      or the MIT License:
//          https://opensource.org/licenses/MIT
//      at the licensee's option. This file may not be copied, modified, or
//      distributed except according to those terms.
//

#ifndef DSA_BSTREE_HPP
#define DSA_BSTREE_HPP

#include <algorithm>        // std::equal, std::lexicographical_compare,
                            // std::max
#include <cassert>          // assert
#include <cstddef>          // std::ptrdiff_t, std::size_t
#include <exception>        // std::current_exception, std::exception_ptr,
                            // std::rethrow_exception
#include <functional>       // std::function, std::less
#include <initializer_list> // std::initializer_list
#include <tuple>            // std::tuple
#include <type_traits>      // 
#include <memory>           // std::allocator, std::allocator_traits,
                            // std::unique_ptr
#include <utility>          // std::pair


namespace dsa
{
namespace detail
{
    // used to differentiate itertor from const_iterator
    struct mutable_tag {};
    struct const_tag {};

    template <class Node, class Value, class Tag>
    struct bstree_iterator
    {
        enum class state
        {
            invalid,
            valid,
            after_end,
        };

        Node * _iter;
        enum state _state;

        bstree_iterator (Node * i, enum state s = state::valid)
            noexcept
            : _iter  {i}
            , _state {s}
        {}

        bstree_iterator & operator= (Node * i) noexcept
        {
            this->_iter = i;
            return *this;
        }

        bool valid (void) const noexcept
        {
            return !this->_iter || this->_state == state::invalid;
        }

        using difference_type   = std::ptrdiff_t;
        using value_type        = Value;
        using pointer           = value_type const *;
        using reference         = value_type const &;
        using iterator_category = std::bidirectional_iterator_tag;

        // default construction is a singular iterator
        bstree_iterator (void)
            : _iter  {nullptr}
            , _state {state::after_end}
        {}

        ~bstree_iterator (void) = default;

        bstree_iterator (bstree_iterator const &) noexcept
            = default;
        bstree_iterator & operator= (bstree_iterator const &)
            noexcept
            = default;

        template <
            class OTag,
            typename = typename std::enable_if <
                std::is_same <Tag, OTag>::value ||
                (std::is_same <Tag, const_tag>::value &&
                 std::is_same <OTag, mutable_tag>::value)
            >::type
        >
        bstree_iterator (bstree_iterator <Node, Value, OTag> const & other)
            noexcept
            : _iter  {other._iter}
            , _state {other._state}
        {}

        template <
            class OTag,
            typename = typename std::enable_if <
                std::is_same <Tag, OTag>::value ||
                (std::is_same <Tag, const_tag>::value &&
                 std::is_same <OTag, mutable_tag>::value)
            >::type
        >
        bstree_iterator &
            operator= (bstree_iterator <Node, Value, OTag> const & other) noexcept
        {
            this->_iter = other._iter;
            this->_state = other._state;
        }

        void swap (bstree_iterator & other) noexcept
        {
            std::swap (this->_iter, other._iter);
            std::swap (this->_state, other._state);
        }

        reference operator* (void) const noexcept
        {
            return this->_iter->value;
        }

        pointer operator-> (void) const noexcept
        {
            return &this->_iter->value;
        }

        bstree_iterator & operator++ (void) noexcept
        {
            if (this->_iter &&
                this->_state != state::after_end &&
                this->_state != state::invalid)
            {
                auto n = this->_iter;
                // Can we move down and to the right? Then move right
                // and down to the left to reach the in-order successor.
                if (n->right) {
                    n = n->right;
                    while (n->left) {
                        n = n->left;
                    }
                    this->_iter = n;
                // We can't move right, which means we must move upwards
                // to reach the in-order successor or we must fall off the
                // end.
                } else {
                    // If we're not the root we have two cases:
                    if (n->parent) {
                        // if we're the left child then move upwards
                        if (n == n->parent->left) {
                            this->_iter = n->parent;
                        // if we're the right child then move upwards and
                        // try to find an ancestor node from the left
                        } else {
                            auto it = n->parent;
                            while (true) {
                                // we've reached the root from the right,
                                // so fall off the end
                                if (!it->parent) {
                                    this->_state = state::after_end;
                                    break;
                                // otherwise check if we move up from the
                                // left, and in this case we're done
                                } else if (it->parent->left == it) {
                                    n = it->parent;
                                    break;
                                // otherwise continue upwards
                                } else {
                                    it = it->parent;
                                    continue;
                                }
                            }

                            this->_iter = n;
                        }
                    // if we are the root then we fall off the end
                    } else {
                        this->_state = state::after_end;
                    }
                }
            }

            return *this;
        }

        bstree_iterator operator++ (int) noexcept
        {
            auto tmp = *this;
            ++(*this);
            return tmp;
        }

        bstree_iterator & operator-- (void) noexcept
        {
            if (this->_iter &&
                this->_state != state::after_end &&
                this->_state != state::invalid)
            {
                auto n = this->_iter;
                // Can we move down and to the left? Then move left and down
                // to the right to reach the in-order predecessor.
                if (n->left) {
                    n = n->left;
                    while (n->right) {
                        n = n->right;
                    }
                    this->_iter = n;
                // We can't move left, which means we must move upwards
                // to reach the in-order predecessor or we must fall of the
                // front.
                } else {
                    // If we're not the root we have two cases:
                    if (n->parent) {
                        // if we're the right child then move upwards
                        if (n == n->parent->right) {
                            this->_iter = n->parent;
                        // if we're the left child then move upwards and try
                        // to find an ancestor node from the right
                        } else {
                            auto it = n->parent;
                            while (true) {
                                // we've reached the root from the left,
                                // so we're in an invalid state
                                if (!it->parent) {
                                    this->_state = state::invalid;
                                    this->_iter = nullptr;
                                // otherwise check if we move up from the
                                // right, and in this case we're done
                                } else if (it->parent->right == it) {
                                    n = it->parent;
                                // otherwise continue upwards
                                } else {
                                    it = it->parent;
                                    continue;
                                }
                            }
                        }
                    // if we are the root then we're in an invalid state
                    } else {
                        this->_state = state::invalid;
                        this->_iter = nullptr;
                    }
                }
            } else if (this->_iter && this->_state == state::after_end) {
                this->_state = state::valid;
            }

            return *this;
        }

        bstree_iterator operator-- (int) noexcept
        {
            auto tmp = *this;
            --(*this);
            return tmp;
        }

        bool operator== (bstree_iterator const & other) const noexcept
        {
            if (this->_state == state::valid &&
                other._state == state::valid)
            {
                return this->_iter == other._iter;
            } else if (this->_state == state::after_end &&
                       other._state == state::after_end)
            {
                // this check is a formality; they *should* compare equal
                // unless one of them is a stale iterator.
                return this->_iter == other._iter;
            } else {
                // invalid iterators have fallen off the front, so they
                // should not compare equal to one another; likewise,
                // iterators of different states should not compare equal.
                return false;
            }
        }

        bool operator!= (bstree_iterator const & other) const noexcept
        {
            return !(*this == other);
        }
    };
}   // namespace detail

    template <
        class Key,
        class Compare = std::less <Key>,
        class Allocator = std::allocator <Key>
    >
    class bstree
    {
    public:
        using key_type               = Key;
        using value_type             = Key;
        using size_type              = std::size_t;
        using difference_type        = std::ptrdiff_t;
        using key_compare            = Compare;
        using value_compare          = Compare;
        using allocator_type         = Allocator;

        using reference = typename std::add_lvalue_reference <value_type>::type;
        using const_reference = typename std::add_lvalue_reference <
            typename std::add_const <value_type>::type
        >::type;
        using pointer = typename std::allocator_traits <Allocator>::pointer;
        using const_pointer = typename std::allocator_traits <
            Allocator
        >::const_pointer;

    private:
        using alloc_traits = std::allocator_traits <allocator_type>;
        using propogate_on_copy_assign =
            typename alloc_traits::propogate_on_container_copy_assignment;
        using propogate_on_move_assign =
            typename alloc_traits::propogate_on_container_move_assignment;
        using propogate_on_swap =
            typename alloc_traits::propogate_on_container_swap;

        struct node
        {
            value_type value;
            node * left;
            node * right;
            node * parent;

            node (value_type && val, node * p = nullptr)
                : value  {std::move (val)}
                , left   {nullptr}
                , right  {nullptr}
                , parent {p}
            {}

            node (value_type const & val, node * p = nullptr)
                : value  {val}
                , left   {nullptr}
                , right  {nullptr}
                , parent {p}
            {}

            node (node && other)
                noexcept (
                    std::is_nothrow_move_constructible <value_type>::value
                )
                : value  {std::move (other.value)}
                , left   {other.left}
                , right  {other.right}
                , parent {other.parent}
            {
                other.left   = nullptr;
                other.right  = nullptr;
                other.parent = nullptr;
            }

            ~node (void)
                noexcept (std::is_nothrow_destructible <value_type>::value)
                = default;

            void swap (node & other)
                noexcept (noexcept (
                    std::swap (
                        std::declval <value_type &> (),
                        std::declval <value_type &> ()
                    )
                ))
            {
                using std::swap;
                swap (this->value, other.value);

                std::swap (this->left, other.left);
                std::swap (this->right, other.right);
                std::swap (this->parent, other.parent);
            }
        };

    public:
        using iterator = detail::bstree_iterator <
            node, value_type, detail::mutable_tag
        >;
        using const_iterator = detail::bstree_iterator <
            node, value_type, detail::const_tag
        >;
        using reverse_iterator       = std::reverse_iterator <iterator>;
        using const_reverse_iterator = std::reverse_iterator <const_iterator>;

    private:
        using node_allocator = typename allocator_type::template rebind <node>;
        using tree_deleter   = std::function <void (node *)>;
        using node_handle    = std::unique_ptr <node, tree_deleter>;

        template <class ... Args>
        static node * node_create (node * const parent,
                                   node_allocator & alloc,
                                   Args && ... args)
        {
            auto mem = alloc.allocate (1);

            try {
                ::new (mem) node {
                    value_type (std::forward <Args> (args)...), parent
                };
            } catch (...) {
                alloc.deallocate (mem, 1);
                std::rethrow_exception (std::current_exception ());
            }

            return static_cast <node *> (mem);
        }

        static void tree_delete (node * n, node_allocator & alloc)
        {
            if (!n)
                return;

            while (n != nullptr)
            {
                if (n->left) {
                    n = n->left;
                } else if (n->right) {
                    n = n->right;
                } else {
                    auto p = n->parent;
                    if (p != nullptr) {
                        if (n == p->left)
                            p->left = nullptr;
                        else if (n == p->right)
                            p->right = nullptr;
                    }
                    (&n->value)->~value_type ();
                    alloc.deallocate (n, 1);
                    n = p;
                }
            }
        }

        static node_handle
            copy_from (node_handle const & nh, node_allocator & a)
        {
            using namespace std::placeholders;
            node_handle result {nullptr, std::bind (tree_delete, _1, a)};

            if (nh == nullptr)
                return result;

            // copy root value
            result.reset (node_create (nullptr, a, nh->value));

            // copy tree
            auto copy_walk = nh.get ();
            auto result_walk = result.get ();

            while (copy_walk != nullptr)
            {
                // Can/Should we move down the left branch? If so, copy that
                // node and walk down and to the left.
                if (copy_walk->left && !result_walk->left) {
                    result_walk->left = node_create (
                        result_walk, a, copy_walk->left->value
                    );
                    copy_walk = copy_walk->left;
                    result_walk = result_walk->left;
                // Can/Should we move down the right branch? If so, copy that
                // node and walk down and to the right.
                } else if (copy_walk->right && !result_walk->right) {
                    result_walk->right = node_create (
                        result_walk, a, copy_walk->right->value
                    );
                    copy_walk = copy_walk->right;
                    result_walk = result_walk->right;
                // Otherwise we're either at a leaf node or a node that has
                // already been copied, an in either case we should move up.
                } else {
                    copy_walk = copy_walk->parent;
                    result_walk = result_walk->parent;
                }
            }

            return result;
        }

        static node_handle move_from (node_handle & nh, node_allocator & a)
        {
            using namespace std::placeholders;
            node_handle result {nullptr, std::bind (tree_delete, _1, a)};

            if (nh == nullptr)
                return result;

            // copy root value
            result.reset (node_create (nullptr, a, nh->value));

            // copy tree
            auto copy_walk = nh.get ();
            auto result_walk = result.get ();

            while (copy_walk != nullptr)
            {
                // Can/Should we move down the left branch? If so, copy that
                // node and walk down and to the left.
                if (copy_walk->left && !result_walk->left) {
                    result_walk->left = node_create (
                        result_walk, a, std::move (copy_walk->left->value)
                    );
                    copy_walk = copy_walk->left;
                    result_walk = result_walk->left;
                // Can/Should we move down the right branch? If so, copy that
                // node and walk down and to the right.
                } else if (copy_walk->right && !result_walk->right) {
                    result_walk->right = node_create (
                        result_walk, a, std::move (copy_walk->right->value)
                    );
                    copy_walk = copy_walk->right;
                    result_walk = result_walk->right;
                // Otherwise we're either at a leaf node or a node that has
                // already been copied, an in either case we should move up.
                } else {
                    copy_walk = copy_walk->parent;
                    result_walk = result_walk->parent;
                }
            }

            return result;
        }

        template <class ... Args>
        void make_root (Args && ... args)
        {
            this->_tree_root.reset (
                node_create (
                    nullptr, this->_node_alloc, std::forward <Args> (args)...
                )
            );
            this->_begin = iterator {this->_tree_root.get ()};
            this->_end = iterator {
                this->_tree_root.get (), iterator::state::after_end
            };
            this->_tree_size = 1;
        }

        void make_root (node * n)
        {
            this->_tree_root.reset (n);
            this->_begin = iterator {this->_tree_root.get ()};
            this->_end = iterator {
                this->_tree_root.get (), iterator::state::after_end
            };
            this->_tree_size = 1;
        }

        template <class ... Args>
        void insert_left (node * const parent, Args && ... args)
        {
            parent->left = node_create (
                parent, this->_node_alloc, std::forward <Args> (args)...
            );
            this->_tree_size += 1;

            if (parent == this->_begin) {
                this->_begin = parent->left;
            }
        }

        void insert_left (node * const parent, node * const n)
        {
            parent->left = n;
            n->parent = parent;
            this->_tree_size += 1;

            if (parent == this->_begin) {
                this->_begin = parent->left;
            }
        }

        template <class ... Args>
        void insert_left (iterator parent, Args && ... args)
        {
            this->insert_left (parent._iter, std::forward <Args> (args)...);
        }

        void insert_left (iterator parent, node * const n)
        {
            this->insert_left (parent._iter, n);
        }

        template <class ... Args>
        void insert_right (node * const parent, Args && ... args)
        {
            parent->right = node_create (
                parent, this->_node_alloc, std::forward <Args> (args)...
            );
            this->_tree_size += 1;

            if (parent == this->_end) {
                this->_end = parent->right;
            }
        }

        void insert_right (node * const parent, node * const n)
        {
            parent->right = n;
            n->parent = parent;
            this->_tree_size += 1;

            if (parent == this->_end) {
                this->_end = parent->right;
            }
        }

        template <class ... Args>
        void insert_right (iterator parent, Args && ... args)
        {
            this->insert_right (parent._iter, std::forward <Args> (args)...);
        }

        void insert_right (iterator parent, node * const n)
        {
            this->insert_right (parent._iter, n);
        }

        iterator insert_at (iterator pos, value_type const & value)
        {
            if (this->_key_comp (value, pos->value)) {
                assert (!pos->left);
                this->insert_left (pos._iter, value);
                if (pos == this->_begin) {
                    this->_begin = iterator {pos->left};
                }
                return iterator {pos->left};
            } else if (this->_key_comp (pos->value, value)) {
                assert (!pos->right);
                this->insert_right (pos._iter, value);
                if (pos == this->_end) {
                    this->_end = iterator {pos->right};
                }
                return iterator {pos->right};
            } else {
                return pos;
            }
        }

        std::pair <iterator, bool> insert_at (iterator pos, node * const n)
        {
            if (this->_key_comp (n->value, pos->value)) {
                assert (!pos->left);
                this->insert_left (pos._iter, n);
                if (pos == this->_begin) {
                    this->_begin = iterator {pos->left};
                }
                return std::make_pair (iterator {pos->left}, true);
            } else if (this->_key_comp (pos->value, n->value)) {
                assert (!pos->right);
                this->insert_right (pos._iter, n);
                if (pos == this->_end) {
                    this->_end = iterator {pos->right};
                }
                return std::make_pair (iterator {pos->right}, true);
            } else {
                return std::make_pair (pos, false);
            }
        }

        key_compare    _key_comp;
        node_allocator _node_alloc;
        node_handle    _tree_root;
        size_type      _tree_size;
        iterator       _begin;
        iterator       _end;

        void set_iterators (void) noexcept
        {
            if (this->_tree_size > 0) {
                auto l = this->_tree_root.get ();
                while (l->left) {
                    l = l->left;
                }
                this->_begin = l;

                auto r = this->_tree_root.get ();
                while (r->right) {
                    r = r->right;
                }
                this->_end = iterator {r, iterator::state::after_end};
            } else {
                this->_begin = iterator {nullptr};
                this->_end = iterator {nullptr};
            }
        }

    public:
        bstree (void)
            : bstree (Compare (), Allocator ())
        {}

        ~bstree (void) = default;

        explicit bstree (Compare const & key_comp,
                         Allocator const & key_alloc = Allocator ())
            : _key_comp   {key_comp}
            , _node_alloc {key_alloc}
            , _tree_root  {
                nullptr,
                std::bind (
                    tree_delete, std::placeholders::_1, this->_node_alloc
                )
            }
            , _tree_size {0}
            , _begin     {}
            , _end       {}
        {}

        explicit bstree (Allocator const & key_alloc)
            : _key_comp   {}
            , _node_alloc {key_alloc}
            , _tree_root  {
                nullptr,
                std::bind (
                    tree_delete, std::placeholders::_1, this->_node_alloc
                )
            }
            , _tree_size {0}
            , _begin     {}
            , _end       {}
        {}

        template <class InputIt>
        bstree (InputIt first,
                InputIt last,
                Compare const & key_comp    = Compare (),
                Allocator const & key_alloc = Allocator ())
            : _key_comp   {key_comp}
            , _node_alloc {key_alloc}
            , _tree_root  {
                nullptr,
                std::bind (
                    tree_delete, std::placeholders::_1, this->_node_alloc
                )
            }
            , _tree_size {0}
            , _begin     {}
            , _end       {}
        {
            this->insert (first, last);
        }

        bstree (bstree const & other, Allocator const & alloc)
            : _key_comp   {other._key_comp}
            , _node_alloc {alloc}
        {
            this->_tree_root = copy_from (other._tree_root, this->_node_alloc);
            this->_tree_size = other._tree_size;
            this->set_iterators ();
        }

        bstree (bstree const & other)
            : bstree (
                other,
                alloc_traits::select_on_container_copy_construction (
                    other.get_allocator ()
                )
            )
        {}

        bstree (bstree && other, Allocator const & alloc)
            : _key_comp   {std::move (other._key_comp)}
            , _node_alloc {alloc}
            , _tree_root  {std::move (other._tree_root)}
            , _tree_size  {other._tree_size}
            , _begin      {other._begin}
            , _end        {other._end}
        {
            other._tree_size = 0;
            other._begin = iterator {};
            other._end = iterator {};
        }

        bstree (bstree && other)
            noexcept (std::is_nothrow_move_constructible <Allocator>::value &&
                      std::is_nothrow_move_constructible <Compare>::value)
            : _key_comp   {std::move (other._key_comp)}
            , _node_alloc {std::move (other._node_alloc)}
            , _tree_root  {std::move (other._tree_root)}
            , _tree_size  {other._tree_size}
            , _begin      {other._begin}
            , _end        {other._end}
        {
            other._tree_size = 0;
            other._begin = iterator {};
            other._end = iterator {};
        }

        bstree (std::initializer_list <value_type> init,
                Allocator const & alloc)
            : bstree (init.begin (), init.end (), Compare (), alloc)
        {}

        bstree & operator= (bstree const & other)
        {
            this->_key_comp = other._key_comp;

            if (propogate_on_copy_assign::value) {
                if (this->_node_alloc != other._node_alloc) {
                    this->_tree_root.reset ();
                }

                this->_node_alloc = other._node_alloc;
            }

            this->_tree_root = copy_from (other._tree_root, this->_node_alloc);
            this->_tree_size = other._tree_size;
            this->set_iterators ();
            return *this;
        }

        bstree & operator= (bstree && other)
            noexcept (
                alloc_traits::is_always_equal::value &&
                std::is_nothrow_move_assignable <Compare>::value
            )
        {
            this->_key_comp = other._key_comp;

            if (propogate_on_move_assign::value) {
                this->clear ();
                this->_node_alloc = other._node_alloc;
                std::swap (this->_tree_root, other._tree_root);
                std::swap (this->_tree_size, other._tree_size);
                this->_begin.swap (other._begin);
                this->_end.swap (other._end);
            } else {
                if (this->_node_alloc != other._node_alloc) {
                    this->clear ();
                    this->_tree_root = move_from (
                        other._tree_root, this->_node_alloc
                    );
                    this->_tree_size = other._tree_size;
                    this->set_iterators ();
                    other.clear ();
                } else {
                    this->clear ();
                    std::swap (this->_tree_root, other._tree_root);
                    std::swap (this->_tree_size, other._tree_size);
                    this->_begin.swap (other._begin);
                    this->_end.swap (other._end);
                }
            }

            return *this;
        }

        bstree & operator= (std::initializer_list <value_type> ilist)
        {
            this->clear ();
            this->insert (ilist.begin (), ilist.end ());
            return *this;
        }

        void swap (bstree & other) noexcept
        {
            using std::swap;

            if (propogate_on_swap::value) {
                swap (this->_node_alloc, other._node_alloc);
            }

            swap (this->_key_comp, other._key_comp);
            swap (this->_tree_size, other._tree_size);
            swap (this->_tree_root, other._tree_root);
            this->_begin.swap (other._begin);
            this->_end.swap (other._end);
        }

        allocator_type get_allocator (void) const
        {
            return allocator_type {this->_node_alloc};
        }

        key_compare key_comp (void) const
        {
            return this->_key_comp;
        }

        value_compare value_comp (void) const
        {
            return this->key_comp ();
        }

        size_type size (void) const noexcept
        {
            return this->_tree_size;
        }

        size_type max_size (void) const noexcept
        {
            return std::numeric_limits <size_type>::max ();
        }

        bool empty (void) const noexcept
        {
            return this->_tree_size == 0;
        }

        iterator begin (void) noexcept
        {
            return this->_begin;
        }

        const_iterator begin (void) const noexcept
        {
            return const_iterator {this->_begin._iter};
        }

        const_iterator cbegin (void) const noexcept
        {
            return const_iterator {this->_begin._iter};
        }

        iterator end (void) noexcept
        {
            return this->_end;
        }

        const_iterator end (void) const noexcept
        {
            return const_iterator {this->_end._iter};
        }

        const_iterator cend (void) const noexcept
        {
            return const_iterator {this->_end._iter};
        }

        reverse_iterator rbegin (void) noexcept
        {
            return reverse_iterator {this->_end};
        }

        const_reverse_iterator rbegin (void) const noexcept
        {
            return const_reverse_iterator {this->cend ()};
        }

        const_reverse_iterator crbegin (void) const noexcept
        {
            return const_reverse_iterator {this->cend ()};
        }

        reverse_iterator rend (void) noexcept
        {
            return reverse_iterator {this->_begin};
        }

        const_reverse_iterator rend (void) const noexcept
        {
            return const_reverse_iterator {this->cbegin ()};
        }

        const_reverse_iterator crend (void) const noexcept
        {
            return const_reverse_iterator {this->cbegin ()};
        }

        void clear (void) noexcept
        {
            this->_tree_size = 0;
            this->_begin = iterator {};
            this->_end = iterator {};
            this->_tree_root.reset ();
        }

        iterator erase (iterator pos)
        {
            auto n = pos._iter;
            auto const retval = ++pos;

            if (!n->left && !n->right) {
                if (this->_tree_root == n) {
                    this->_tree_root.reset ();
                } else if (n->parent->left == n) {
                    n->parent->left = nullptr;
                } else if (n->parent->right == n) {
                    n->parent->right = nullptr;
                }

                if (this->_begin == n) {
                    this->_begin = n->parent;
                } else if (this->_end == n) {
                    this->_end = n->parent;
                }
            } else if (!n->left) {
                if (this->_tree_root == n) {
                    this->_tree_root.release ();
                    this->_tree_root.reset (n->right);
                } else if (n->parent->left == n) {
                    n->parent->left = n->right;
                } else if (n->parent->right == n) {
                    n->parent->right = n->right;
                }

                n->right->parent = n->parent;

                if (this->_begin == n) {
                    this->_begin = n->parent;
                }
            } else if (!n->right) {
                if (this->_tree_root == n) {
                    this->_tree_root.release ();
                    this->_tree_root.reset (n->left);
                } else if (n->parent->left == n) {
                    n->parent->left = n->left;
                } else if (n->parent->right == n) {
                    n->parent->right = n->left;
                }

                n->left->parent = n->parent;

                if (this->_end == n) {
                    this->_end = n->parent;
                }
            } else {
                // we have to remove a node with two children; since we have
                // access to the node's parent, we can shuffle around the
                // children nodes as follows:
                //
                //      if tree_size is even, then:
                //          replace n with n's left-child, L, and insert n's
                //          right-child, R, into L's right sub-tree.
                //      if tree_size is odd, then
                //          replace n with n's right-child, R, and insert n's
                //          left-child, L, into R's left sub-tree.
                //
                // where the conditional flips each time based on the size of
                // the tree in order to prevent the pathological removal case
                // in which the tree degenerates over time.
                //
                auto const parity = this->_tree_size % 2 == 0;

                if (this->_tree_root == n) {
                    this->_tree_root.release ();
                    if (parity) {
                        this->_tree_root.reset (n->left);
                        n->left->parent = nullptr;
                    } else {
                        this->_tree_root.reset (n->right);
                        n->right->parent = nullptr;
                    }
                } else {
                    auto parent = n->parent;
                    auto branch = n == parent->left ? &parent->left
                                                    : &parent->right;
                    if (parity) {
                        *branch = n->left;
                        n->left->parent = *branch;
                    } else {
                        *branch = n->right;
                        n->right->parent = *branch;
                    }
                }

                // now insert one child into the other's subtree; we know that
                // the values in the tree are unique, so we do not have to
                // consider the case when two values compare equal.
                auto it = parity ? n->left : n->right;
                auto in = parity ? n->right : n->left;
                while (true)
                {
                    if (this->_key_comp (in->value, it->value)) {
                         if (it->left) {
                            it = it->left;
                            continue;
                         } else {
                            it->left = in;
                            in->parent = it;
                            break;
                         }
                    } else {
                        if (it->right) {
                            it = it->right;
                            continue;
                        } else {
                            it->right = in;
                            in->parent = it;
                            break;
                        }
                    }
                }
            }

            try {
                (&n->value)->~value_type ();
            } catch (...) {
                this->_tree_size -= 1;
                this->_node_alloc.deallocate (n, 1);
                std::rethrow_exception (std::current_exception ());
            }

            this->_tree_size -= 1;
            this->_node_alloc.deallocate (n, 1);
            return retval;
        }

        iterator erase (const_iterator first, const_iterator last)
        {
            for (auto it = first; it != last; ++it)
                (void) this->erase (it);

            return last;
        }

        size_type erase (key_type const & key)
        {
            auto n = this->_tree_root.get ();

            // find the erasure point, if it exists, and then perform erasure
            while (true) {
                // key is less than that of n, so walk the left-branch
                if (this->_key_comp (key, n->value)) {
                    if (n->left) {
                        n = n->left;
                        continue;
                    } else {
                        // key was not found, so return 0
                        break;
                    }
                // key is greater than that of n, so walk the right-branch
                } else if (this->_key_comp (n->value, key)) {
                    if (n->right) {
                        n = n->right;
                        continue;
                    } else {
                        // key was not found, so return 0
                        break;
                    }
                // value compares equal to that of n, so erase here
                } else {
                    (void) this->erase (iterator {n});
                    return 1;
                }
            }

            return 0;
        }

        std::pair <iterator, bool> insert (value_type const & value)
        {
            if (this->empty ()) {
                this->make_root (value);
                return std::make_pair (iterator {this->_tree_root}, true);
            } else {
                auto n = this->_tree_root.get ();

                // find insertion point and then perform insertion
                while (true) {
                    // value is less than that of n, so walk the left-branch
                    if (this->_key_comp (value, n->value)) {
                        if (n->left) {
                            n = n->left;
                            continue;
                        } else {
                            this->insert_left (n, value);
                            return std::make_pair (iterator {n->left}, true);
                        }
                    // value is greater than that of n, so walk the right-branch
                    } else if (this->_key_comp (n->value, value)) {
                        if (n->right) {
                            n = n->right;
                            continue;
                        } else {
                            this->insert_right (n, value);
                            return std::make_pair (iterator {n->right}, true);
                        }
                    // value compares equal to that of n, so we cannot insert
                    } else {
                        return std::make_pair (iterator {n}, false);
                    }
                }
            }
        }

        std::pair <iterator, bool> insert (value_type && value)
        {
            if (this->empty ()) {
                this->make_root (std::move (value));
                return std::make_pair (iterator {this->_tree_root}, true);
            } else {
                auto n = this->_tree_root.get ();

                // find insertion point and then perform insertion
                while (true) {
                    // value is less than that of n, so walk the left-branch
                    if (this->_key_comp (value, n->value)) {
                        if (n->left) {
                            n = n->left;
                            continue;
                        } else {
                            this->insert_left (n, std::move (value));
                            return std::make_pair (iterator {n->left}, true);
                        }
                    // value is greater than that of n, so walk the right-branch
                    } else if (this->_key_comp (n->value, value)) {
                        if (n->right) {
                            n = n->right;
                            continue;
                        } else {
                            this->insert_right (n, std::move (value));
                            return std::make_pair (iterator {n->right}, true);
                        }
                    // value compares equal to that of n, so we cannot insert
                    } else {
                        return std::make_pair (iterator {n}, false);
                    }
                }
            }
        }

        iterator insert (const_iterator hint, value_type const & value)
        {
            auto pos = iterator {hint._iter};

            // attempt iteration backwards to find the lower bound of value
            if (this->_key_comp (value, pos->value)) {
                while (pos != this->_begin &&
                       this->_key_comp (value, pos->value))
                {
                    --pos;
                }
            // otherwise, attempt iteration forwards to find the upper bound of
            // value
            } else {
                while (pos != this->_end &&
                       this->_key_comp (pos->value, value))
                {
                    ++pos;
                }
            }

            return this->insert_at (pos, value);
        }

        iterator insert (const_iterator hint, value_type && value)
        {
            auto pos = iterator {hint._iter};

            // attempt iteration backwards to find the lower bound of value
            if (this->_key_comp (value, pos->value)) {
                while (pos != this->_begin &&
                       this->_key_comp (value, pos->value))
                {
                    --pos;
                }
            // otherwise, attempt iteration forwards to find the upper bound of
            // value
            } else {
                while (pos != this->_end &&
                       this->_key_comp (pos->value, value))
                {
                    ++pos;
                }
            }

            return this->insert_at (pos, std::move (value));
        }

        template <class InputIt>
        void insert (InputIt first, InputIt last)
        {
            for (auto it = first; it != last; ++it)
                this->insert (*it);
        }

        void insert (std::initializer_list <value_type> ilist)
        {
            this->insert (ilist.begin (), ilist.end ());
        }

        template <class ... Args>
        std::pair <iterator, bool> emplace (Args && ... args)
        {
            if (this->empty ()) {
                this->make_root (std::forward <Args> (args)...);
                return std::make_pair (iterator {this->_tree_root}, true);
            } else {
                auto it = this->_tree_root.get ();
                auto in = node_create (
                    nullptr, this->_node_alloc, std::forward <Args> (args)...
                );

                try {
                    // find insertion point and then perform insertion
                    while (true) {
                        // value is less than that of it, so walk the
                        // left-branch
                        if (this->_key_comp (in->value, it->value)) {
                            if (it->left) {
                                it = it->left;
                                continue;
                            } else {
                                this->insert_left (it, in);
                                return std::make_pair (
                                    iterator {it->left}, true
                                );
                            }
                        // value is greater than that of it, so walk the
                        // right-branch
                        } else if (this->_key_comp (it->value, in->value)) {
                            if (it->right) {
                                it = it->right;
                                continue;
                            } else {
                                this->insert_right (it, in);
                                return std::make_pair (
                                    iterator {it->right}, true
                                );
                            }
                        // value compares equal to that of it, so we cannot
                        // insert
                        } else {
                            (&in->value)->~value_type ();
                            this->_node_alloc.deallocate (in, 1);
                            return std::make_pair (iterator {it}, false);
                        }
                    }
                } catch (...) {
                    (&in->value)->~value_type ();
                    this->_node_alloc.deallocate (in, 1);
                    std::rethrow_exception (std::current_exception ());
                }
            }
        }

        template <class ... Args>
        iterator emplace_hint (const_iterator hint, Args && ... args)
        {
            auto pos = iterator {hint._iter};
            auto in = node_create (
                nullptr, this->_node_alloc, std::forward <Args> (args)...
            );

            try {
                // attempt iteration backwards to find the lower bound of value
                if (this->_key_comp (in->value, pos->value)) {
                    while (pos != this->_begin &&
                           this->_key_comp (in->value, pos->value))
                    {
                        --pos;
                    }
                // otherwise, attempt iteration forwards to find the upper bound
                // of value
                } else {
                    while (pos != this->_end &&
                           this->_key_comp (pos->value, in->value))
                    {
                        ++pos;
                    }
                }
            } catch (...) {
                (&in->value)->~value_type ();
                this->_node_alloc.deallocate (in, 1);
                std::rethrow_exception (std::current_exception ());
            }

            auto result = this->insert_at (pos, in);
            if (!result.second) {
                (&in->value)->~value_type ();
                this->_node_alloc.deallocate (in, 1);
            }
            return result.first;
        }

        size_type count (key_type const & key) const
        {
            if (this->empty ()) {
                return 0;
            }

            auto n = this->_tree_root.get ();

            // find the location of key, if it exists
            while (true) {
                // key is less than the value at n, so walk the left-branch
                if (this->_key_comp (key, n->value)) {
                    if (n->left) {
                        n = n->left;
                        continue;
                    } else {
                        // key was not found, so return 0
                        break;
                    }
                // key is greater than the value at n, so walk the right-branch
                } else if (this->_key_comp (n->value, key)) {
                    if (n->right) {
                        n = n->right;
                        continue;
                    } else {
                        // key was not found, so return 0
                        break;
                    }
                // key compares equal to the value at n, so return 1
                } else {
                    return 1;
                }
            }

            return 0;
        }

        template <
            class K,
            typename = typename std::enable_if <
                key_compare::is_transparent::value
            >::type
        >
        size_type count (K const & x) const
        {
            if (this->empty ()) {
                return 0;
            }

            auto n = this->_tree_root.get ();

            // find the location of x, if it exists
            while (true) {
                // x is less than the value at n, so walk the left-branch
                if (this->_key_comp (x, n->value)) {
                    if (n->left) {
                        n = n->left;
                        continue;
                    } else {
                        // x was not found, so return 0
                        break;
                    }
                // x is greater than the value at n, so walk the right-branch
                } else if (this->_key_comp (n->value, x)) {
                    if (n->right) {
                        n = n->right;
                        continue;
                    } else {
                        // x was not found, so return 0
                        break;
                    }
                // x compares equal to the value at n, so return 1
                } else {
                    return 1;
                }
            }

            return 0;
        }

    private:
        iterator find_impl (key_type const & key) const
        {
            if (this->empty ()) {
                return this->_end;
            }

            auto n = this->_tree_root.get ();

            // find the location of key, if it exists
            while (true) {
                // key is less than the value at n, so walk the left-branch
                if (this->_key_comp (key, n->value)) {
                    if (n->left) {
                        n = n->left;
                        continue;
                    } else {
                        // key was not found, so return end
                        break;
                    }
                // key is greater than the value at n, so walk the right-branch
                } else if (this->_key_comp (n->value, key)) {
                    if (n->right) {
                        n = n->right;
                        continue;
                    } else {
                        // key was not found, so return end
                        break;
                    }
                // key compares equal to the value at n, so return 1
                } else {
                    return iterator {n};
                }
            }

            return this->_end;
        }

        template <
            class K,
            typename = typename std::enable_if <
                key_compare::is_transparent::value
            >::type
        >
        iterator find_impl (K const & x) const
        {
            if (this->empty ()) {
                return this->_end;
            }

            auto n = this->_tree_root.get ();

            // find the location of x, if it exists
            while (true) {
                // x is less than the value at n, so walk the left-branch
                if (this->_key_comp (x, n->value)) {
                    if (n->left) {
                        n = n->left;
                        continue;
                    } else {
                        // x was not found, so return end
                        break;
                    }
                // x is greater than the value at n, so walk the right-branch
                } else if (this->_key_comp (n->value, x)) {
                    if (n->right) {
                        n = n->right;
                        continue;
                    } else {
                        // x was not found, so return end
                        break;
                    }
                // x compares equal to the value at n, so return 1
                } else {
                    return iterator {n};
                }
            }

            return this->_end;
        }

    public:
        iterator find (key_type const & key)
        {
            return this->find_impl (key);
        }

        const_iterator find (key_type const & key) const
        {
            return const_iterator {this->find_impl (key)};
        }

        template <
            class K,
            typename = typename std::enable_if <
                key_compare::is_transparent::value
            >::type
        >
        iterator find (K const & x)
        {
            return this->find_impl (x);
        }

        template <
            class K,
            typename = typename std::enable_if <
                key_compare::is_transparent::value
            >::type
        >
        const_iterator find (K const & x) const
        {
            return const_iterator {this->find_impl (x)};
        }

    private:
        iterator lower_bound_impl (key_type const & key) const
        {
            if (this->empty ()) {
                return this->_end;
            }

            // we wish to find the first value not less than key (i.e., greater
            // than or equal to key)
            auto pos = iterator {this->_tree_root.get ()};

            // if we start off ahead of key, then move backwards
            if (this->_key_comp (key, pos->value)) {
                while (this->_key_comp (key, pos->value)) {
                    if (this->_begin == pos) {
                        return pos;
                    } else {
                        --pos;
                    }
                }

                // if we went behind key, move forward by one
                if (this->_key_comp (pos->value, key)) {
                    return ++pos;
                // otherwise pos->value and key compare equal
                } else {
                    return pos;
                }
            // otherwise, if we start off behind key, then move forwards
            } else if (this->_key_comp (pos->value, key)) {
                while (this->_key_comp (pos->value, key)) {
                    if (this->_end == pos) {
                        return pos;
                    } else {
                        ++pos;
                    }
                }

                // we haven't reached the end at this point, which means we
                // must be greater than or equal to key, so simply return
                return pos;
            // otherwise, if we start off equal to key, return this position
            } else {
                return pos;
            }
        }

        template <
            class K,
            typename = typename std::enable_if <
                key_compare::is_transparent::value
            >::type
        >
        iterator lower_bound_impl (K const & x) const
        {
            if (this->empty ()) {
                return this->_end;
            }

            // we wish to find the first value not less than key (i.e., greater
            // than or equal to x)
            auto pos = iterator {this->_tree_root.get ()};

            // if we start off ahead of x, then move backwards
            if (this->_key_comp (x, pos->value)) {
                while (this->_key_comp (x, pos->value)) {
                    if (this->_begin == pos) {
                        return pos;
                    } else {
                        --pos;
                    }
                }

                // if we went behind x, move forward by one
                if (this->_key_comp (pos->value, x)) {
                    return ++pos;
                // otherwise pos->value and x compare equal
                } else {
                    return pos;
                }
            // otherwise, if we start off behind x, then move forwards
            } else if (this->_key_comp (pos->value, x)) {
                while (this->_key_comp (pos->value, x)) {
                    if (this->_end == pos) {
                        return pos;
                    } else {
                        ++pos;
                    }
                }

                // we haven't reached the end at this point, which means we
                // must be greater than or equal to x, so simply return
                return pos;
            // otherwise, if we start off equal to x, return this position
            } else {
                return pos;
            }
        }

        iterator upper_bound_impl (key_type const & key) const
        {
            if (this->empty ()) {
                return this->_end;
            }

            // we wish to find the first value greater than key
            auto pos = iterator {this->_tree_root.get ()};

            // if we start off ahead of key, then move backwards
            if (this->_key_comp (key, pos->value)) {
                while (this->_key_comp (key, pos->value)) {
                    if (this->_begin == pos) {
                        return pos;
                    } else {
                        --pos;
                    }
                }

                // we haven't reached the beginning at this point, which
                // means key must be greater than or equal to the value
                // at pos, so in either case move forward by one
                return ++pos;
            // otherwise, if we start off behind key, then move forwards
            } else if (this->_key_comp (pos->value, key)) {
                while (this->_key_comp (pos->value, key)) {
                    if (this->_end == pos) {
                        return pos;
                    } else {
                        ++pos;
                    }
                }

                // if we went ahead of key, return this position
                if (this->_key_comp (key, pos->value)) {
                    return pos;
                // otherwise pos->value and key compare equal, so move forward
                // by one
                } else {
                    return ++pos;
                }
            // otherwise, if we start off equal to key, return the next position
            } else {
                return ++pos;
            }
        }

        template <
            class K,
            typename = typename std::enable_if <
                key_compare::is_transparent::value
            >::type
        >
        iterator upper_bound_impl (K const & x) const
        {
            if (this->empty ()) {
                return this->_end;
            }

            // we wish to find the first value greater than x
            auto pos = iterator {this->_tree_root.get ()};

            // if we start off ahead of x, then move backwards
            if (this->_key_comp (x, pos->value)) {
                while (this->_key_comp (x, pos->value)) {
                    if (this->_begin == pos) {
                        return pos;
                    } else {
                        --pos;
                    }
                }

                // we haven't reached the beginning at this point, which
                // means x must be greater than or equal to the value
                // at pos, so in either case move forward by one
                return ++pos;
            // otherwise, if we start off behind x, then move forwards
            } else if (this->_key_comp (pos->value, x)) {
                while (this->_key_comp (pos->value, x)) {
                    if (this->_end == pos) {
                        return pos;
                    } else {
                        ++pos;
                    }
                }

                // if we went ahead of x, return this position
                if (this->_key_comp (x, pos->value)) {
                    return pos;
                // otherwise pos->value and x compare equal, so move forward
                // by one
                } else {
                    return ++pos;
                }
            // otherwise, if we start off equal to x, return the next position
            } else {
                return ++pos;
            }
        }

    public:
        iterator lower_bound (key_type const & key)
        {
            return this->lower_bound_impl (key);
        }

        const_iterator lower_bound (key_type const & key) const
        {
            return const_iterator {this->lower_bound_impl (key)};
        }

        template <
            class K,
            typename = typename std::enable_if <
                key_compare::is_transparent::value
            >::type
        >
        iterator lower_bound (K const & x)
        {
            return this->lower_bound_impl (x);
        }

        template <
            class K,
            typename = typename std::enable_if <
                key_compare::is_transparent::value
            >::type
        >
        const_iterator lower_bound (K const & x) const
        {
            return const_iterator {this->lower_bound_impl (x)};
        }

        iterator upper_bound (key_type const & key)
        {
            return this->upper_bound_impl (key);
        }

        const_iterator upper_bound (key_type const & key) const
        {
            return const_iterator {this->upper_bound_impl (key)};
        }

        template <
            class K,
            typename = typename std::enable_if <
                key_compare::is_transparent::value
            >::type
        >
        iterator upper_bound (K const & x)
        {
            return this->upper_bound_impl (x);
        }

        template <
            class K,
            typename = typename std::enable_if <
                key_compare::is_transparent::value
            >::type
        >
        const_iterator upper_bound (K const & x) const
        {
            return const_iterator {this->upper_bound_impl (x)};
        }

        std::pair <iterator, iterator> equal_range (key_type const & key)
        {
            // find the lower bound; since we have unique keys the upper bound
            // is always just one-past the lower bound.
            auto const lower = this->lower_bound (key);

            // increment on this->_end returns the same value, so it's okay
            // to increment lower in any case
            auto upper = lower;
            ++upper;

            return std::make_pair (lower, upper);
        }

        std::pair <const_iterator, const_iterator>
            equal_range (key_type const & key) const
        {
            // find the lower bound; since we have unique keys the upper bound
            // is always just one-past the lower bound.
            auto const lower = this->lower_bound (key);

            // increment on this->_end returns the same value, so it's okay
            // to increment lower in any case
            auto upper = lower;
            ++upper;

            return std::make_pair (lower, upper);
        }

        template <
            class K,
            typename = typename std::enable_if <
                key_compare::is_transparent::value
            >::type
        >
        std::pair <iterator, iterator> equal_range (K const & x)
        {
            // find the lower bound; since we have unique keys the upper bound
            // is always just one-past the lower bound.
            auto const lower = this->lower_bound (x);

            // increment on this->_end returns the same value, so it's okay
            // to increment lower in any case
            auto upper = lower;
            ++upper;

            return std::make_pair (lower, upper);
        }

        template <
            class K,
            typename = typename std::enable_if <
                key_compare::is_transparent::value
            >::type
        >
        std::pair <const_iterator, const_iterator> equal_range (K const & x)
        {
            // find the lower bound; since we have unique keys the upper bound
            // is always just one-past the lower bound.
            auto const lower = this->lower_bound (x);

            // increment on this->_end returns the same value, so it's okay
            // to increment lower in any case
            auto upper = lower;
            ++upper;

            return std::make_pair (lower, upper);
        }
    };

    template <class Key, class Compare, class Alloc>
    bool operator== (bstree <Key, Compare, Alloc> const & lhs,
                     bstree <Key, Compare, Alloc> const & rhs)
    {
        return lhs.size () == rhs.size () &&
            std::equal (lhs.begin (), lhs.end (), rhs.begin ());
    }

    template <class Key, class Compare, class Alloc>
    bool operator!= (bstree <Key, Compare, Alloc> const & lhs,
                     bstree <Key, Compare, Alloc> const & rhs)
    {
        return !(lhs == rhs);
    }

    template <class Key, class Compare, class Alloc>
    bool operator< (bstree <Key, Compare, Alloc> const & lhs,
                    bstree <Key, Compare, Alloc> const & rhs)
    {
        return std::lexicographical_compare (
            lhs.begin (), lhs.end (), rhs.begin (), rhs.end ()
        );
    }

    template <class Key, class Compare, class Alloc>
    bool operator> (bstree <Key, Compare, Alloc> const & lhs,
                    bstree <Key, Compare, Alloc> const & rhs)
    {
        return rhs < lhs;
    }

    template <class Key, class Compare, class Alloc>
    bool operator<= (bstree <Key, Compare, Alloc> const & lhs,
                     bstree <Key, Compare, Alloc> const & rhs)
    {
        return !(lhs > rhs);
    }

    template <class Key, class Compare, class Alloc>
    bool operator>= (bstree <Key, Compare, Alloc> const & lhs,
                     bstree <Key, Compare, Alloc> const & rhs)
    {
        return !(lhs < rhs);
    }
}   // namespace dsa

#endif  // #ifndef DSA_BSTREE_HPP

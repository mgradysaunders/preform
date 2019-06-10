/* Copyright (c) 2018-19 M. Grady Saunders
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *   1. Redistributions of source code must retain the above
 *      copyright notice, this list of conditions and the following
 *      disclaimer.
 *
 *   2. Redistributions in binary form must reproduce the above
 *      copyright notice, this list of conditions and the following
 *      disclaimer in the documentation and/or other materials
 *      provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/*+-+*/
#if !DOXYGEN
#if !(__cplusplus >= 201703L)
#error "preform/aabbtree.hpp requires >=C++17"
#endif // #if !(__cplusplus >= 201703L)
#endif // #if !DOXYGEN
#pragma once
#ifndef PREFORM_AABBTREE_HPP
#define PREFORM_AABBTREE_HPP

// for assert
#include <cassert>

// for std::uint32_t, std::uint8_t
#include <cstdint>

// for std::array
#include <array>

// for std::allocator, std::allocator_traits
#include <memory>

// for std::vector
#include <vector>

// for std::pair
#include <utility>

// for std::atomic
#include <atomic>

// for std::thread
#include <thread>

// for std::mutex
#include <mutex>

// for std::future, std::async
#include <future>

// for pr::range
#include <preform/range.hpp>

// for pr::static_stack
#include <preform/static_stack.hpp>

// for pr::aabb, pr::multi
#include <preform/aabb.hpp>

namespace pr {

/**
 * @defgroup aabbtree Axis-aligned bounding box tree
 *
 * `<preform/aabbtree.hpp>`
 *
 * __C++ version__: >=C++17
 */
/**@{*/

/**
 * @brief Axis-aligned bounding box tree.
 */
template <
    typename Tfloat, std::size_t N,
    typename Tsplit_mode,
    typename Tnode_alloc = std::allocator<char>
    >
class aabbtree
{
public:

    // Sanity check.
    static_assert(
        std::is_floating_point<Tfloat>::value,
        "Tfloat must be floating point");

    /**
     * @name Container typedefs
     */
    /**@{*/

    /**
     * @brief Size type.
     */
    typedef std::size_t size_type;

    /**
     * @brief Floating point type.
     */
    typedef Tfloat float_type;

#if !DOXYGEN
    // prototype
    struct node_type;
#endif // #if !DOXYGEN

    /**
     * @brief Node allocator type.
     */
    typedef typename std::allocator_traits<Tnode_alloc>::
            template rebind_alloc<node_type> node_allocator_type;

    /**
     * @brief Node allocator traits.
     */
    typedef typename std::allocator_traits<Tnode_alloc>::
            template rebind_traits<node_type> node_allocator_traits;

    /**@}*/

public:

    /**
     * @brief Proxy type.
     */
    struct proxy_type
    {
    public:

        /**
         * @brief Box.
         */
        aabb<float_type, N> box;

        /**
         * @brief Box center.
         */
        multi<float_type, N> box_center;

        /**
         * @brief Value index.
         */
        size_type value_index;
    };

    /**
     * @brief Node type.
     */
    struct node_type
    {
    public:

        /**
         * @brief Box.
         */
        aabb<float_type, N> box;

        /**
         * @brief If branch, left child.
         */
        node_type* left;

        /**
         * @brief If branch, right child.
         */
        node_type* right;

        /**
         * @brief If branch, split dimension.
         */
        size_type split_dim;

        /**
         * @brief If leaf, first proxy index.
         */
        size_type first_index;

        /**
         * @brief If leaf, proxy count.
         */
        size_type count;
    };

public:

    /**
     * @brief Default constructor.
     */
    aabbtree() = default;

    /**
     * @brief Constructor.
     *
     * @param[in] leaf_cutoff
     * Leaf cutoff.
     *
     * @param[in] node_alloc
     * Node allocator.
     */
    aabbtree(
        size_type leaf_cutoff,
        const node_allocator_type& node_alloc =
              node_allocator_type()) :
            leaf_cutoff_(leaf_cutoff),
            node_alloc_(node_alloc)
    {
        // Sanity check.
        assert(
            leaf_cutoff_ <
            size_type(256));
    }

    /**
     * @brief Non-copyable.
     */
    aabbtree(const aabbtree&) = delete;

    /**
     * @brief Destructor.
     */
    ~aabbtree()
    {
        clear();
    }

public:

    /**
     * @brief Initialize.
     *
     * Initializes axis-aligned bounding box tree by
     * 1. converting input range to axis-aligned bounding box
     * proxy representation with predicate function,
     * 2. generating tree recursively from the top down, splitting
     * according to `Tsplit_mode`.
     *
     * @param[in] from
     * Input range begin.
     *
     * @param[in] to
     * Input range end.
     *
     * @param[in] func
     * Function converting input to axis-aligned bounding box.
     *
     * @note
     * Function must have signature equivalent to
     * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{cpp}
     * aabb<float_type, N>(const Tvalue&)
     * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
     * where `Tvalue` is the value type corresponding to
     * `Tforward_itr`.
     */
    template <typename Tforward_itr, typename Tfunc>
    void init(
            Tforward_itr from,
            Tforward_itr to,
            Tfunc&& func)
    {
        // Clear.
        clear();

        // Count.
        size_type count = std::distance(from, to);
        if (count < 1) {
            return;
        }

        // Initialize proxies.
        size_type value_index = 0;
        while (from != to) {
            aabb<float_type, N> box = std::forward<Tfunc>(func)(*from);
            assert((box[0] < box[1]).all());
            proxies_.emplace_back(
            proxy_type{
                box,
                box.center(),
                value_index
            });
            ++from;
            ++value_index;
        }

        // Initialize.
        std::atomic<size_type> total_branches = 0;
        std::atomic<size_type> total_leaves = 0;
        size_type first_index = 0;
        root_ =
        init_recursive(
            total_branches,
            total_leaves,
            first_index,
            {&proxies_[0],
             &proxies_[0] + proxies_.size()});
        assert(first_index == proxies_.size());
        total_branches_ = total_branches;
        total_leaves_ = total_leaves;
    }

    /**
     * @brief Clear.
     */
    void clear()
    {
        // Destroy root.
        deallocate_recursive(root_);
        root_ = nullptr;
        total_branches_ = 0;
        total_leaves_ = 0;

        // Destroy proxies.
        proxies_.clear();
        proxies_.shrink_to_fit();
    }

public:

    /**
     * @name Accessors
     */
    /**@{*/

    /**
     * @brief Root.
     */
    const node_type* root() const
    {
        return root_;
    }

    /**
     * @brief Proxies.
     */
    const std::vector<proxy_type>& proxies() const
    {
        return proxies_;
    }

    /**@}*/

private:

    /**
     * @brief Leaf cutoff.
     */
    size_type leaf_cutoff_ = 8;

    /**
     * @brief Node allocator.
     */
    node_allocator_type node_alloc_;

    /**
     * @brief Node allocator mutual exclusion lock.
     */
    std::mutex node_alloc_mutex_;

    /**
     * @brief Root.
     */
    node_type* root_ = nullptr;

    /**
     * @brief Total branches.
     */
    size_type total_branches_ = 0;

    /**
     * @brief Total leaves.
     */
    size_type total_leaves_ = 0;

    /**
     * @brief Proxies.
     */
    std::vector<proxy_type> proxies_;

private:

    /**
     * @brief Allocate.
     */
    node_type* allocate()
    {
        std::unique_lock<std::mutex> lock(node_alloc_mutex_);
        return
            node_allocator_traits::allocate(
            node_alloc_, 1);
    }

    /**
     * @brief Deallocate.
     */
    void deallocate(node_type* node)
    {
        if (node) {
            std::unique_lock<std::mutex> lock(node_alloc_mutex_);
            node_allocator_traits::deallocate(
            node_alloc_, node, 1);
        }
    }

    /**
     * @brief Deallocate recursively.
     */
    void deallocate_recursive(node_type* node)
    {
        if (node) {
            // Recurse.
            deallocate_recursive(node->left);
            deallocate_recursive(node->right);

            // Deallocate.
            deallocate(node);
        }
    }

    /**
     * @brief Initialize recursively.
     */
    node_type* init_recursive(
            std::atomic<size_type>& total_branches,
            std::atomic<size_type>& total_leaves,
            size_type& first_index,
            range<proxy_type*> proxies)
    {
        // Allocate.
        node_type* node = allocate();

        // Surround boxes and box centers.
        aabb<float_type, N> box;
        aabb<float_type, N> box_center;
        assert((box[0] > box[1]).all());
        for (const proxy_type& proxy : proxies) {
            box |= proxy.box;
            box_center |= proxy.box_center;
        }

        // Proxies count.
        size_type count = proxies.size();
        assert(count);

        if (count <= leaf_cutoff_) {
            // Initialize leaf.
            *node = node_type{
                box,
                nullptr,
                nullptr,
                size_type(0),
                first_index,
                count
            };

            // Shift first index.
            first_index += count;

            // Increment.
            total_leaves++;
        }
        else {

            // Split dimension.
            size_type split_dim =
                        box_center.diag().argmax();

            // Split.
            proxy_type* split = Tsplit_mode()(
                    box,
                    box_center,
                    split_dim,
                    proxies);
            assert(split &&
                   proxies.begin() <= split &&
                   proxies.end() > split);

            // Children.
            node_type* left;
            node_type* right;

            // Count sufficiently small?
            if (count <= 16384) {

                // Recurse.
                left =
                init_recursive(
                    total_branches,
                    total_leaves,
                    first_index,
                    {proxies.begin(), split});

                // Recurse.
                right =
                init_recursive(
                    total_branches,
                    total_leaves,
                    first_index,
                    {split, proxies.end()});
            }
            else {

                // Recurse async.
                size_type first_index_left = first_index;
                first_index += std::distance(proxies.begin(), split);
                std::future<node_type*> future_left =
                std::async([&]() {
                    return init_recursive(
                                total_branches,
                                total_leaves,
                                first_index_left,
                                {proxies.begin(), split});
                });

                // Recurse.
                right =
                init_recursive(
                    total_branches,
                    total_leaves,
                    first_index,
                    {split, proxies.end()});

                // Wait.
                left = future_left.get();
            }

            // Initialize.
            *node = node_type{
                box,
                left,
                right,
                split_dim,
                size_type(0),
                size_type(0)
            };

            // Increment.
            total_branches++;
        }
        return node;
    }

    template <typename, std::size_t, typename>
    friend class linear_aabbtree;
};

/**
 * @brief Template alias for convenience.
 */
template <
    typename Tfloat, 
    typename Tsplit_mode,
    typename Tnode_alloc = std::allocator<char>
    >
using aabbtree2 = aabbtree<Tfloat, 2, Tsplit_mode, Tnode_alloc>;

/**
 * @brief Template alias for convenience.
 */
template <
    typename Tfloat, 
    typename Tsplit_mode,
    typename Tnode_alloc = std::allocator<char>
    >
using aabbtree3 = aabbtree<Tfloat, 3, Tsplit_mode, Tnode_alloc>;

/**
 * @brief Split by equal counts.
 */
struct aabbtree_split_equal_counts
{
    /**
     * @brief Split.
     */
    template <
        typename Tfloat, std::size_t N,
        typename Tproxy
        >
    Tproxy* operator()(
            const aabb<Tfloat, N>& box,
            const aabb<Tfloat, N>& box_center,
            const std::size_t split_dim,
            range<Tproxy*> proxies) const
    {
        (void) box;
        (void) box_center;

        Tproxy* split =
                proxies.begin() +
                proxies.size() / 2;

        // Partition.
        std::nth_element(
                proxies.begin(), split,
                proxies.end(),
                [=](const Tproxy& proxy0,
                    const Tproxy& proxy1) -> bool {
                    return proxy0.box_center[split_dim] <
                           proxy1.box_center[split_dim];
                });

        return split;
    }
};

/**
 * @brief Split by equal dimensions.
 */
struct aabbtree_split_equal_dimensions
{
    /**
     * @brief Split.
     */
    template <
        typename Tfloat, std::size_t N,
        typename Tproxy
        >
    Tproxy* operator()(
            const aabb<Tfloat, N>& box,
            const aabb<Tfloat, N>& box_center,
            const std::size_t split_dim,
            range<Tproxy*> proxies) const
    {
        Tfloat cen =
            (box_center[0][split_dim] +
             box_center[1][split_dim]) / 2;

        // Partition.
        Tproxy* split =
            std::partition(
                proxies.begin(),
                proxies.end(),
                [=](const Tproxy& proxy) {
                    return proxy.box_center[split_dim] < cen;
                });

        // Partition successful?
        if (split != proxies.begin() &&
            split != proxies.end()) {
            return split;
        }
        else {
            // Default to equal counts.
            return aabbtree_split_equal_counts()(
                        box,
                        box_center,
                        split_dim,
                        proxies);
        }
    }
};

/**
 * @brief Split by surface area heuristic.
 */
template <std::size_t Nbins = 8>
struct aabbtree_split_surface_area
{
    // Sanity check.
    static_assert(
        Nbins > 1,
        "Nbins must be greater than 1");

    /**
     * @brief Split.
     */
    template <
        typename Tfloat, std::size_t N,
        typename Tproxy
        >
    Tproxy* operator()(
            const aabb<Tfloat, N>& box,
            const aabb<Tfloat, N>& box_center,
            const std::size_t split_dim,
            range<Tproxy*> proxies) const
    {
        // Degenerate?
        if (box_center[0][split_dim] ==
            box_center[1][split_dim]) {
            // Default to equal counts.
            return aabbtree_split_equal_counts()(
                        box,
                        box_center,
                        split_dim,
                        proxies);
        }

        // Bin type.
        typedef std::pair<aabb<Tfloat, N>, std::size_t> bin;

        // Initialize bins.
        std::array<bin, Nbins> bins = {};
        for (const Tproxy& proxy : proxies) {
            Tfloat cen = proxy.box_center[split_dim];
            Tfloat cenmin = box_center[0][split_dim];
            Tfloat cenmax = box_center[1][split_dim];
            std::size_t pos =
            std::min<std::size_t>(
                Nbins * ((cen - cenmin) / (cenmax - cenmin)),
                Nbins - 1);

            bins[pos].first |= proxy.box;
            bins[pos].second++;
        }

        // Initialize sweeps.
        std::array<bin, Nbins - 1> lsweep;
        std::array<bin, Nbins - 1> rsweep; {
            auto itrlsweep = lsweep.begin(), itrlbins = bins.begin();
            auto itrrsweep = rsweep.rbegin(), itrrbins = bins.rbegin();
            *itrlsweep++ = *itrlbins++;
            *itrrsweep++ = *itrrbins++;
            for (; itrlsweep < lsweep.end();
                    ++itrlsweep, ++itrlbins,
                    ++itrrsweep, ++itrrbins) {
                itrlsweep->first = (itrlsweep - 1)->first | itrlbins->first;
                itrrsweep->first = (itrrsweep - 1)->first | itrrbins->first;
                itrlsweep->second = (itrlsweep - 1)->second + itrlbins->second;
                itrrsweep->second = (itrrsweep - 1)->second + itrrbins->second;
            }
        }

        // Compute costs.
        std::array<Tfloat, Nbins - 1> costs; {
            auto itrcosts = costs.begin();
            auto itrlsweep = lsweep.begin();
            auto itrrsweep = rsweep.begin();
            for (; itrrsweep < rsweep.end();
                    ++itrcosts,
                    ++itrlsweep,
                    ++itrrsweep) {
                *itrcosts =
                    (itrlsweep->first.surface_area() * itrlsweep->second +
                     itrrsweep->first.surface_area() * itrrsweep->second);
            }
        }

        // Compute costs argmin.
        std::size_t costs_argmin =
            std::distance(
                costs.begin(),
                std::min_element(
                    costs.begin(),
                    costs.end()));

        // Partition.
        Tproxy* split =
            std::partition(
                proxies.begin(),
                proxies.end(),
                [=](const Tproxy& proxy) {
                    Tfloat cen = proxy.box_center[split_dim];
                    Tfloat cenmin = box_center[0][split_dim];
                    Tfloat cenmax = box_center[1][split_dim];
                    std::size_t pos =
                    std::min<std::size_t>(
                        Nbins * ((cen - cenmin) / (cenmax - cenmin)),
                        Nbins - 1);
                    return pos <= costs_argmin;
                });

        // Partition successful?
        if (split != proxies.begin() &&
            split != proxies.end()) {
            return split;
        }
        else {
            // Default to equal counts.
            return aabbtree_split_equal_counts()(
                        box,
                        box_center,
                        split_dim,
                        proxies);
        }
    }
};

/**
 * @brief Linear axis-aligned bounding box tree.
 */
template <
    typename Tfloat, std::size_t N,
    typename Tnode_alloc = std::allocator<char>
    >
class linear_aabbtree
{
public:

    // Sanity check.
    static_assert(
        std::is_floating_point<Tfloat>::value,
        "Tfloat must be floating point");

    /**
     * @name Container typedefs
     */
    /**@{*/

    /**
     * @brief Size type.
     */
    typedef std::size_t size_type;

    /**
     * @brief Floating point type.
     */
    typedef Tfloat float_type;

#if !DOXYGEN
    // prototype
    struct node_type;
#endif // #if !DOXYGEN

    /**
     * @brief Node allocator type.
     */
    typedef typename std::allocator_traits<Tnode_alloc>::
            template rebind_alloc<node_type> node_allocator_type;

    /**
     * @brief Node allocator traits.
     */
    typedef typename std::allocator_traits<Tnode_alloc>::
            template rebind_traits<node_type> node_allocator_traits;

    /**@}*/

public:

    /**
     * @brief Node type.
     */
    struct node_type
    {
    public:

        /**
         * @brief Box.
         */
        aabb<float_type, N> box;

        union {

            /**
             * @brief If branch, right child index.
             */
            std::uint32_t right_offset;

            /**
             * @brief If leaf, first proxy index.
             */
            std::uint32_t first_index;
        };

        /**
         * @brief Proxy count.
         */
        std::uint8_t count;

        /**
         * @brief If branch, split dimension.
         */
        std::uint8_t split_dim;

    public:

        /**
         * @name Traversal helpers
         */
        /**@{*/

        /**
         * @brief Is branch?
         */
        __attribute__((always_inline))
        bool is_branch() const noexcept
        {
            return count == 0;
        }

        /**
         * @brief Left child.
         *
         * @note
         * This is only valid for branch nodes.
         *
         * @note
         * This function assumes that `this` is a valid pointer 
         * to a node inside the parent node array. Hence, calling this 
         * on a copy is ill-defined.
         */
        __attribute__((always_inline))
        const node_type* left_child() const noexcept
        {
            assert(count == 0);
            return this + 1;
        }

        /**
         * @brief Right child.
         *
         * @note
         * This is only valid for branch nodes.
         *
         * @note
         * This function assumes that `this` is a valid pointer 
         * to a node inside the parent node array. Hence, calling this
         * on a copy is ill-defined.
         */
        __attribute__((always_inline))
        const node_type* right_child() const noexcept
        {
            assert(count == 0);
            return this + right_offset;
        }

        /**@}*/
    };

public:

    /**
     * @name Constructors
     */
    /**@{*/

    /**
     * @brief Default constructor.
     */
    linear_aabbtree() = default;

    /**
     * @brief Constructor.
     */
    template <typename... Tother>
    linear_aabbtree(const aabbtree<Tfloat, N, Tother...>& tree)
    {
        // Reserve memory.
        nodes_.reserve(
            tree.total_branches_ + 
            tree.total_leaves_);

        // Initialize.
        if (tree.root_) {
            init_recursive(tree.root_);
            assert(nodes_.size() ==
                   tree.total_branches_ + 
                   tree.total_leaves_);
        }
    }

    /**@}*/

public:

    /**
     * @name Container interface
     */
    /**@{*/

    /**
     * @brief Empty?
     */
    __attribute__((always_inline))
    bool empty() const noexcept
    {
        return nodes_.empty();
    }

    /**
     * @brief Size.
     */
    __attribute__((always_inline))
    size_type size() const noexcept
    {
        return nodes_.size();
    }

    /**
     * @brief Begin iterator.
     *
     * @note
     * If `nodes_` is empty, returns nullptr.
     */
    __attribute__((always_inline))
    const node_type* begin() const noexcept
    {
        if (nodes_.empty()) {
            return nullptr;
        }
        else {
            return &nodes_[0];
        }
    }

    /**
     * @brief End iterator.
     *
     * @note
     * If `nodes_` is empty, returns nullptr.
     */
    __attribute__((always_inline))
    const node_type* end() const noexcept
    {
        if (nodes_.empty()) {
            return nullptr;
        }
        else {
            return &nodes_[0] + nodes_.size();
        }
    }

    /**
     * @brief Index accessor.
     */
    __attribute__((always_inline))
    const node_type& operator[](size_type pos) const noexcept
    {
        return nodes_[pos];
    }

    /**@}*/

private:

    /**
     * @brief Nodes.
     */
    std::vector<
            node_type,
            node_allocator_type> nodes_;

private:

    /**
     * @brief Flatten.
     */
    template <typename Ttree_node>
    void init_recursive(Ttree_node* tree_node)
    {
        // Sanity check.
        assert(tree_node);

        // Next node.
        nodes_.emplace_back();
        size_type node_index = nodes_.size() - 1; // Remember index.
        node_type& node = nodes_.back();
        node.box = tree_node->box;

        if (tree_node->count) {
            // Sanity check.
            assert(
                !tree_node->left &&
                !tree_node->right);
            assert(
                tree_node->count <
                size_type(256));

            // Initialize.
            node.first_index = tree_node->first_index;
            node.count = tree_node->count;
        }
        else {
            // Sanity check.
            assert(
                tree_node->left &&
                tree_node->right);

            // Initialize left branch.
            init_recursive(tree_node->left);

            // Initialize.
            node.right_offset = nodes_.size() - node_index;
            node.split_dim = tree_node->split_dim;

            // Initialize right branch.
            init_recursive(tree_node->right);
        }
    }
};

/**
 * @brief Template alias for convenience.
 */
template <
    typename Tfloat, 
    typename Tnode_alloc = std::allocator<char>
    >
using linear_aabbtree2 = pr::linear_aabbtree<Tfloat, 2, Tnode_alloc>;

/**
 * @brief Template alias for convenience.
 */
template <
    typename Tfloat, 
    typename Tnode_alloc = std::allocator<char>
    >
using linear_aabbtree3 = pr::linear_aabbtree<Tfloat, 3, Tnode_alloc>;

/**@}*/

} // namespace pr

#endif // #ifndef PREFORM_AABBTREE_HPP

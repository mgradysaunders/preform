/* Copyright (c) 2018-20 M. Grady Saunders
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
#if !(__cplusplus >= 201402L)
#error "preform/static_queue.hpp requires >=C++14"
#endif // #if !(__cplusplus >= 201402L)
#endif // #if !DOXYGEN
#pragma once
#ifndef PREFORM_STATIC_QUEUE_HPP
#define PREFORM_STATIC_QUEUE_HPP

// for std::size_t, std::ptrdiff_t
#include <cstddef>

// for std::reverse_iterator
#include <iterator>

// for std::runtime_error, std::length_error, ...
#include <stdexcept>

// for std::is_default_constructible, std::is_copy_assignable, ...
#include <type_traits>

// for std::move
#include <utility>

namespace pre {

/**
 * @defgroup static_queue Static queue
 *
 * `<preform/static_queue.hpp>`
 *
 * __C++ version__: >=C++14
 */
/**@{*/

/**
 * @brief Static queue.
 *
 * Static queue (first-in first-out), where elements are
 * pushed onto the top/back and popped from the bottom/front.
 *
 * @note
 * The implementation minimizes move operations by tracking
 * the effective bottom and top of the queue. That is, popping
 * shifts the bottom up instead of moving the elements
 * down. When pushing shifts the top past the queue capacity, the
 * implementation moves all elements down and resets the bottom
 * to zero.
 *
 * @tparam T
 * Queue value type, which must be
 * - default constructible,
 * - copy assignable, and
 * - destructible.
 *
 * @tparam N
 * Queue capacity, which must be positive.
 */
template <typename T, std::size_t N>
class static_queue
{
public:

    // Sanity check.
    static_assert(N > 0, "N must be positive");

    // Sanity check.
    static_assert(
        std::is_default_constructible<T>::value &&
        std::is_copy_assignable<T>::value &&
        std::is_destructible<T>::value,
        "T must be default constructible, copy assignable, and "
        "destructible.");

    /**
     * @name Container typedefs
     */
    /**@{*/

    /**
     * @brief Size type.
     */
    typedef std::size_t size_type;

    /**
     * @brief Difference type.
     */
    typedef std::ptrdiff_t difference_type;

    /**
     * @brief Value type.
     */
    typedef T value_type;

    /**
     * @brief Reference.
     */
    typedef value_type& reference;

    /**
     * @brief Pointer.
     */
    typedef value_type* pointer;

    /**
     * @brief Const reference.
     */
    typedef const value_type& const_reference;

    /**
     * @brief Const pointer.
     */
    typedef const value_type* const_pointer;

    /**
     * @brief Iterator.
     */
    typedef pointer iterator;

    /**
     * @brief Const iterator.
     */
    typedef const_pointer const_iterator;

    /**
     * @brief Reverse iterator.
     */
    typedef std::reverse_iterator<iterator> reverse_iterator;

    /**
     * @brief Const reverse iterator.
     */
    typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

    /**@}*/

public:

    /**
     * @brief Default constructor
     */
    constexpr static_queue() = default;

public:

    /**
     * @name Container interface
     */
    /**@{*/

    /**
     * @brief Size.
     */
    constexpr size_type size() const noexcept
    {
        return top_ - bot_;
    }

    /**
     * @brief Capacity.
     */
    constexpr size_type capacity() const noexcept
    {
        return N;
    }

    /**
     * @brief Clear.
     */
    constexpr void clear() noexcept
    {
        // Destroy.
        for (reference val : *this) {
            val.~value_type();
        }
        // Reset.
        bot_ = 0;
        top_ = 0;
    }

    /**
     * @brief Empty?
     */
    constexpr bool empty() const noexcept
    {
        return bot_ == top_;
    }

    /**
     * @brief Full?
     */
    constexpr bool full() const noexcept
    {
        return bot_ == 0 && top_ == N;
    }

    /**
     * @brief Begin iterator.
     */
    constexpr iterator begin() noexcept
    {
        return &arr_[0] + bot_;
    }

    /**
     * @brief Begin iterator, const variant.
     */
    constexpr const_iterator begin() const noexcept
    {
        return &arr_[0] + bot_;
    }

    /**
     * @brief Begin iterator, force const variant.
     */
    constexpr const_iterator cbegin() const noexcept
    {
        return begin(); // Force const
    }

    /**
     * @brief End iterator.
     */
    constexpr iterator end() noexcept
    {
        return &arr_[0] + top_;
    }

    /**
     * @brief End iterator, const variant.
     */
    constexpr const_iterator end() const noexcept
    {
        return &arr_[0] + top_;
    }

    /**
     * @brief End iterator, force const variant.
     */
    constexpr const_iterator cend() const noexcept
    {
        return end(); // Force const
    }

    /**
     * @brief Reverse begin iterator.
     */
    constexpr reverse_iterator rbegin() noexcept
    {
        return reverse_iterator(end());
    }

    /**
     * @brief Reverse begin iterator, const variant.
     */
    constexpr const_reverse_iterator rbegin() const noexcept
    {
        return const_reverse_iterator(end());
    }

    /**
     * @brief Reverse begin iterator, force const variant.
     */
    constexpr const_reverse_iterator crbegin() const noexcept
    {
        return rbegin(); // Force const
    }

    /**
     * @brief Reverse end iterator.
     */
    constexpr reverse_iterator rend() noexcept
    {
        return reverse_iterator(begin());
    }

    /**
     * @brief Reverse end iterator, const variant.
     */
    constexpr const_reverse_iterator rend() const noexcept
    {
        return reverse_iterator(begin());
    }

    /**
     * @brief Reverse end iterator, force const variant.
     */
    constexpr const_reverse_iterator crend() const noexcept
    {
        return rend(); // Force const
    }

    /**
     * @brief Bottom/front value.
     */
    constexpr reference front() noexcept
    {
        return *begin();
    }

    /**
     * @brief Bottom/front value, const variant.
     */
    constexpr const_reference front() const noexcept
    {
        return *begin();
    }

    /**
     * @brief Top/back value.
     */
    constexpr reference back() noexcept
    {
        return *rbegin();
    }

    /**
     * @brief Top/back value, const variant.
     */
    constexpr const_reference back() const noexcept
    {
        return *rbegin();
    }

    /**
     * @brief Bottom/front value with empty check.
     *
     * @throw std::runtime_error
     * If empty.
     */
    constexpr reference bottom()
    {
        if (empty()) {
            throw std::runtime_error(__PRETTY_FUNCTION__);
        }
        return front();
    }

    /**
     * @brief Bottom/front value with empty check, const variant.
     *
     * @throw std::runtime_error
     * If empty.
     */
    constexpr const_reference bottom() const
    {
        if (empty()) {
            throw std::runtime_error(__PRETTY_FUNCTION__);
        }
        return front();
    }

    /**
     * @brief Top/back value with empty check.
     *
     * @throw std::runtime_error
     * If empty.
     */
    constexpr reference top()
    {
        if (empty()) {
            throw std::runtime_error(__PRETTY_FUNCTION__);
        }
        return back();
    }

    /**
     * @brief Top/back value with empty check, const variant.
     *
     * @throw std::runtime_error
     * If empty.
     */
    constexpr const_reference top() const
    {
        if (empty()) {
            throw std::runtime_error(__PRETTY_FUNCTION__);
        }
        return back();
    }

    /**@}*/

public:

    /**
     * @name Queue interface
     */
    /**@{*/

    /**
     * @brief Pop and return bottom/front value.
     *
     * @throw std::runtime_error
     * If empty.
     */
    constexpr value_type pop()
    {
        // Empty?
        if (bot_ == top_) {
            throw std::runtime_error(__PRETTY_FUNCTION__);
        }

        // Pop.
        value_type& val = arr_[bot_++];
        value_type res = std::move(val);
        val.~value_type();

        // Empty now?
        if (bot_ == top_) {
            // Clear.
            bot_ = 0;
            top_ = 0;
        }
        return res;
    }

    /**
     * @brief Push top/back value.
     *
     * @throw std::length_error
     * If full.
     */
    constexpr void push(const_reference val)
    {
        // Possibly full?
        if (top_ >= N) {

            // Full?
            if (bot_ == 0) {
                throw std::length_error(__PRETTY_FUNCTION__);
            }

            // Shift.
            iterator itr0 = &arr_[0];
            iterator itr1 = &arr_[0] + bot_;
            for (; itr1 < &arr_[0] + top_; ++itr0, ++itr1) {
                *itr0 = std::move(*itr1);
                itr1->~value_type();
            }
            top_ -= bot_;
            bot_ = 0;
        }

        // Push.
        arr_[top_++] = val;
    }

    /**@}*/

public:

    /**
     * @name Access operators
     */
    /**@{*/

    /**
     * @brief Index accessor.
     *
     * @note
     * To access from the top down, use negative indices.
     * That is, `queue[-1]` is the top value, `queue[-2]` is the
     * next value down, and so on.
     */
    constexpr reference operator[](difference_type n) noexcept
    {
        if (n < 0) {
            n += top_ - bot_;
        }
        n += bot_;
        return arr_[n];
    }

    /**
     * @brief Index accessor, const variant.
     *
     * @note
     * To access from the top down, use negative indices.
     * That is, `queue[-1]` is the top value, `queue[-2]` is the
     * next value down, and so on.
     */
    constexpr const_reference operator[](difference_type n) const noexcept
    {
        if (n < 0) {
            n += top_ - bot_;
        }
        n += bot_;
        return arr_[n];
    }

    /**
     * @brief Index access with range check.
     *
     * @note
     * To access from the top down, use negative indices.
     * That is, `queue[-1]` is the top value, `queue[-2]` is the
     * next value down, and so on.
     *
     * @throw std::out_of_range
     * If index is out of range.
     */
    constexpr reference at(difference_type n)
    {
        if (n < 0) {
            n += top_ - bot_;
        }
        n += bot_;
        if (n < difference_type(bot_) || n >= difference_type(top_)) {
            throw std::out_of_range(__PRETTY_FUNCTION__);
        }
        return arr_[n];
    }

    /**
     * @brief Index access with range check, const variant.
     *
     * @note
     * To access from the top down, use negative indices.
     * That is, `queue[-1]` is the top value, `queue[-2]` is the
     * next value down, and so on.
     *
     * @throw std::out_of_range
     * If index is out of range.
     */
    constexpr const_reference at(difference_type n) const
    {
        if (n < 0) {
            n += top_ - bot_;
        }
        n += bot_;
        if (n < difference_type(bot_) || n >= difference_type(top_)) {
            throw std::out_of_range(__PRETTY_FUNCTION__);
        }
        return arr_[n];
    }

    /**@}*/

private:

    /**
     * @brief Values.
     */
    value_type arr_[N] = {};

    /**
     * @brief Bottom.
     */
    size_type bot_ = 0;

    /**
     * @brief Top.
     */
    size_type top_ = 0;
};

/**@}*/

} // namespace pre

#endif // #ifndef PREFORM_STATIC_QUEUE_HPP

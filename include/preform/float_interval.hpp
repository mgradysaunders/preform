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
#if !(__cplusplus >= 201703L)
#error "preform/float_interval.hpp requires >=C++17"
#endif // #if !(__cplusplus >= 201703L)
#endif // #if !DOXYGEN
#pragma once
#ifndef PREFORM_FLOAT_INTERVAL_HPP
#define PREFORM_FLOAT_INTERVAL_HPP

// for assert
#include <cassert>

// for std::basic_istream
#include <istream>

// for std::basic_ostream
#include <ostream>

#if PREFORM_USE_FENV
#pragma STDC FENV_ACCESS ON
#include <cfenv>
#endif // #if PREFORM_USE_FENV

// for pre::numeric_limits
#include <preform/math.hpp>

// for pre::finc, pre::fdec
#include <preform/misc_float.hpp>

namespace pre {

/**
 * @defgroup float_interval Float interval
 *
 * `<preform/float_interval.hpp>`
 *
 * __C++ version__: >=C++17
 *
 * @note
 * To use `std::fesetround` instead of `pre::fdec` and `pre::finc` where
 * sensible, `#define PREFORM_USE_FENV 1` before including. This ought to
 * yield tighter bounds, though the code may not run any faster.
 *
 * @note
 * If `PREFORM_USE_FENV` is truthy, this header will
 * `#pragma STDC FENV_ACCESS ON` to be standard compliant. GCC and clang
 * may warn that this pragma is unknown&mdash;this is not an issue, as GCC
 * and clang (on x86-64 at least) provide floating-point environment access
 * by default.
 */
/**@{*/

/**
 * @brief Float bounds.
 *
 * @tparam T
 * Float type.
 */
template <typename T>
class float_interval
{
public:

    // Sanity check.
    static_assert(
        std::is_floating_point<T>::value,
        "T must be floating point");

    /**
     * @name Constructors
     */
    /**@{*/

    /**
     * @brief Default constructor.
     */
    float_interval() = default;

    /**
     * @brief Constructor.
     *
     * @param[in] x
     * Value.
     */
    float_interval(T x) :
            x_(x),
            x0_(x),
            x1_(x)
    {
    }

    /**
     * @brief Constructor.
     *
     * @param[in] x
     * Value.
     *
     * @param[in] x0
     * Value lower bound.
     *
     * @param[in] x1
     * Value upper bound.
     */
    float_interval(T x, T x0, T x1) :
            x_(x),
            x0_(x0),
            x1_(x1)
    {
#if !NDEBUG
        if (pre::isfinite(x_) &&
            pre::isfinite(x0_) &&
            pre::isfinite(x1_)) {
            assert(x0_ <= x &&
                   x1_ >= x);
        }
#endif // #if !NDEBUG
    }

    /**
     * @brief Constructor.
     *
     * @param[in] x
     * Value.
     *
     * @param[in] xerr
     * Value absolute error.
     */
    float_interval(T x, T xerr) : x_(x)
    {
#if PREFORM_USE_FENV
        int mode = std::fegetround();
        std::fesetround(FE_DOWNWARD);
        x0_ = x - xerr;
        std::fesetround(FE_UPWARD);
        x1_ = x + xerr;
        std::fesetround(mode);
#else
        x0_ = pre::fdec(x - xerr);
        x1_ = pre::finc(x + xerr);
#endif // #if PREFORM_USE_FENV
    }

    /**@}*/

public:

    /**
     * @brief Value.
     */
    __attribute__((always_inline))
    T value() const
    {
        return x_;
    }

    /**
     * @brief Lower bound.
     */
    __attribute__((always_inline))
    T lower_bound() const
    {
        return x0_;
    }

    /**
     * @brief Upper bound.
     */
    __attribute__((always_inline))
    T upper_bound() const
    {
        return x1_;
    }

    /**
     * @brief Lower bound of absolute interval.
     */
    __attribute__((always_inline))
    T abs_lower_bound() const
    {
        return pre::signbit(x0_) == 0 ? +x0_ :
               pre::signbit(x1_) == 1 ? -x1_ : 0;
    //  return pre::fabs(pre::signbit(x_) ? x1_ : x0_);
    }

    /**
     * @brief Upper bound of absolute interval.
     */
    __attribute__((always_inline))
    T abs_upper_bound() const
    {
        return pre::signbit(x0_) == 0 ? +x1_ :
               pre::signbit(x1_) == 1 ? -x0_ : pre::fmax(-x0_, x1_);
    //  return pre::fabs(pre::signbit(x_) ? x0_ : x1_);
    }

    /**
     * @brief Absolute error.
     */
    __attribute__((always_inline))
    T abs_error() const
    {
#if PREFORM_USE_FENV
        int mode = std::fegetround();
        std::fesetround(FE_UPWARD);
        T dx0 = x_ - x0_;
        T dx1 = x1_ - x_;
        std::fesetround(mode);
        return pre::max(dx0, dx1);
#else
        return pre::finc(pre::fmax(x_ - x0_, x1_ - x_));
#endif // #if PREFORM_USE_FENV
    }

    /**
     * @brief Relative error.
     */
    __attribute__((always_inline))
    T rel_error() const
    {
        return abs_error() / value();
    }

public:

    /**
     * @name Tests
     */
    /**@{*/

    /**
     * @brief Overlaps other?
     */
    template <
        bool inclusive0 = true,
        bool inclusive1 = false
        >
    __attribute__((always_inline))
    bool overlaps(const float_interval& oth) const
    {
        if constexpr (inclusive0 && inclusive1) {
            // Both inclusive.
            return lower_bound() <= oth.upper_bound() &&
                   upper_bound() >= oth.lower_bound();
        }
        else if constexpr (inclusive0 && !inclusive1) {
            // First inclusive, second exclusive.
            return lower_bound() <= oth.upper_bound() &&
                   upper_bound() > oth.lower_bound();
        }
        else if constexpr (!inclusive0 && inclusive1) {
            // First exclusive, second inclusive.
            return lower_bound() < oth.upper_bound() &&
                   upper_bound() >= oth.lower_bound();
        }
        else {
            // Both exclusive.
            return lower_bound() < oth.upper_bound() &&
                   upper_bound() > oth.lower_bound();
        }
    }

    /**
     * @brief Contains other?
     */
    template <
        bool inclusive0 = true,
        bool inclusive1 = false
        >
    __attribute__((always_inline))
    bool contains(const float_interval& oth) const
    {
        if constexpr (inclusive0 && inclusive1) {
            // Both inclusive.
            return lower_bound() <= oth.lower_bound() &&
                   upper_bound() >= oth.upper_bound();
        }
        else if constexpr (inclusive0 && !inclusive1) {
            // First inclusive, second exclusive.
            return lower_bound() <= oth.lower_bound() &&
                   upper_bound() > oth.upper_bound();
        }
        else if constexpr (!inclusive0 && inclusive1) {
            // First exclusive, second inclusive.
            return lower_bound() < oth.lower_bound() &&
                   upper_bound() >= oth.upper_bound();
        }
        else {
            // Both exclusive.
            return lower_bound() < oth.lower_bound() &&
                   upper_bound() > oth.upper_bound();
        }
    }

    /**@}*/

private:

    /**
     * @brief Value @f$ x @f$.
     */
    T x_ = T();

    /**
     * @brief Value lower bound @f$ x_0 @f$.
     */
    T x0_ = T();

    /**
     * @brief Value upper bound @f$ x_1 @f$.
     */
    T x1_ = T();

public:

    /**
     * @name Stream operators
     */
    /**@{*/

    /**
     * @brief Parse from `std::basic_istream`.
     *
     * Format is `(x,[x0,x1])` or `x`. Sets `std::ios_base::failbit` on error.
     */
    template <typename C, typename Ctraits>
    friend
    inline std::basic_istream<C, Ctraits>& operator>>(
           std::basic_istream<C, Ctraits>& is, float_interval& b)
    {
        C ch;
        if (!(is >> ch) ||
            !Ctraits::eq(ch,
             Ctraits::to_char_type('('))) {
            T x;
            if (!(is >> x)) {
                is.setstate(std::ios_base::failbit);
                return is;
            }
            b = float_interval(x);
            return is;
        }
        T x;
        is >> x;
        if (!(is >> ch) ||
            !Ctraits::eq(ch,
             Ctraits::to_char_type(','))) {
            is.setstate(std::ios_base::failbit);
            return is;
        }
        if (!(is >> ch) ||
            !Ctraits::eq(ch,
             Ctraits::to_char_type('['))) {
            is.setstate(std::ios_base::failbit);
            return is;
        }
        T x0;
        is >> x0;
        if (!(is >> ch) ||
            !Ctraits::eq(ch,
             Ctraits::to_char_type(','))) {
            is.setstate(std::ios_base::failbit);
            return is;
        }
        T x1;
        is >> x1;
        if (!(is >> ch) ||
            !Ctraits::eq(ch,
             Ctraits::to_char_type(']'))) {
            is.setstate(std::ios_base::failbit);
            return is;
        }
        if (!(is >> ch) ||
            !Ctraits::eq(ch,
             Ctraits::to_char_type(')'))) {
            is.setstate(std::ios_base::failbit);
            return is;
        }
        b = float_interval(x, x0, x1);
        return is;
    }

    /**
     * @brief Write into `std::basic_ostream`.
     *
     * Format is `(x,[x0,x1])`.
     */
    template <typename C, typename Ctraits>
    friend
    inline std::basic_ostream<C, Ctraits>& operator<<(
           std::basic_ostream<C, Ctraits>& os, const float_interval& b)
    {
        os << '(' << b.x_ << ',';
        os << '[' << b.x0_ << ',' << b.x1_ << ']';
        os << ')';
        return os;
    }

    /**@}*/

public:

    /**
     * @brief Solve for roots of degree-1 polynomial.
     *
     * @f[
     *      a_0 + a_1 t = 0
     * @f]
     *
     * @param[in] a0
     * Polynomial coefficient.
     *
     * @param[in] a1
     * Polynomial coefficient.
     *
     * @param[out] t0
     * Root.
     */
    static void solve_poly1(
                const float_interval& a0,
                const float_interval& a1,
                float_interval& t0)
    {
        t0 = -a0 / a1;
    }

    /**
     * @brief Solve for roots of degree-2 polynomial.
     *
     * @f[
     *      a_0 + a_1 t + a_2 t^2 = 0
     * @f]
     *
     * @param[in] a0
     * Polynomial coefficient.
     *
     * @param[in] a1
     * Polynomial coefficient.
     *
     * @param[in] a2
     * Polynomial coefficient.
     *
     * @param[out] t0
     * Solution.
     *
     * @param[out] t1
     * Solution.
     *
     * @post
     * - `!(t1.value() > t0.value())`
     */
    static void solve_poly2(
                const float_interval& a0,
                const float_interval& a1,
                const float_interval& a2,
                float_interval& t0,
                float_interval& t1);
};

/**
 * @name Unary operators (float_interval)
 */
/**@{*/

/**
 * @brief Bound `operator+`.
 *
 * @f[
 *      +[b_0, b_1] \subseteq [+b_0, +b_1]
 * @f]
 */
template <typename T>
__attribute__((always_inline))
inline float_interval<T> operator+(const float_interval<T>& b)
{
    return b;
}

/**
 * @brief Bound `operator-`.
 *
 * @f[
 *      -[b_0, b_1] \subseteq [-b_1, -b_0]
 * @f]
 */
template <typename T>
__attribute__((always_inline))
inline float_interval<T> operator-(const float_interval<T>& b)
{
    return {
        -b.value(),
        -b.upper_bound(),
        -b.lower_bound()
    };
}

/**@}*/

/**
 * @name Binary operators (float_interval/float_interval)
 */
/**@{*/

/**
 * @brief Bound `operator+`.
 *
 * @f[
 *      [b_{00}, b_{01}] \oplus
 *      [b_{10}, b_{11}] \subset
 *      [\operatorname{fdec}(b_{00} \oplus b_{10}),
 *       \operatorname{finc}(b_{01} \oplus b_{11})]
 * @f]
 *
 * @note
 * If `PREFORM_USE_FENV` is truthy, the implementation uses `std::fesetround`
 * instead of `pre::fdec` and `pre::finc`.
 */
template <typename T>
__attribute__((always_inline))
inline float_interval<T> operator+(
                const float_interval<T>& b0,
                const float_interval<T>& b1)
{
#if PREFORM_USE_FENV
    T x = b0.value() + b1.value();
    T x0;
    T x1;
    int mode = std::fegetround();
    std::fesetround(FE_DOWNWARD);
    x0 = b0.lower_bound() + b1.lower_bound();
    std::fesetround(FE_UPWARD);
    x1 = b0.upper_bound() + b1.upper_bound();
    std::fesetround(mode);
    return {x, x0, x1};
#else
    return {
        b0.value() + b1.value(),
        pre::fdec(b0.lower_bound() + b1.lower_bound()),
        pre::finc(b0.upper_bound() + b1.upper_bound())
    };
#endif // #if PREFORM_USE_FENV
}

/**
 * @brief Bound `operator-`.
 *
 * @f[
 *      [b_{00}, b_{01}] \ominus
 *      [b_{10}, b_{11}] \subset
 *      [\operatorname{fdec}(b_{00} \ominus b_{11}),
 *       \operatorname{finc}(b_{01} \ominus b_{10})]
 * @f]
 *
 * @note
 * If `PREFORM_USE_FENV` is truthy, the implementation uses `std::fesetround`
 * instead of `pre::fdec` and `pre::finc`.
 */
template <typename T>
__attribute__((always_inline))
inline float_interval<T> operator-(
                const float_interval<T>& b0,
                const float_interval<T>& b1)
{
#if PREFORM_USE_FENV
    T x = b0.value() - b1.value();
    T x0;
    T x1;
    int mode = std::fegetround();
    std::fesetround(FE_DOWNWARD);
    x0 = b0.lower_bound() - b1.upper_bound();
    std::fesetround(FE_UPWARD);
    x1 = b0.upper_bound() - b1.lower_bound();
    std::fesetround(mode);
    return {x, x0, x1};
#else
    return {
        b0.value() - b1.value(),
        pre::fdec(b0.lower_bound() - b1.upper_bound()),
        pre::finc(b0.upper_bound() - b1.lower_bound())
    };
#endif // #if PREFORM_USE_FENV
}

/**
 * @brief Bound `operator*`.
 *
 * @f[
 *      [b_{00}, b_{01}] \otimes
 *      [b_{10}, b_{11}] \subset
 *      [\operatorname{fdec}(\min\{t_{jk}\}),
 *       \operatorname{finc}(\max\{t_{jk}\})]
 * @f]
 * where
 * @f[
 *      t_{jk} = b_{0j} \otimes b_{1k}
 * @f]
 */
template <typename T>
inline float_interval<T> operator*(
                const float_interval<T>& b0,
                const float_interval<T>& b1)
{
    T tmp[] = {
        b0.lower_bound() * b1.lower_bound(),
        b0.lower_bound() * b1.upper_bound(),
        b0.upper_bound() * b1.lower_bound(),
        b0.upper_bound() * b1.upper_bound()
    };
    return {
        b0.value() * b1.value(),
        pre::fdec(pre::fmin(pre::fmin(tmp[0], tmp[1]), pre::fmin(tmp[2], tmp[3]))),
        pre::finc(pre::fmax(pre::fmax(tmp[0], tmp[1]), pre::fmax(tmp[2], tmp[3])))
    };
}

/**
 * @brief Bound `operator/`.
 *
 * @f[
 *      [b_{00}, b_{01}] \oslash
 *      [b_{10}, b_{11}] \subset
 *      [\operatorname{fdec}(\min\{t_{jk}\}),
 *       \operatorname{finc}(\max\{t_{jk}\})]
 * @f]
 * where
 * @f[
 *      t_{jk} = b_{0j} \oslash b_{1k}
 * @f]
 */
template <typename T>
inline float_interval<T> operator/(
                const float_interval<T>& b0,
                const float_interval<T>& b1)
{
    // Denominator contains zero?
    if (b1.lower_bound() <= T(0) &&
        b1.upper_bound() >= T(0)) {
        return {
            b0.value() / b1.value(),
            -pre::numeric_limits<T>::infinity(),
            +pre::numeric_limits<T>::infinity()
        };
    }

    T tmp[] = {
        b0.lower_bound() / b1.lower_bound(),
        b0.lower_bound() / b1.upper_bound(),
        b0.upper_bound() / b1.lower_bound(),
        b0.upper_bound() / b1.upper_bound()
    };
    return {
        b0.value() / b1.value(),
        pre::fdec(pre::fmin(pre::fmin(tmp[0], tmp[1]), pre::fmin(tmp[2], tmp[3]))),
        pre::finc(pre::fmax(pre::fmax(tmp[0], tmp[1]), pre::fmax(tmp[2], tmp[3])))
    };
}

/**@}*/

/**
 * @name Binary operators (float_interval/float)
 */
/**@{*/

/**
 * @brief Wrap `operator+`.
 */
template <typename T>
__attribute__((always_inline))
inline float_interval<T> operator+(const float_interval<T>& b0, T b1)
{
    return b0 + float_interval<T>(b1);
}

/**
 * @brief Wrap `operator-`.
 */
template <typename T>
__attribute__((always_inline))
inline float_interval<T> operator-(const float_interval<T>& b0, T b1)
{
    return b0 - float_interval<T>(b1);
}

/**
 * @brief Wrap `operator*`.
 */
template <typename T>
__attribute__((always_inline))
inline float_interval<T> operator*(const float_interval<T>& b0, T b1)
{
    return b0 * float_interval<T>(b1);
}

/**
 * @brief Wrap `operator/`.
 */
template <typename T>
__attribute__((always_inline))
inline float_interval<T> operator/(const float_interval<T>& b0, T b1)
{
    return b0 / float_interval<T>(b1);
}

/**@}*/

/**
 * @name Binary operators (float/float_interval)
 */
/**@{*/

/**
 * @brief Wrap `operator+`.
 */
template <typename T>
__attribute__((always_inline))
inline float_interval<T> operator+(T b0, const float_interval<T>& b1)
{
    return float_interval<T>(b0) + b1;
}

/**
 * @brief Wrap `operator-`.
 */
template <typename T>
__attribute__((always_inline))
inline float_interval<T> operator-(T b0, const float_interval<T>& b1)
{
    return float_interval<T>(b0) - b1;
}

/**
 * @brief Wrap `operator*`.
 */
template <typename T>
__attribute__((always_inline))
inline float_interval<T> operator*(T b0, const float_interval<T>& b1)
{
    return float_interval<T>(b0) * b1;
}

/**
 * @brief Wrap `operator/`.
 */
template <typename T>
__attribute__((always_inline))
inline float_interval<T> operator/(T b0, const float_interval<T>& b1)
{
    return float_interval<T>(b0) / b1;
}

/**@}*/

/**
 * @name Binary operators (float_interval/any)
 */
/**@{*/

/**
 * @brief Generic `operator+=`.
 */
template <typename T, typename U>
__attribute__((always_inline))
inline float_interval<T>& operator+=(float_interval<T>& b, const U& any)
{
    return b = b + any;
}

/**
 * @brief Generic `operator-=`.
 */
template <typename T, typename U>
__attribute__((always_inline))
inline float_interval<T>& operator-=(float_interval<T>& b, const U& any)
{
    return b = b - any;
}

/**
 * @brief Generic `operator*=`.
 */
template <typename T, typename U>
__attribute__((always_inline))
inline float_interval<T>& operator*=(float_interval<T>& b, const U& any)
{
    return b = b * any;
}

/**
 * @brief Generic `operator/=`.
 */
template <typename T, typename U>
__attribute__((always_inline))
inline float_interval<T>& operator/=(float_interval<T>& b, const U& any)
{
    return b = b / any;
}

/**@}*/

/**
 * @name Math (float_interval)
 */
/**@{*/

/**
 * @brief Bound `pre::fabs()`.
 */
template <typename T>
inline float_interval<T> fabs(const float_interval<T>& b)
{
    if (pre::signbit(b.lower_bound()) == 0) { // Entire interval positive?
        return +b;
    }
    if (pre::signbit(b.upper_bound()) == 1) { // Entire interval negative?
        return -b;
    }
    return {
        pre::fabs(b.value()),
        T(0),
        pre::fmax(
                -b.lower_bound(),
                +b.upper_bound())
    };
}

/**
 * @brief Bound `pre::sqrt()`.
 *
 * @note
 * If `PREFORM_USE_FENV` is truthy, the implementation uses `std::fesetround`
 * instead of `pre::fdec` and `pre::finc`.
 */
template <typename T>
inline float_interval<T> sqrt(const float_interval<T>& b)
{
#if PREFORM_USE_FENV
    T x = pre::sqrt(b.value());
    T x0;
    T x1;
    int mode = std::fegetround();
    std::fesetround(FE_DOWNWARD);
    x0 = pre::sqrt(b.lower_bound());
    std::fesetround(FE_UPWARD);
    x1 = pre::sqrt(b.upper_bound());
    std::fesetround(mode);
    return {x, x0, x1};
#else
    return {
        pre::sqrt(b.value()),
        pre::fdec(pre::sqrt(b.lower_bound())),
        pre::finc(pre::sqrt(b.upper_bound()))
    };
#endif // #if PREFORM_USE_FENV
}

/**@}*/

#if !DOXYGEN

template <typename T>
inline void float_interval<T>::solve_poly2(
            const float_interval& a0,
            const float_interval& a1,
            const float_interval& a2,
            float_interval& t0,
            float_interval& t1)
{
    if (a2.contains<true, true>(T(0)) ||
        a2.abs_upper_bound() <
        a1.abs_lower_bound() *
            pre::numeric_limits<T>::min_invertible() ||
        a2.abs_upper_bound() <
        a0.abs_lower_bound() *
            pre::numeric_limits<T>::min_invertible()) {
        // Solve linear.
        t0 = -a0 / a1;
        t1 = float_interval(pre::numeric_limits<T>::quiet_NaN());
        return;
    }

    // Normalize.
    float_interval c0 = a0 / a2;
    float_interval c1 = a1 / a2;

    // Discriminant.
    float_interval d = c1 * c1 - T(4) * c0;

    // Is discriminant negative?
    if (d.upper_bound() < T(0)) {
        t0 = float_interval(pre::numeric_limits<T>::quiet_NaN());
        t1 = float_interval(pre::numeric_limits<T>::quiet_NaN());
        return;
    }

    // Is discriminant zero?
    if (d.contains<true, true>(T(0))) {
        t0 = T(-0.5) * c1;
    }
    else {
        float_interval sqrt_d = pre::sqrt(d);
        if (c1.lower_bound() < T(0)) {
            t0 = T(-0.5) * (c1 - sqrt_d);
        }
        else {
            t0 = T(-0.5) * (c1 + sqrt_d);
        }
    }

    // Remaining root.
    t1 = c0 / t0;

    // Sort.
    if (t1.value() <
        t0.value()) {
        std::swap(t0, t1);
    }
}

#endif // #if !DOXYGEN

/**@}*/

} // namespace pre

#endif // #ifndef PREFORM_FLOAT_INTERVAL_HPP

/* Copyright (c) 2018 M. Grady Saunders
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
#error "prefabs/quadmath.hpp requires >=C++14"
#endif // #if !(__cplusplus >= 201402L)
#if !(__GNUC__ && !__clang__)
#error "prefabs/quadmath.hpp requires GCC"
#endif // #if !(__GNUC__ && !__clang__)
#endif // #if !DOXYGEN
#pragma once
#ifndef PREFABS_QUADMATH_HPP
#define PREFABS_QUADMATH_HPP

// for std::memcpy
#include <cstring>

// for quadmath
#include <quadmath.h>

#include <prefabs/math.hpp>

namespace pr {

/**
 * @defgroup quadmath Quadmath
 *
 * `<prefabs/quadmath.hpp>`
 *
 * __C++ version__: >=C++14 (`-std=gnu++14`, `-lquadmath`)
 */
/**@{*/

/**
 * @brief Specialize `__float128`.
 */
template <>
struct numeric_limits<__float128>
{
    /**
     * @brief Set `has_infinity`.
     */
    static constexpr bool has_infinity = true;

    /**
     * @brief Set `has_quiet_NaN`.
     */
    static constexpr bool has_quiet_NaN = true;

    /**
     * @brief Set `has_signaling_NaN`.
     */
    static constexpr bool has_signaling_NaN = true;

    /**
     * @brief Set `has_denorm`.
     */
    static constexpr std::float_denorm_style has_denorm =
                     std::float_denorm_style::denorm_present;

    /**
     * @brief Wrap `FLT128_MIN_EXP`.
     */
    static constexpr int min_exponent = FLT128_MIN_EXP;

    /**
     * @brief Wrap `FLT128_MAX_EXP`.
     */
    static constexpr int max_exponent = FLT128_MAX_EXP;

    /**
     * @brief Wrap `FLT128_MIN_10_EXP`.
     */
    static constexpr int min_exponent10 = FLT128_MIN_10_EXP;

    /**
     * @brief Wrap `FLT128_MAX_10_EXP`.
     */
    static constexpr int max_exponent10 = FLT128_MAX_10_EXP;

    /**
     * @brief Wrap `FLT128_MANT_DIG = 113`.
     */
    static constexpr int digits = FLT128_MANT_DIG;

    /**
     * @brief Wrap `FLT128_DIG = 33`.
     */
    static constexpr int digits10 = FLT128_DIG;

    /**
     * @brief From `FLT128_MANT_DIG`, infer `max_digits10 = 36`.
     */
    static constexpr int max_digits10 = 36;

    /**
     * @brief Wrap `FLT128_MIN`.
     */
    static constexpr __float128 min() noexcept
    {
        return FLT128_MIN;
    }

    /**
     * @brief Wrap `FLT128_MAX`.
     */
    static constexpr __float128 max() noexcept
    {
        return FLT128_MAX;
    }

    /**
     * @brief Wrap `FLT128_EPSILON`.
     */
    static constexpr __float128 epsilon() noexcept
    {
        return FLT128_EPSILON;
    }

    /**
     * @brief For floating point types, machine echelon.
     */
    static constexpr __float128 machine_epsilon() noexcept
    {
        return epsilon() / 2;
    }

    /**
     * @brief For floating point types, echelon.
     */
    static constexpr __float128 echelon(unsigned n) noexcept
    {
        return machine_epsilon() * n / (1 - machine_epsilon() * n);
    }

    /**
     * @brief Wrap `HUGE_VALQ`.
     */
    static __float128 infinity() noexcept
    {
        return HUGE_VALQ;
    }

    /**
     * @brief Wrap `nanq()`.
     */
    static __float128 quiet_NaN() noexcept
    {
        return ::nanq("1");
    }

    /**
     * @brief Wrap `nanq()`, flip signaling bit.
     */
    static __float128 signaling_NaN() noexcept
    {
        __float128 f = ::nanq("1");
        __int128 i;
        std::memcpy(&i, &f, sizeof(f));
        i ^= __int128(1) << 111;
        std::memcpy(&f, &i, sizeof(i));
        return f;
    }

    /**
     * @brief Wrap `FLT128_DENORM_MIN`.
     */
    static constexpr __float128 denorm_min() noexcept
    {
        return FLT128_DENORM_MIN;
    }
};

/**
 * @brief Specialize `__float128`.
 */
template <>
struct numeric_constants<__float128>
{
    /**
     * @brief Wrap `M_Eq`.
     */
    static constexpr __float128 M_e() noexcept
    {
        return M_Eq;
    }

    /**
     * @brief Wrap `M_LOG2Eq`.
     */
    static constexpr __float128 M_log2e() noexcept
    {
        return M_LOG2Eq;
    }

    /**
     * @brief Wrap `M_LOG10Eq`.
     */
    static constexpr __float128 M_log10e() noexcept
    {
        return M_LOG10Eq;
    }

    /**
     * @brief Wrap `M_LN2q`.
     */
    static constexpr __float128 M_ln2() noexcept
    {
        return M_LN2q;
    }

    /**
     * @brief Wrap `M_LN10q`.
     */
    static constexpr __float128 M_ln10() noexcept
    {
        return M_LN10q;
    }

    /**
     * @brief Wrap `M_PIq`.
     */
    static constexpr __float128 M_pi() noexcept
    {
        return M_PIq;
    }

    /**
     * @brief Wrap `M_PI_2q`.
     */
    static constexpr __float128 M_pi_2() noexcept
    {
        return M_PI_2q;
    }

    /**
     * @brief Wrap `M_PI_4q`.
     */
    static constexpr __float128 M_pi_4() noexcept
    {
        return M_PI_4q;
    }

    /**
     * @brief Wrap `M_1_PIq`.
     */
    static constexpr __float128 M_1_pi() noexcept
    {
        return M_1_PIq;
    }

    /**
     * @brief Wrap `M_2_PIq`.
     */
    static constexpr __float128 M_2_pi() noexcept
    {
        return M_2_PIq;
    }

    /**
     * @brief Wrap `M_2_SQRTPIq`.
     */
    static constexpr __float128 M_2_sqrtpi() noexcept
    {
        return M_2_SQRTPIq;
    }

    /**
     * @brief Wrap `M_SQRT2q`.
     */
    static constexpr __float128 M_sqrt2() noexcept
    {
        return M_SQRT2q;
    }

    /**
     * @brief Wrap `M_SQRT1_2q`.
     */
    static constexpr __float128 M_sqrt1_2() noexcept
    {
        return M_SQRT1_2q;
    }
};

/**
 * @brief Specialize `__complex128`.
 */
template <>
struct numeric_constants<__complex128> : numeric_constants<__float128>
{
};

/**@}*/

} // namespace pr

#if !DOXYGEN
#include "quadmath.inl"
#endif // #if !DOXYGEN

#endif // #ifndef PREFABS_QUADMATH_HPP

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
#error "preform/color.hpp requires >=C++17"
#endif // #if !(__cplusplus >= 201703L)
#endif // #if !DOXYGEN
#pragma once
#ifndef PREFORM_COLOR_HPP
#define PREFORM_COLOR_HPP

// for pr::pow, pr::nthpow, ...
#include <preform/math.hpp>

// for pr::fstretch
#include <preform/float_helpers.hpp>

// for pr::multi
#include <preform/multi.hpp>

// for pr::dot
#include <preform/multi_math.hpp>

// for pr::fstretch(pr::multi)
#include <preform/multi_float_helpers.hpp>

namespace pr {

/**
 * @defgroup color Color
 *
 * `<preform/color.hpp>`
 *
 * __C++ version__: >=C++17
 */
/**@{*/

/**
 * @name Color conversion
 */
/**@{*/

/**
 * @brief Encode linear RGB as sRGB.
 *
 * @f[
 *      \operatorname{srgbenc}(v) =
 *      \begin{cases}
 *          12.92 v                 & v \le 0.0031308
 *      \\  1.055 v^{1/2.4} - 0.055 & v >   0.0031308
 *      \end{cases}
 * @f]
 *
 * @note
 * If `T` is an unsigned integral type,
 * - uses `fstretch()` to convert to `double` before recursing, then
 * - uses `fstretch()` to convert to `T`.
 */
template <typename T>
inline std::enable_if_t<
       std::is_unsigned<T>::value ||
       std::is_floating_point<T>::value, T> srgbenc(T v)
{
    if constexpr (std::is_floating_point<T>::value) {
        if (v <= T(0.0031308)) {
            return T(12.92) * v;
        }
        else {
            return T(1.055) * pr::pow(v, T(1) / T(2.4)) -
                   T(0.055);
        }
    }
    else {
        return pr::fstretch<T>(srgbenc(
               pr::fstretch<double>(v)));
    }
}

/**
 * @brief Encode linear RGB as sRGB.
 *
 * @f[
 *      \begin{bmatrix} r' \\ g' \\ b' \end{bmatrix} \gets
 *      \begin{bmatrix}
 *          \operatorname{srgbenc}(r)
 *      \\  \operatorname{srgbenc}(g)
 *      \\  \operatorname{srgbenc}(b)
 *      \end{bmatrix}
 * @f]
 *
 * @note
 * If `T` is an unsigned integral type,
 * - uses `fstretch()` to convert to `double` before recursing, then
 * - uses `fstretch()` to convert to `T`.
 */
template <typename T>
inline std::enable_if_t<
       std::is_unsigned<T>::value ||
       std::is_floating_point<T>::value,
       multi<T, 3>> srgbenc(const multi<T, 3>& v)
{
    return {{
        pr::srgbenc(v[0]),
        pr::srgbenc(v[1]),
        pr::srgbenc(v[2])
    }};
}

/**
 * @brief Encode linear RGB as sRGB.
 *
 * @f[
 *      \begin{bmatrix} r' \\ g' \\ b' \\ \alpha' \end{bmatrix} \gets
 *      \begin{bmatrix}
 *          \operatorname{srgbenc}(r)
 *      \\  \operatorname{srgbenc}(g)
 *      \\  \operatorname{srgbenc}(b)
 *      \\  \alpha
 *      \end{bmatrix}
 * @f]
 *
 * @note
 * If `T` is an unsigned integral type,
 * - uses `fstretch()` to convert to `double` before recursing, then
 * - uses `fstretch()` to convert to `T`.
 */
template <typename T>
inline std::enable_if_t<
       std::is_unsigned<T>::value ||
       std::is_floating_point<T>::value,
       multi<T, 4>> srgbenc(const multi<T, 4>& v)
{
    return {{
        pr::srgbenc(v[0]),
        pr::srgbenc(v[1]),
        pr::srgbenc(v[2]),
        v[3]
    }};
}

/**
 * @brief Decode linear RGB from sRGB.
 *
 * @f[
 *      \operatorname{srgbdec}(v) =
 *      \begin{cases}
 *          v / 12.92                   & v \le 0.04045
 *      \\  ((v + 0.055) / 1.055)^{2.4} & v >   0.04045
 *      \end{cases}
 * @f]
 *
 * @note
 * If `T` is an unsigned integral type,
 * - uses `fstretch()` to convert to `double` before recursing, then
 * - uses `fstretch()` to convert to `T`.
 */
template <typename T>
inline std::enable_if_t<
       std::is_unsigned<T>::value ||
       std::is_floating_point<T>::value, T> srgbdec(T v)
{
    if constexpr (std::is_floating_point<T>::value) {
        if (v <= T(0.04045)) {
            return v / T(12.92);
        }
        else {
            return pr::pow((v + T(0.055)) / T(1.055), T(2.4));
        }
    }
    else {
        return pr::fstretch<T>(srgbdec(
               pr::fstretch<double>(v)));
    }
}

/**
 * @brief Decode linear RGB from sRGB.
 *
 * @f[
 *      \begin{bmatrix} r' \\ g' \\ b' \end{bmatrix} \gets
 *      \begin{bmatrix}
 *          \operatorname{srgbdec}(r)
 *      \\  \operatorname{srgbdec}(g)
 *      \\  \operatorname{srgbdec}(b)
 *      \end{bmatrix}
 * @f]
 *
 * @note
 * If `T` is an unsigned integral type,
 * - uses `fstretch()` to convert to `double` before recursing, then
 * - uses `fstretch()` to convert to `T`.
 */
template <typename T>
inline std::enable_if_t<
       std::is_unsigned<T>::value ||
       std::is_floating_point<T>::value,
       multi<T, 3>> srgbdec(const multi<T, 3>& v)
{
    return {{
        pr::srgbenc(v[0]),
        pr::srgbenc(v[1]),
        pr::srgbenc(v[2])
    }};
}

/**
 * @brief Decode linear RGB from sRGB.
 *
 * @f[
 *      \begin{bmatrix} r' \\ g' \\ b' \\ \alpha' \end{bmatrix} \gets
 *      \begin{bmatrix}
 *          \operatorname{srgbdec}(r)
 *      \\  \operatorname{srgbdec}(g)
 *      \\  \operatorname{srgbdec}(b)
 *      \\  \alpha
 *      \end{bmatrix}
 * @f]
 *
 * @note
 * If `T` is an unsigned integral type,
 * - uses `fstretch()` to convert to `double` before recursing, then
 * - uses `fstretch()` to convert to `T`.
 */
template <typename T>
inline std::enable_if_t<
       std::is_unsigned<T>::value ||
       std::is_floating_point<T>::value,
       multi<T, 4>> srgbdec(const multi<T, 4>& v)
{
    return {{
        pr::srgbdec(v[0]),
        pr::srgbdec(v[1]),
        pr::srgbdec(v[2]),
        v[3]
    }};
}

/**
 * @brief Fit of CIE 1931 X by Wyman et. al.
 *
 * @param[in] um
 * Wavelength in micrometers.
 *
 * @see [This publication][1] by Wyman, Sloan, and Shirley.
 * [1]: http://jcgt.org/published/0002/02/01/
 */
template <typename T>
inline std::enable_if_t<std::is_floating_point<T>::value, T> wymanx(T um)
{
    T t1 = um - T(0.4420);
    T t2 = um - T(0.5998);
    T t3 = um - T(0.5011);
    t1 *= pr::signbit(t1) ? T(62.4) : T(37.4);
    t2 *= pr::signbit(t2) ? T(26.4) : T(32.3);
    t3 *= pr::signbit(t3) ? T(49.0) : T(38.2);
    return T(0.362) * pr::exp(T(-0.5) * t1 * t1) +
           T(1.056) * pr::exp(T(-0.5) * t2 * t2) -
           T(0.065) * pr::exp(T(-0.5) * t3 * t3);
}

/**
 * @brief Fit of CIE 1931 Y by Wyman et. al.
 *
 * @param[in] um
 * Wavelength in micrometers.
 *
 * @see [This publication][1] by Wyman, Sloan, and Shirley.
 * [1]: http://jcgt.org/published/0002/02/01/
 */
template <typename T>
inline std::enable_if_t<std::is_floating_point<T>::value, T> wymany(T um)
{
    T t1 = um - T(0.5688);
    T t2 = um - T(0.5309);
    t1 *= pr::signbit(t1) ? T(21.3) : T(24.7);
    t2 *= pr::signbit(t2) ? T(61.3) : T(32.2);
    return T(0.821) * pr::exp(T(-0.5) * t1 * t1) +
           T(0.286) * pr::exp(T(-0.5) * t2 * t2);
}

/**
 * @brief Fit of CIE 1931 Z by Wyman et. al.
 *
 * @param[in] um
 * Wavelength in micrometers.
 *
 * @see [This publication][1] by Wyman, Sloan, and Shirley.
 * [1]: http://jcgt.org/published/0002/02/01/
 */
template <typename T>
inline std::enable_if_t<std::is_floating_point<T>::value, T> wymanz(T um)
{
    T t1 = um - T(0.4370);
    T t2 = um - T(0.4590);
    t1 *= pr::signbit(t1) ? T(84.5) : T(27.8);
    t2 *= pr::signbit(t2) ? T(38.5) : T(72.5);
    return T(1.217) * pr::exp(T(-0.5) * t1 * t1) +
           T(0.681) * pr::exp(T(-0.5) * t2 * t2);
}

/**
 * @brief XYZ triple to RGB triple.
 *
 * This uses standard CIE parameters:
 * - @f$ C_r = (0.7350, 0.2650) @f$,
 * - @f$ C_g = (0.2740, 0.7170) @f$,
 * - @f$ C_b = (0.1670, 0.0090) @f$, and
 * - @f$ W = (1, 1, 1) @f$.
 *
 * @see
 * [Bruce Lindbloom's page][1].
 * [1]: http://brucelindbloom.com/index.html?Eqn_RGB_XYZ_Matrix.html
 *
 * @note
 * If `T` is an unsigned integral type,
 * - uses `fstretch()` to convert to `double` before recursing, then
 * - uses `fstretch()` to convert to `T`.
 */
template <typename T>
inline std::enable_if_t<
       std::is_unsigned<T>::value ||
       std::is_floating_point<T>::value,
       multi<T, 3>> xyztorgb(const multi<T, 3>& v)
{
    if constexpr (std::is_floating_point<T>::value) {
        const multi<T, 3, 3> m = {{
            {{T(+2.3706743), T(-0.9000405), T(-0.4706338)}},
            {{T(-0.5138850), T(+1.4253036), T(+0.0885814)}},
            {{T(+0.0052982), T(-0.0146949), T(+1.0093968)}}
        }};
        return pr::dot(m, v);
    }
    else {
        return pr::fstretch<T>(pr::xyztorgb(
               pr::fstretch<double>(v)));
    }
}

/**
 * @brief RGB triple to XYZ triple.
 *
 * This uses standard CIE parameters:
 * - @f$ C_r = (0.7350, 0.2650) @f$,
 * - @f$ C_g = (0.2740, 0.7170) @f$,
 * - @f$ C_b = (0.1670, 0.0090) @f$, and
 * - @f$ W = (1, 1, 1) @f$.
 *
 * @see
 * [Bruce Lindbloom's page][1].
 * [1]: http://brucelindbloom.com/index.html?Eqn_RGB_XYZ_Matrix.html
 *
 * @note
 * If `T` is an unsigned integral type,
 * - uses `fstretch()` to convert to `double` before recursing, then
 * - uses `fstretch()` to convert to `T`.
 */
template <typename T>
inline std::enable_if_t<
       std::is_unsigned<T>::value ||
       std::is_floating_point<T>::value,
       multi<T, 3>> rgbtoxyz(const multi<T, 3>& v)
{
    if constexpr (std::is_floating_point<T>::value) {
        const multi<T, 3, 3> m = {{
            {{T(0.4887180), T(0.3106803), T(0.2006017)}},
            {{T(0.1762044), T(0.8129847), T(0.0108109)}},
            {{T(0.0000000), T(0.0102048), T(0.9897952)}}
        }};
        return pr::dot(m, v);
    }
    else {
        return pr::fstretch<T>(pr::rgbtoxyz(
               pr::fstretch<double>(v)));
    }
}

/**
 * @brief RGB to XYZ conversion matrix.
 *
 * @see
 * [Bruce Lindbloom's page][1].
 * [1]: http://brucelindbloom.com/index.html?Eqn_RGB_XYZ_Matrix.html
 */
template <typename T>
inline std::enable_if_t<
       std::is_floating_point<T>::value,
       multi<T, 3, 3>> rgbtoxyz(
                    const multi<T, 2>& cr,
                    const multi<T, 2>& cg,
                    const multi<T, 2>& cb,
                    const multi<T, 3>& w)
{
    // Conversion matrix.
    multi<T, 3, 3> m;

    // Temp matrix.
    multi<T, 3, 3> a = {{
        {{cr[0] / cr[1],
          cg[0] / cg[1],
          cb[0] / cb[1]}},
        {{T(1), T(1), T(1)}},
        {{(T(1) - cr[0] - cr[1]) / cr[1],
          (T(1) - cg[0] - cg[1]) / cg[1],
          (T(1) - cb[0] - cb[1]) / cb[1]}}
    }};

    // Temp matrix cofactors.
    multi<T, 3, 3> acof = {{
        pr::cross(a[1], a[2]),
        pr::cross(a[2], a[0]),
        pr::cross(a[0], a[1])
    }};

    // Temp matrix inverse times reference white.
    multi<T, 3> s = dot(w, acof) / acof[1].sum();

    // Initialize conversion matrix.
    m[0] = a[0] * s;
    m[1] = a[1] * s;
    m[2] = a[2] * s;
    return m;
}

/**
 * @brief XYZ triple to Lab triple.
 *
 * @f[
 *      \begin{bmatrix} L \\ a \\ b \end{bmatrix} \gets
 *      \begin{bmatrix}
 *          116 f(Y) - 16
 *      \\  500 (f(X) - f(Y))
 *      \\  200 (f(Y) - f(Z))
 *      \end{bmatrix}
 * @f]
 * where
 * @f[
 *      f(t) = 
 *      \begin{cases}
 *          t^{1/3}                     & t >   216 / 24389
 *      \\  ((24389 / 27) t + 16) / 116 & t \le 216 / 24389
 *      \end{cases}
 * @f]
 *
 * @see
 * [Bruce Lindbloom's page][1].
 * [1]: http://brucelindbloom.com/index.html?Eqn_XYZ_to_Lab.html
 *
 * @note
 * If `T` is an unsigned integral type,
 * - uses `fstretch()` to convert to `double` before recursing, then
 * - uses `fstretch()` to convert to `T`.
 */
template <typename T>
inline std::enable_if_t<
       std::is_unsigned<T>::value ||
       std::is_floating_point<T>::value,
       multi<T, 3>> xyztolab(const multi<T, 3>& v)
{
    if constexpr (std::is_floating_point<T>::value) {
        auto f = [](T t) {
            if (t > T(216) / T(24389)) {
                return pr::cbrt(t);
            }
            else {
                return (t * (T(24389) / T(27)) + T(16)) / T(116);
            }
        };
        return {{
            T(116) * f(v[1]) - T(16),
            T(500) * (f(v[0]) - f(v[1])),
            T(200) * (f(v[1]) - f(v[2]))
        }};
    }
    else {
        return pr::fstretch<T>(pr::xyztolab(
               pr::fstretch<double>(v)));
    }
}

/**
 * @brief Lab triple to XYZ triple.
 *
 * @f[
 *      \begin{bmatrix} X \\ Y \\ Z \end{bmatrix} \gets
 *      \begin{bmatrix}
 *          f^{-1}(f_X)
 *      \\  f^{-1}(f_Y)
 *      \\  f^{-1}(f_Z)
 *      \end{bmatrix}
 * @f]
 * where
 * @f[
 *      \begin{aligned}
 *          f_Y &= (L + 16) / 116
 *      \\  f_X &= f_Y + a / 500
 *      \\  f_Z &= f_Y - b / 200
 *      \end{aligned}
 * @f]
 * and where
 * @f[
 *      f^{-1}(t) = 
 *      \begin{cases}
 *          t^3                       & t^3 >   216 / 24389
 *      \\  (116 t - 16) (27 / 24389) & t^3 \le 216 / 24389
 *      \end{cases}
 * @f]
 *
 * @see
 * [Bruce Lindbloom's page][1].
 * [1]: http://brucelindbloom.com/index.html?Eqn_Lab_to_XYZ.html
 *
 * @note
 * If `T` is an unsigned integral type,
 * - uses `fstretch()` to convert to `double` before recursing, then
 * - uses `fstretch()` to convert to `T`. 
 */
template <typename T>
inline std::enable_if_t<
       std::is_unsigned<T>::value ||
       std::is_floating_point<T>::value,
       multi<T, 3>> labtoxyz(const multi<T, 3>& v)
{
    if constexpr (std::is_floating_point<T>::value) {
        auto finv = [](T t) {
            if (pr::nthpow(t, 3) > T(216) / T(24389)) {
                return pr::nthpow(t, 3);
            }
            else {
                return (t * T(116) - T(16)) * (T(27) / T(24389));
            }
        };
        T fy = (v[0] + T(16)) / T(116);
        T fx = fy + v[1] / T(500);
        T fz = fy - v[2] / T(200);
        return {{
            finv(fx),
            finv(fy),
            finv(fz)
        }};
    }
    else {
        return pr::fstretch<T>(pr::labtoxyz(
               pr::fstretch<double>(v)));
    }
}

/**@}*/

/**
 * @brief Composite modes.
 */
enum class composite_mode : int
{
    /**
     * @brief Only source.
     *
     * - @f$ f_{\text{src}} = 1 @f$
     * - @f$ f_{\text{dst}} = 0 @f$
     */
    src,

    /**
     * @brief Only destination.
     *
     * - @f$ f_{\text{src}} = 0 @f$
     * - @f$ f_{\text{dst}} = 1 @f$
     */
    dst,

    /**
     * @brief Source over destination.
     *
     * - @f$ f_{\text{src}} = 1 @f$
     * - @f$ f_{\text{dst}} = 1 - \alpha_{\text{src}} @f$
     */
    src_over,

    /**
     * @brief Destination over source.
     *
     * - @f$ f_{\text{src}} = 1 - \alpha_{\text{dst}} @f$
     * - @f$ f_{\text{dst}} = 1 @f$
     */
    dst_over,

    /**
     * @brief Source inside destination.
     *
     * - @f$ f_{\text{src}} = \alpha_{\text{dst}} @f$
     * - @f$ f_{\text{dst}} = 0 @f$
     */
    src_in,

    /**
     * @brief Destination inside source.
     *
     * - @f$ f_{\text{src}} = 0 @f$
     * - @f$ f_{\text{dst}} = \alpha_{\text{src}} @f$
     */
    dst_in,

    /**
     * @brief Source outside destination.
     *
     * - @f$ f_{\text{src}} = 1 - \alpha_{\text{dst}} @f$
     * - @f$ f_{\text{dst}} = 0 @f$
     */
    src_out,

    /**
     * @brief Destination outside source.
     *
     * - @f$ f_{\text{src}} = 0 @f$
     * - @f$ f_{\text{dst}} = 1 - \alpha_{\text{src}} @f$
     */
    dst_out,

    /**
     * @brief Source atop destination.
     *
     * - @f$ f_{\text{src}} = \alpha_{\text{dst}} @f$
     * - @f$ f_{\text{dst}} = 1 - \alpha_{\text{src}} @f$
     */
    src_atop,

    /**
     * @brief Destination atop source.
     *
     * - @f$ f_{\text{src}} = 1 - \alpha_{\text{dst}} @f$
     * - @f$ f_{\text{dst}} = \alpha_{\text{src}} @f$
     */
    dst_atop,

    /**
     * @brief Exclusive or.
     *
     * - @f$ f_{\text{src}} = 1 - \alpha_{\text{dst}} @f$
     * - @f$ f_{\text{dst}} = 1 - \alpha_{\text{src}} @f$
     */
    exclusive_or,

    /**
     * @brief Plus.
     *
     * - @f$ f_{\text{src}} = 1 @f$
     * - @f$ f_{\text{dst}} = 1 @f$
     */
    plus
};

/**
 * @name Alpha compositing
 */
/**@{*/

/**
 * @brief Premultiply.
 *
 * @f[
 *      \operatorname{premultiply}(
 *      \mathbf{v} = [\mathbf{c} \;\; \alpha]^\top) =
 *                   [\alpha \mathbf{c} \;\; \alpha]^\top 
 * @f]
 *
 * @note
 * If `T` is an unsigned integral type,
 * - uses `fstretch()` to convert to `double` before recursing, then
 * - uses `fstretch()` to convert to `T`. 
 */
template <typename T, std::size_t N>
inline std::enable_if_t<
       std::is_unsigned<T>::value ||
       std::is_floating_point<T>::value,
       multi<T, N>> premultiply(const multi<T, N>& v)
{
    if constexpr (std::is_floating_point<T>::value) {
        multi<T, N> a = 
        multi<T, N>::value(v[N - 1]);
        a[N - 1] = 1;
        return v * a;
    }
    else {
        return pr::fstretch<T>(pr::premultiply(
               pr::fstretch<double>(v)));
    }
}

/**
 * @brief Unpremultiply.
 *
 * @f[
 *      \operatorname{unpremultiply}(
 *      \mathbf{v} = [\mathbf{c} \;\; \alpha]^\top) =
 *      \begin{cases}
 *          [\mathbf{c} / \alpha \;\; \alpha]^\top  & \alpha > 0
 *      \\  [\mathbf{c}          \;\; \alpha]^\top  & \alpha = 0
 *      \end{cases}
 * @f]
 *
 * @note
 * If `T` is an unsigned integral type,
 * - uses `fstretch()` to convert to `double` before recursing, then
 * - uses `fstretch()` to convert to `T`. 
 */
template <typename T, std::size_t N>
inline std::enable_if_t<
       std::is_unsigned<T>::value ||
       std::is_floating_point<T>::value,
       multi<T, N>> unpremultiply(const multi<T, N>& v)
{
    if constexpr (std::is_floating_point<T>::value) {
        if (v[N - 1] > 0) {
            multi<T, N> a = 
            multi<T, N>::value(v[N - 1]);
            a[N - 1] = 1;
            return v / a;
        }
        else {
            return v;
        }
    }
    else {
        return pr::fstretch<T>(pr::unpremultiply(
               pr::fstretch<double>(v)));
    }
}

/**
 * @brief Composite via Porter-Duff equation (premultiplied).
 *
 * @f[
 *      \mathbf{v} \gets
 *      f_{\text{src}} \mathbf{v}_{\text{src}} +
 *      f_{\text{dst}} \mathbf{v}_{\text{dst}}
 * @f]
 *
 * @note
 * If `T` is an unsigned integral type,
 * - uses `fstretch()` to convert to `double` before recursing, then
 * - uses `fstretch()` to convert to `T`. 
 */
template <typename T, std::size_t N>
inline std::enable_if_t<
       std::is_unsigned<T>::value ||
       std::is_floating_point<T>::value, 
       multi<T, N>> composite_premul(
                    composite_mode mode,
                    const multi<T, N>& vsrc,
                    const multi<T, N>& vdst)
{
    if constexpr (std::is_floating_point<T>::value) {
        // Alpha.
        const T& asrc = vsrc[N - 1];
        const T& adst = vdst[N - 1];

        // Coefficients.
        T fsrc = 0;
        T fdst = 0;
        switch (mode) {
            // Only source.
            case composite_mode::src: 
                fsrc = 1;
                fdst = 0; 
                break;

            // Only destination.
            case composite_mode::dst:
                fsrc = 0; 
                fdst = 1;
                break;

            // Source over destination.
            case composite_mode::src_over: 
                fsrc = 1;
                fdst = 1 - asrc; 
                break;

            // Destination over source.
            case composite_mode::dst_over:
                fsrc = 1 - adst; 
                fdst = 1;
                break;

            // Source inside destination.
            case composite_mode::src_in:
                fsrc = adst;
                fdst = 0; 
                break;

            // Destination inside source.
            case composite_mode::dst_in: 
                fsrc = 0; 
                fdst = asrc;
                break;

            // Source outside destination.
            case composite_mode::src_out:
                fsrc = 1 - adst;
                fdst = 0; 
                break;

            // Destination outside source.
            case composite_mode::dst_out:
                fsrc = 0; 
                fdst = 1 - asrc;
                break;

            // Source atop destination.
            case composite_mode::src_atop:
                fsrc = adst;
                fdst = 1 - asrc; 
                break;

            // Destination atop source.
            case composite_mode::dst_atop:
                fsrc = 1 - adst;
                fdst = asrc;
                break;

            // Exclusive or.
            case composite_mode::exclusive_or:
                fsrc = 1 - adst;
                fdst = 1 - asrc; 
                break;

            // Plus.
            case composite_mode::plus:
                fsrc = 1;
                fdst = 1; 
                break;

            default:
                // Error?
                break;
        }
        return fsrc * vsrc + fdst * vdst;
    }
    else {
        return 
            pr::fstretch<T>(
                pr::composite_premul(mode,
                pr::fstretch<double>(vsrc),
                pr::fstretch<double>(vdst)));
    }
}

/**
 * @brief Composite via Porter-Duff equation.
 *
 * @f[
 *      \mathbf{v} \gets
 *      \operatorname{unpremultiply}(
 *      f_{\text{src}} \operatorname{premultiply}(\mathbf{v}_{\text{src}}) +
 *      f_{\text{dst}} \operatorname{premultiply}(\mathbf{v}_{\text{dst}}))
 * @f]
 *
 * @note
 * If `T` is an unsigned integral type,
 * - uses `fstretch()` to convert to `double` before recursing, then
 * - uses `fstretch()` to convert to `T`. 
 */
template <typename T, std::size_t N>
inline std::enable_if_t<
       std::is_unsigned<T>::value ||
       std::is_floating_point<T>::value, 
       multi<T, N>> composite(
                    composite_mode mode,
                    const multi<T, N>& vsrc,
                    const multi<T, N>& vdst)
{
    if constexpr (std::is_floating_point<T>::value) {
        return 
            pr::unpremultiply(
                pr::composite_premul(mode,
                pr::premultiply(vsrc),
                pr::premultiply(vdst)));
    }
    else {
        return 
            pr::fstretch<T>(
                pr::composite(mode,
                pr::fstretch<double>(vsrc),
                pr::fstretch<double>(vdst)));
    }
}

/**@}*/

/**@}*/

} // namespace pr

#endif // #ifndef PREFORM_COLOR_HPP
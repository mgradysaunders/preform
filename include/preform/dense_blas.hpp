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
#if !(__cplusplus >= 201402L)
#error "preform/dense_blas.hpp requires >=C++14"
#endif // #if !(__cplusplus >= 201402L)
#endif // #if !DOXYGEN
#pragma once
#ifndef PREFORM_DENSE_BLAS_HPP
#define PREFORM_DENSE_BLAS_HPP

// for std::vector
#include <vector>

// for pr::sqrt, pr::fmin, pr::fmax, ...
#include <preform/math.hpp>

// for pr::dense_vector_view
#include <preform/dense_vector_view.hpp>

// for pr::dense_matrix_view
#include <preform/dense_matrix_view.hpp>

namespace pr {

/**
 * @defgroup dense_blas Basic linear algebra subroutines
 *
 * `<preform/dense_blas.hpp>`
 *
 * __C++ version__: >=C++14
 */
/**@{*/

template <typename T>
struct default_blas_traits
{
    typedef T value_type;

    typedef T float_type;

    __attribute__((always_inline))
    static float_type abs(const value_type& x)
    {
        return pr::fabs(x);
    }

    __attribute__((always_inline))
    static float_type norm(const value_type& x)
    {
        return x * x;
    }

    __attribute__((always_inline))
    static value_type conj(const value_type& x)
    {
        return x;
    }

    __attribute__((always_inline))
    static value_type sign(const value_type& x)
    {
        return pr::copysign(value_type(1), x);
    }
};

template <typename T>
struct default_blas_traits<std::complex<T>>
{
    typedef std::complex<T> value_type;

    typedef T float_type;

    __attribute__((always_inline))
    static float_type abs(const value_type& x)
    {
        return pr::abs(x);
    }

    __attribute__((always_inline))
    static float_type norm(const value_type& x)
    {
        return pr::norm(x);
    }

    __attribute__((always_inline))
    static value_type conj(const value_type& x)
    {
        return pr::conj(x);
    }

    static value_type sign(const value_type& x)
    {
        return pr::sign(x);
    }
};

/**
 * @brief Dense basic linear algebra subroutines.
 */
template <
    typename T, 
    typename Ttraits = default_blas_traits<T>
    >
struct dense_blas
{
public:

    /**
     * @brief Value type.
     */
    typedef typename Ttraits::value_type value_type;

    /**
     * @brief Float type.
     */
    typedef typename Ttraits::float_type float_type;

    // Sanity check.
    static_assert(
        std::is_floating_point<float_type>::value,
        "float_type must be floating point");

    /**
     * @brief Wrap `Ttraits::abs()`.
     *
     * For every @f$ x \in T @f$, the implementation must
     * satisfy
     * - @f$ |x| = \sqrt{x^\dagger x} \in \mathbb{R} @f$,
     * - @f$ |x| \ge 0 @f$, and
     * - @f$ |x| = 0 @f$ iff @f$ x \sim 0 @f$.
     */
    __attribute__((always_inline))
    static float_type abs(const value_type& x)
    {
        return Ttraits::abs(x);
    }

    /**
     * @brief Wrap `Ttraits::norm()`.
     *
     * For every @f$ x \in T @f$, the implementation must
     * satisfy
     * - @f$ |x|^2 = x^\dagger x \in \mathbb{R} @f$,
     * - @f$ |x|^2 \ge 0 @f$, and
     * - @f$ |x|^2 = 0 @f$ iff @f$ x \sim 0 @f$.
     */
    __attribute__((always_inline))
    static float_type norm(const value_type& x)
    {
        return Ttraits::norm(x);
    }

    /**
     * @brief Wrap `Ttraits::conj()`.
     *
     * For every @f$ x \in T @f$, the implementation must
     * satisfy
     * - @f$ x = (x^\dagger)^\dagger @f$.
     */
    __attribute__((always_inline))
    static value_type conj(const value_type& x)
    {
        return Ttraits::conj(x);
    }

    /**
     * @brief Wrap `Ttraits::sign()`.
     *
     * For every @f$ x \in T @f$, the implementation must
     * satisfy
     * - @f$ \operatorname{sign}(x) = x/|x| @f$ iff @f$ |x| > 0 @f$,
     * - @f$ \operatorname{sign}(x) = 1 @f$ if @f$ |x| = 0 @f$.
     */
    __attribute__((always_inline))
    static value_type sign(const value_type& x)
    {
        return Ttraits::sign(x);
    }

public:

    /**
     * @name Basic operations
     */
    /**@{*/

    /**
     * @brief Dot product.
     *
     * @f[
     *      \mathbf{x} \cdot
     *      \mathbf{y} = \sum_{k=0}^{n-1} x_{[k]} y_{[k]}
     * @f]
     *
     * @param[in] x
     * Vector @f$ \mathbf{x} @f$.
     *
     * @param[in] y
     * Vector @f$ \mathbf{y} @f$.
     *
     * @throw std::invalid_argument
     * Unless `x.size() == y.size()`.
     */
    static value_type dot(
                dense_vector_view<const value_type*> x,
                dense_vector_view<const value_type*> y)
    {
        if (x.size() != y.size()) {
            throw std::invalid_argument(__PRETTY_FUNCTION__);
        }

        // Default construct.
        value_type res = {};

        // Dot.
        auto itrx = x.begin();
        auto itry = y.begin();
        for (; itry < y.end(); ++itrx, ++itry) {
            res = (*itrx) * (*itry) + res;
        }

        return res;
    }

    /**
     * @brief Dot product, conjugating left argument.
     *
     * @f[
     *      \bar{\mathbf{x}} \cdot
     *           \mathbf{y} = \sum_{k=0}^{n-1} \bar{x}_{[k]} y_{[k]}
     * @f]
     *
     * @param[in] x
     * Vector @f$ \mathbf{x} @f$.
     *
     * @param[in] y
     * Vector @f$ \mathbf{y} @f$.
     * 
     * @throw std::invalid_argument
     * Unless `x.size() == y.size()`.
     */
    static value_type dot_conj(
                dense_vector_view<const value_type*> x,
                dense_vector_view<const value_type*> y)
    {
        if (x.size() != y.size()) {
            throw std::invalid_argument(__PRETTY_FUNCTION__);
        }

        // Default construct.
        value_type res = {};

        // Dot.
        auto itrx = x.begin();
        auto itry = y.begin();
        for (; itry < y.end(); ++itrx, ++itry) {
            res = conj(*itrx) * (*itry) + res;
        }

        return res;
    }

    /**
     * @brief @f$ L^2 @f$ length.
     *
     * @f[
     *      \sqrt{\sum_{j=0}^{n-1} |x_{[j]}|^2}
     * @f]
     *
     * @param[in] x
     * Vector @f$ \mathbf{x} @f$.
     *
     * @note
     * This method maintains a static thread-local buffer of 
     * temporary values. If `x` is empty, the implementation 
     * resets the buffer and returns immediately.
     */
    static float_type length(dense_vector_view<const value_type*> x)
    {
        switch (x.size()) {
            case 1: 
                // Delegate.
                return float_type(abs(x[0]));
            case 2: 
                // Delegate.
                return float_type(pr::hypot(abs(x[0]), abs(x[1])));
            default:
                break;
        }

        // Temporary buffer.
        static thread_local std::vector<float_type> tmpbuf;
        if (tmpbuf.size() < std::size_t(x.size())) {
            tmpbuf.resize(x.size());
        }

        // Empty?
        if (x.empty()) {
            // Use as signal to free buffer.
            tmpbuf.clear();
            tmpbuf.shrink_to_fit();
            return 0;
        }

        // Temporary values vector view.
        dense_vector_view<float_type*> tmp = {tmpbuf.data(), 1, x.size()};

        // Temporary extrema.
        float_type tmpmin = pr::numeric_limits<float_type>::max();
        float_type tmpmax = 0;
        {
            // Initialize.
            auto itrtmp = tmp.begin();
            auto itrx = x.begin();
            for (; itrx < x.end(); ++itrtmp, ++itrx) {
                *itrtmp = abs(*itrx);
                if (*itrtmp != 0) {
                    tmpmin = pr::fmin(tmpmin, *itrtmp);
                    tmpmax = pr::fmax(tmpmax, *itrtmp);
                }
            }
        }

        // Impending overflow or underflow?
        if (tmpmax * 
            tmpmax >= pr::numeric_limits<float_type>::max() / x.size() ||
            tmpmin <= pr::numeric_limits<float_type>::min_squarable()) {
            // Factor out maximum.
            if (tmpmax >= pr::numeric_limits<float_type>::min_invertible()) {
                tmp *= 1 / tmpmax;
            }
            else {
                tmp /= tmpmax; // Inverse overflows.
            }

            // Length.
            float_type tmpsum = 0;
            for (auto itrtmp = tmp.begin();
                      itrtmp < tmp.end(); ++itrtmp) {
                tmpsum = (*itrtmp) * (*itrtmp) + tmpsum;
            }
            return pr::sqrt(tmpsum) * tmpmax;
        }
        else {

            // Length.
            float_type tmpsum = 0;
            for (auto itrtmp = tmp.begin();
                      itrtmp < tmp.end(); ++itrtmp) {
                tmpsum = (*itrtmp) * (*itrtmp) + tmpsum;
            }
            return pr::sqrt(tmpsum);
        }
    }

    /**
     * @brief @f$ L^2 @f$ normalize.
     *
     * @param[inout] x
     * Vector @f$ \mathbf{x} @f$.
     */
    static void normalize(dense_vector_view<value_type*> x)
    {
        float_type tmplen = length(x);
        if (tmplen >= pr::numeric_limits<float_type>::min_invertible()) {
            x *= 1 / tmplen;
        }
        else if (tmplen != 0) {
            x /= tmplen;   
        }
    }

    /**
     * @brief Reflect.
     *
     * @f[
     *      \mathbf{y} \gets
     *      \mathbf{y} - 2 \mathbf{x} (\bar{\mathbf{x}} \cdot \mathbf{y}) 
     * @f]
     *
     * @param[in] x
     * Vector @f$ \mathbf{x} @f$.
     *
     * @param[inout] y
     * Vector @f$ \mathbf{y} @f$.
     *
     * @throw std::invalid_argument
     * Unless `x.size() == y.size()`.
     *
     * @note
     * If the implementations of `abs`, `norm`, `conj`, and `sign`
     * do not violate any fundamental assumptions and @f$ \mathbf{x} @f$
     * is unit length, this operation is unitary and involutory, even 
     * if the underlying type is non-commutative.
     */
    static void reflect(
                dense_vector_view<const value_type*> x,
                dense_vector_view<value_type*> y)
    {
        if (x.size() != y.size()) {
            throw std::invalid_argument(__PRETTY_FUNCTION__);
        }

        // Default construct.
        value_type fac = {};

        // Dot.
        {
            auto itrx = x.begin();
            auto itry = y.begin();
            for (; itry < y.end(); ++itrx, ++itry) {
                fac = conj(*itrx) * (*itry) + fac;
            }
        }

        // Absorb multiplier.
        fac *= float_type(-2);

        // Update.
        {
            auto itrx = x.begin();
            auto itry = y.begin();
            for (; itry < y.end(); ++itrx, ++itry) {
                *itry = (*itrx) * fac + (*itry);
            }
        }
    }

    /**
     * @brief Adjoint.
     *
     * @f[
     *      Y_{[j,i]} = X_{[i,j]}^\dagger
     * @f]
     *
     * @param[in] x
     * Matrix @f$ \mathbf{X} @f$.
     *
     * @param[inout] y
     * Matrix @f$ \mathbf{Y} @f$.
     *
     * @throw std::invalid_argument
     * If `x` is not empty and the dimensions of `y`
     * do not match. If `x` is empty and `y` is empty 
     * or not square.
     *
     * @note
     * If `x` is empty and `y` is square, the implementation operates 
     * on `y` in-place.
     */
    static void adjoint(
                dense_matrix_view<const value_type*> x, 
                dense_matrix_view<value_type*> y)
    {
        if (x.empty()) {

            // Ensure square.
            if (y.size0() != y.size1() || y.empty()) {
                throw std::invalid_argument(__PRETTY_FUNCTION__);
            } 

            // Swap.
            for (int i = 0; i < y.size0(); i++) {
                y[i][i] = conj(y[i][i]);
                for (int j = i + 1; j < y.size0(); j++) {
                    value_type tmp0 = conj(y[i][j]);
                    value_type tmp1 = conj(y[j][i]);
                    y[i][j] = tmp1;
                    y[j][i] = tmp0;
                }
            }
        }
        else {

            // Ensure matching dimensions.
            if (x.size0() != y.size1() ||
                x.size1() != y.size0()) {
                throw std::invalid_argument(__PRETTY_FUNCTION__);
            }

            // Copy.
            for (int i = 0; i < x.size0(); i++) {
                for (int j = 0; j < x.size1(); j++) {
                    y[j][i] = conj(x[i][j]);
                }
            }
        }
    }

    /**
     * @brief Load identity.
     *
     * @param[out] x
     * Matrix @f$ \mathbf{X} @f$.
     */
    static void load_identity(dense_matrix_view<value_type*> x)
    {
        for (int i = 0; i < x.size0(); i++)
        for (int j = 0; j < x.size1(); j++) {
            x[i][j] = float_type(i == j);
        }
    }

    /**@}*/

public:

    /**
     * @name Householder transformations
     */
    /**@{*/

    /**
     * @brief Householder step.
     *
     * For @f$ \mathbf{X} \in T^{m \times n} @f$, construct
     * the @f$ (p, q) @f$ reflector @f$ \mathbf{w} \in T^{m-p} @f$ as
     * follows:
     * - let @f$ \mathbf{w} = X_{[p:m,q]} @f$;
     * - let @f$ \alpha = \operatorname{sign}(w_{[0]})
     *                    \lVert\mathbf{w}\rVert @f$;
     * - update @f$ \mathbf{w} \gets 
     *              \mathbf{w} + \alpha\mathbf{\delta}_0 @f$;
     * - update @f$ \mathbf{w} \gets 
     *              \mathbf{w} / \lVert\mathbf{w}\rVert @f$ to normalize.
     * 
     * For @f$ j \ge q @f$, reflect 
     * the @f$ X_{[p:m,j]} @f$ over @f$ \mathbf{w} @f$. This 
     * assumes sequential application of Householder steps, such that 
     * @f$ X_{[p:m,j]} = 0 @f$ for @f$ j < q @f$.
     *
     * @param[in] p
     * Target index @f$ p @f$.
     *
     * @param[in] q
     * Target index @f$ q @f$.
     *
     * @param[inout] x
     * Matrix @f$ \mathbf{X} @f$.
     *
     * @param[inout] y
     * Matrix @f$ \mathbf{Y} @f$. _Optional_.
     *
     * @note
     * This method maintains a static thread-local buffer of 
     * temporary values. If `x` is empty, the implementation 
     * resets the buffer and returns immediately.
     */
    static void householderl(
                int p,
                int q,
                dense_matrix_view<value_type*> x,
                dense_matrix_view<value_type*> y = {})
    {
        // Temporary buffer.
        static thread_local std::vector<value_type> tmpbuf;
        if (tmpbuf.size() < std::size_t(x.size0())) {
            tmpbuf.resize(x.size0());
        }

        // Empty?
        if (x.empty()) {
            // Use as signal to free buffer.
            tmpbuf.clear();
            tmpbuf.shrink_to_fit();
            return;
        }

        // Quit if out of range.
        if (p < 0 || p >= x.size0() ||
            q < 0 || q >= x.size1()) {
            return;
        }

        // Temporary buffer view.
        dense_vector_view<value_type*> w = {tmpbuf.data(), 1, x.size0() - p};

        // Copy qth column.
        std::copy(
            x.transpose()[q].range(p, x.size0()).begin(),
            x.transpose()[q].range(p, x.size0()).end(),
            w.begin());

        // Compute reflector.
        value_type alpha = sign(w[0]) * length(w);
        w[0] += alpha;

        // Normalize.
        normalize(w);

        // Reflect qth column.
        x[p][q] = -alpha;
        std::fill(
            x.transpose()[q].range(p + 1, x.size0()).begin(),
            x.transpose()[q].range(p + 1, x.size0()).end(),
            value_type());

        // Reflect remaining columns. For j < q, assume no effect.
        for (int j = q + 1; j < x.size1(); j++) {
            reflect(w, x.transpose()[j].range(p, x.size0()));
        }

        // Accumulate.
        if (!y.empty()) {

            // Sanity check.
            if (x.size0() != y.size0()) {
                throw std::invalid_argument(__PRETTY_FUNCTION__);
            }

            // Reflect columns.
            for (int j = 0; j < y.size1(); j++) {
                reflect(w, y.transpose()[j].range(p, x.size0()));
            }
        }
    }

    /**
     * @brief Householder step.
     *
     * @param[in] p
     * Target index @f$ p @f$.
     *
     * @param[in] q
     * Target index @f$ q @f$.
     *
     * @param[inout] x
     * Matrix @f$ \mathbf{X} @f$.
     *
     * @param[inout] y
     * Matrix @f$ \mathbf{Y} @f$. _Optional_.
     *
     * @note
     * This method calls `householderl()` with the appropriate
     * view transform.
     */
    __attribute__((always_inline))
    static void householderr(
                int p,
                int q,
                dense_matrix_view<value_type*> x,
                dense_matrix_view<value_type*> y = {})
    {
        // Conjugate.
        for (int i = 0; i < x.size0(); i++) 
        for (int j = 0; j < x.size1(); j++) {
            x[i][j] = conj(x[i][j]);
        }
        for (int i = 0; i < y.size0(); i++) 
        for (int j = 0; j < y.size1(); j++) {
            y[i][j] = conj(y[i][j]);
        }

        // Delegate.
        householderl(
                q, p, 
                x.transpose(),
                y.transpose());

        // Conjugate.
        for (int i = 0; i < x.size0(); i++) 
        for (int j = 0; j < x.size1(); j++) {
            x[i][j] = conj(x[i][j]);
        }
        for (int i = 0; i < y.size0(); i++) 
        for (int j = 0; j < y.size1(); j++) {
            y[i][j] = conj(y[i][j]);
        }
    }

    /**@}*/

public:

    /**
     * @name Unitary-triangular decomposition
     */
    /**@{*/

    /**
     * @brief Decomp @f$ \mathbf{X} \to \mathbf{Q}\mathbf{R} @f$.
     *
     * @param[inout] x
     * Matrix @f$ \mathbf{X} \to \mathbf{R} @f$.
     *
     * @param[out] q
     * Matrix @f$ \mathbf{Q} @f$. _Optional_.
     *
     * @throw std::invalid_argument
     * If `x` is empty. If 
     * `q.size0() != x.size0()` or 
     * `q.size1() != x.size0()`, unless `q` is empty.
     */
    static void decompqr(
                dense_matrix_view<value_type*> x,
                dense_matrix_view<value_type*> q = {})
    {
        // Ensure valid.
        if (x.empty()) {
            throw std::invalid_argument(__PRETTY_FUNCTION__);
        }

        // Load identity.
        if (!q.empty()) {
            // Ensure square.
            if (q.size0() != x.size0() ||
                q.size1() != x.size0()) {
                throw std::invalid_argument(__PRETTY_FUNCTION__);
            }
            load_identity(q);
        }

        // Householder reduction.
        for (int k = 0; k < x.diag().size() - 1; k++) {
            householderl(k, k, x, q);
        }

        // Adjoint.
        if (!q.empty()) {
            adjoint({}, q);
        }
    }

    /**
     * @brief Decomp @f$ \mathbf{X} \to \mathbf{Q}\mathbf{L} @f$.
     *
     * @param[inout] x
     * Matrix @f$ \mathbf{X} \to \mathbf{L} @f$.
     *
     * @param[out] q
     * Matrix @f$ \mathbf{Q} @f$. _Optional_.
     *
     * @note
     * This method calls `decompqr()` with the appropriate
     * view transform.
     */
    static void decompql(
                dense_matrix_view<value_type*> x,
                dense_matrix_view<value_type*> q = {})
    {
        // Delegate.
        decompqr(
            x.block(x.size0(), x.size1(), 0, 0),
            q.block(q.size0(), q.size1(), 0, 0));
    }

    /**
     * @brief Decomp @f$ \mathbf{X} \to \mathbf{R}\mathbf{Q} @f$.
     *
     * @param[inout] x
     * Matrix @f$ \mathbf{X} \to \mathbf{R} @f$.
     *
     * @param[out] q
     * Matrix @f$ \mathbf{Q} @f$. _Optional_.
     *
     * @note
     * This method calls `decompqr()` with the appropriate
     * view transform.
     */
    static void decomprq(
                dense_matrix_view<value_type*> x,
                dense_matrix_view<value_type*> q = {})
    {
        // Conjugate.
        for (int i = 0; i < x.size0(); i++) 
        for (int j = 0; j < x.size1(); j++) {
            x[i][j] = conj(x[i][j]);
        }
        for (int i = 0; i < q.size0(); i++) 
        for (int j = 0; j < q.size1(); j++) {
            q[i][j] = conj(q[i][j]);
        }

        // Delegate.
        decompqr(
            x.block(x.size0(), x.size1(), 0, 0).transpose(),
            q.block(q.size0(), q.size1(), 0, 0).transpose());

        // Conjugate.
        for (int i = 0; i < x.size0(); i++) 
        for (int j = 0; j < x.size1(); j++) {
            x[i][j] = conj(x[i][j]);
        }
        for (int i = 0; i < q.size0(); i++) 
        for (int j = 0; j < q.size1(); j++) {
            q[i][j] = conj(q[i][j]);
        }
    }

    /**
     * @brief Decomp @f$ \mathbf{X} \to \mathbf{L}\mathbf{Q} @f$.
     *
     * @param[inout] x
     * Matrix @f$ \mathbf{X} \to \mathbf{L} @f$.
     *
     * @param[out] q
     * Matrix @f$ \mathbf{Q} @f$. _Optional_.
     *
     * @note
     * This method calls `decompqr()` with the appropriate
     * view transform.
     */
    static void decomplq(
                dense_matrix_view<value_type*> x,
                dense_matrix_view<value_type*> q = {})
    {
        // Conjugate.
        for (int i = 0; i < x.size0(); i++) 
        for (int j = 0; j < x.size1(); j++) {
            x[i][j] = conj(x[i][j]);
        }
        for (int i = 0; i < q.size0(); i++) 
        for (int j = 0; j < q.size1(); j++) {
            q[i][j] = conj(q[i][j]);
        }

        // Delegate.
        decompqr(
            x.transpose(),
            q.transpose());

        // Conjugate.
        for (int i = 0; i < x.size0(); i++) 
        for (int j = 0; j < x.size1(); j++) {
            x[i][j] = conj(x[i][j]);
        }
        for (int i = 0; i < q.size0(); i++) 
        for (int j = 0; j < q.size1(); j++) {
            q[i][j] = conj(q[i][j]);
        }
    }

    /**@}*/
};

/**@}*/

} // namespace pr

#endif // #ifndef PREFORM_DENSE_BLAS_HPP
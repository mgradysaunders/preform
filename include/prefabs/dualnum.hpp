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
#if !(__cplusplus >= 201703L)
#error "prefabs/dualnum.hpp requires >=C++17"
#endif // #if !(__cplusplus >= 201703L)
#endif // #if !DOXYGEN
#pragma once
#ifndef PREFABS_DUALNUM_HPP
#define PREFABS_DUALNUM_HPP

// for std::basic_istream
#include <istream>

// for std::basic_ostream
#include <ostream>

#include <prefabs/math.hpp>

namespace pr {

/**
 * @defgroup dualnum Dual numbers
 *
 * `<prefabs/dualnum.hpp>`
 *
 * __C++ version__: >=C++17
 */
/**@{*/

#if !DOXYGEN

template <typename T>
class dualnum;

template <typename T>
struct is_dualnum : std::false_type
{
};

template <typename T>
struct is_dualnum<dualnum<T>> : std::true_type
{
};

template <typename T>
struct is_dualnum_complex : std::false_type
{
};

template <typename T>
struct is_dualnum_complex<dualnum<std::complex<T>>> : std::true_type
{
};

template <typename T>
struct is_dualnum_param : 
            std::integral_constant<bool,
            std::is_arithmetic<T>::value ||
            is_complex<T>::value>
{
};

#endif // #if !DOXYGEN

/**
 * @brief Dual number.
 */
template <typename T>
class dualnum
{
public:

    // Sanity check.
    static_assert(
        is_dualnum_param<T>::value, 
        "T must be arithmetic or complex");

    /**
     * @brief Value type.
     */
    typedef T value_type;

public:

    /**
     * @name Constructors
     */
    /**@{*/

    /**
     * @brief Default constructor.
     */
    constexpr dualnum() = default;

    /**
     * @brief Default copy constructor.
     */
    constexpr dualnum(const dualnum&) = default;

    /**
     * @brief Default move constructor.
     */
    constexpr dualnum(dualnum&&) = default;

    /**
     * @brief Constructor.
     */
    constexpr dualnum(T a, T b = T()) : a_(a), b_(b)
    {
    }

    /**@}*/

public:

    /**
     * @name Assign operators
     */
    /**@{*/

    /**
     * @brief Default copy assign.
     */
    constexpr dualnum& operator=(const dualnum&) = default;

    /**
     * @brief Default move assign.
     */
    constexpr dualnum& operator=(dualnum&&) = default;

    /**
     * @brief Real assign.
     */
    constexpr dualnum& operator=(T a)
    {
        return *this = dualnum(a);
    }

    /**@}*/

public:

    /**
     * @name Accessors
     */
    /**@{*/

    /**
     * @brief Get real part.
     */
    constexpr const T& real() const
    {
        return a_;
    }

    /**
     * @brief Get dual part.
     */
    constexpr const T& dual() const
    {
        return b_;
    }

    /**
     * @brief Set real part, return previous real part.
     */
    constexpr T real(T val)
    { 
        T a = a_; a_ = val; return a; 
    }

    /**
     * @brief Set dual part, return previous dual part.
     */
    constexpr T dual(T val)
    { 
        T b = b_; b_ = val; return b; 
    }

    /**@}*/

private:

    /**
     * @brief Real part.
     */
    T a_ = T();

    /**
     * @brief Dual part.
     */
    T b_ = T();

public:

    /**
     * @name Stream operators
     */
    /**@{*/

    /**
     * @brief Parse from `std::basic_istream`.
     *
     * Format is `(a,b)`. Sets `std::ios_base::failbit` on error.
     */
    template <typename C, typename Ctraits>
    friend
    inline std::basic_istream<C, Ctraits>& operator>>(
           std::basic_istream<C, Ctraits>& is, dualnum& x)
    {
        C ch;
        if (!(is >> ch) ||
            !Ctraits::eq(ch, 
             Ctraits::to_char_type('('))) {
            is.setstate(std::ios_base::failbit);
            return is;
        }
        is >> x.a_;
        if (!(is >> ch) ||
            !Ctraits::eq(ch, 
             Ctraits::to_char_type(','))) {
            is.setstate(std::ios_base::failbit);
            return is;
        }
        is >> x.b_;
        if (!(is >> ch) ||
            !Ctraits::eq(ch, 
             Ctraits::to_char_type(')'))) {
            is.setstate(std::ios_base::failbit);
            return is;
        }
        return is;
    }

    /**
     * @brief Write into `std::basic_ostream`.
     *
     * Format is `(a,b)`.
     */
    template <typename C, typename Ctraits>
    friend
    inline std::basic_ostream<C, Ctraits>& operator<<(
           std::basic_ostream<C, Ctraits>& os, const dualnum& x)
    {
        os << '(';
        os << x.a_ << ',';
        os << x.b_ << ')';
        return os;
    }

    /**@}*/
};

/**
 * @name Unary operators (dualnum)
 */
/**@{*/

/**
 * @brief Distribute `operator+`.
 */
template <typename T>
__attribute__((always_inline))
constexpr dualnum<T> operator+(const dualnum<T>& x)
{
    return {+x.real(), +x.dual()};
}

/**
 * @brief Distribute `operator-`.
 */
template <typename T>
__attribute__((always_inline))
constexpr dualnum<T> operator-(const dualnum<T>& x)
{
    return {-x.real(), -x.dual()};
}

/**@}*/

/**
 * @name Binary operators (dualnum/dualnum)
 */
/**@{*/

/**
 * @brief Distribute `operator+`.
 *
 * @f[
 *      (\real(x_0) + \varepsilon \dual(x_0)) + 
 *      (\real(x_1) + \varepsilon \dual(x_1)) = 
 *      (\real(x_0) + \real(x_1)) + \varepsilon 
 *      (\dual(x_0) + \dual(x_1))
 * @f]
 */
template <typename T, typename U>
__attribute__((always_inline))
constexpr dualnum<decltype(T() + U())> operator+(
                    const dualnum<T>& x0, const dualnum<U>& x1)
{
    return {x0.real() + x1.real(), x0.dual() + x1.dual()};
}

/**
 * @brief Distribute `operator-`.
 *
 * @f[
 *      (\real(x_0) + \varepsilon \dual(x_0)) -
 *      (\real(x_1) + \varepsilon \dual(x_1)) = 
 *      (\real(x_0) - \real(x_1)) + \varepsilon 
 *      (\dual(x_0) - \dual(x_1))
 * @f]
 */
template <typename T, typename U>
__attribute__((always_inline))
constexpr dualnum<decltype(T() - U())> operator-(
                    const dualnum<T>& x0, const dualnum<U>& x1)
{
    return {x0.real() - x1.real(), x0.dual() - x1.dual()};
}

/**
 * @brief Distribute `operator*`.
 *
 * @f[
 *      (\real(x_0) + \varepsilon \dual(x_0))
 *      (\real(x_1) + \varepsilon \dual(x_1)) = 
 *       \real(x_0) \real(x_1) + \varepsilon 
 *      (\real(x_0) \dual(x_1) + \dual(x_0) \real(x_1))
 * @f]
 */
template <typename T, typename U>
__attribute__((always_inline))
constexpr dualnum<decltype(T() * U())> operator*(
                    const dualnum<T>& x0, const dualnum<U>& x1)
{
    return {
        x0.real() * x1.real(),
        x0.real() * x1.dual() + x0.dual() * x1.real()
    };
}

/**
 * @brief Distribute `operator*`, inverting right hand side.
 *
 * @f[
 *      (\real(x_0) + \varepsilon \dual(x_0)) 
 *      (\real(x_1) + \varepsilon \dual(x_1))^{-1} = 
 *       \real(x_0)\real(x_1)^{-1} + \varepsilon 
 *      (\dual(x_0)\real(x_1) - \real(x_0)\dual(x_1)) \real(x_1)^{-2}
 * @f]
 */
template <typename T, typename U>
__attribute__((always_inline))
constexpr dualnum<decltype(T() / U())> operator/(
                    const dualnum<T>& x0, const dualnum<U>& x1)
{
    return {
        x0.real() / x1.real(),
        (x0.dual() * x1.real() - x0.real() * x1.dual()) / 
        (x1.real() * x1.real())
    };
}

/**@}*/

/**
 * @name Binary operators (dualnum/num)
 */
/**@{*/

/**
 * @brief Distribute `operator+`.
 *
 * @f[
 *      (\real(x_0) + \varepsilon \dual(x_0)) + x_1 = 
 *      (\real(x_0) + x_1) + \varepsilon \dual(x_0)
 * @f]
 */
template <typename T, typename U>
__attribute__((always_inline))
constexpr std::enable_if_t<
                        is_dualnum_param<U>::value,
                        dualnum<decltype(T() + U())>> operator+(
                            const dualnum<T>& x0, const U& x1)
{
    return {x0.real() + x1, x0.dual()};
}

/**
 * @brief Distribute `operator-`.
 *
 * @f[
 *      (\real(x_0) + \varepsilon \dual(x_0)) - x_1 = 
 *      (\real(x_0) - x_1) + \varepsilon \dual(x_0)
 * @f]
 */
template <typename T, typename U>
__attribute__((always_inline))
constexpr std::enable_if_t<
                        is_dualnum_param<U>::value,
                        dualnum<decltype(T() - U())>> operator-(
                            const dualnum<T>& x0, const U& x1)
{
    return {x0.real() - x1, x0.dual()};
}

/**
 * @brief Distribute `operator*`.
 *
 * @f[
 *      (\real(x_0) + \varepsilon \dual(x_0)) x_1 = 
 *       \real(x_0) x_1 + \varepsilon \dual(x_0) x_1
 * @f]
 */
template <typename T, typename U>
__attribute__((always_inline))
constexpr std::enable_if_t<
                        is_dualnum_param<U>::value,
                        dualnum<decltype(T() * U())>> operator*(
                            const dualnum<T>& x0, const U& x1)
{
    return {x0.real() * x1, x0.dual() * x1};
}

/**
 * @brief Distribute `operator*`, inverting right hand side.
 *
 * @f[
 *      (\real(x_0) + \varepsilon \dual(x_0)) x_1^{-1} = 
 *       \real(x_0) x_1^{-1} + \varepsilon \dual(x_0) x_1^{-1}
 * @f]
 */
template <typename T, typename U>
__attribute__((always_inline))
constexpr std::enable_if_t<
                        is_dualnum_param<U>::value,
                        dualnum<decltype(T() / U())>> operator/(
                            const dualnum<T>& x0, const U& x1)
{
    return {x0.real() / x1, x0.dual() / x1};
}

/**@}*/

/**
 * @name Binary operators (num/dualnum)
 */
/**@{*/

/**
 * @brief Distribute `operator+`.
 *
 * @f[
 *       x_0 + (\real(x_1) + \varepsilon \dual(x_1)) = 
 *      (x_0 + \real(x_1)) + \varepsilon \dual(x_1)
 * @f]
 */
template <typename T, typename U>
__attribute__((always_inline))
constexpr std::enable_if_t<
                        is_dualnum_param<T>::value,
                        dualnum<decltype(T() + U())>> operator+(
                            const T& x0, const dualnum<U>& x1)
{
    return {x0 + x1.real(), x1.dual()};
}

/**
 * @brief Distribute `operator-`.
 *
 * @f[
 *       x_0 - (\real(x_1) + \varepsilon \dual(x_1))  = 
 *      (x_0 - \real(x_1)) - \varepsilon \dual(x_1)
 * @f]
 */
template <typename T, typename U>
__attribute__((always_inline))
constexpr std::enable_if_t<
                        is_dualnum_param<T>::value,
                        dualnum<decltype(T() - U())>> operator-(
                            const T& x0, const dualnum<U>& x1)
{
    return {x0 - x1.real(), -x1.dual()};
}

/**
 * @brief Distribute `operator*`.
 *
 * @f[
 *      x_0 (\real(x_1) + \varepsilon \dual(x_1)) = 
 *      x_0  \real(x_1) + \varepsilon x_0 \dual(x_1)
 * @f]
 */
template <typename T, typename U>
__attribute__((always_inline))
constexpr std::enable_if_t<
                        is_dualnum_param<T>::value,
                        dualnum<decltype(T() * U())>> operator*(
                            const T& x0, const dualnum<U>& x1)
{
    return {x0 * x1.real(), x0 * x1.dual()};
}

/**
 * @brief Distribute `operator*`, inverting right hand side.
 *
 * @f[
 *      x_0 (\real(x_1) + \varepsilon \dual(x_1))^{-1} = 
 *      x_0  \real(x_1)^{-1} - \varepsilon x_0 \dual(x_1) \real(x_1)^{-2}
 * @f]
 */
template <typename T, typename U>
__attribute__((always_inline))
constexpr std::enable_if_t<
                        is_dualnum_param<T>::value,
                        dualnum<decltype(T() / U())>> operator/(
                            const T& x0, const dualnum<U>& x1)
{
    return {
        x0 / x1.real(),
        -x0 * x1.dual() / 
        (x1.real() * x1.real())
    };
}

/**@}*/

/**
 * @name Binary operators (dualnum/any)
 */
/**@{*/

/**
 * @brief Generic `operator+=`.
 */
template <typename T, typename U>
__attribute__((always_inline))
constexpr dualnum<T>& operator+=(dualnum<T>& x, const U& any)
{
    return x = x + any;
}

/**
 * @brief Generic `operator-=`.
 */
template <typename T, typename U>
__attribute__((always_inline))
constexpr dualnum<T>& operator-=(dualnum<T>& x, const U& any)
{
    return x = x - any;
}

/**
 * @brief Generic `operator*=`.
 */
template <typename T, typename U>
__attribute__((always_inline))
constexpr dualnum<T>& operator*=(dualnum<T>& x, const U& any)
{
    return x = x * any;
}

/**
 * @brief Generic `operator/=`.
 */
template <typename T, typename U>
__attribute__((always_inline))
constexpr dualnum<T>& operator/=(dualnum<T>& x, const U& any)
{
    return x = x / any;
}

/**@}*/

/**
 * @name Comparison operators (dualnum)
 */
/**@{*/

/**
 * @brief Compare `operator==`.
 */
template <typename T, typename U>
__attribute__((always_inline))
constexpr bool operator==(const dualnum<T>& x0, const dualnum<U>& x1)
{
    return x0.real() == x1.real() && x0.dual() == x1.dual();
}

/**
 * @brief Compare `operator==`.
 */
template <typename T, typename U>
__attribute__((always_inline))
constexpr std::enable_if_t<
                        is_dualnum_param<U>::value, bool> operator==(
                        const dualnum<T>& x0, const U& x1)
{
    return x0.real() == x1 && x0.dual() == T();
}

/**
 * @brief Compare `operator==`.
 */
template <typename T, typename U>
__attribute__((always_inline))
constexpr std::enable_if_t<
                        is_dualnum_param<T>::value, bool> operator==(
                        const T& x0, const dualnum<U>& x1)
{
    return x0 == x1.real() && U() == x1.dual();
}

/**
 * @brief Compare `operator!=`.
 */
template <typename T, typename U>
__attribute__((always_inline))
constexpr bool operator!=(const dualnum<T>& x0, const dualnum<U>& x1)
{
    return !(x0 == x1);
}

/**
 * @brief Compare `operator!=`.
 */
template <typename T, typename U>
__attribute__((always_inline))
constexpr std::enable_if_t<
                        is_dualnum_param<U>::value, bool> operator!=(
                        const dualnum<T>& x0, const U& x1)
{
    return !(x0 == x1);
}

/**
 * @brief Compare `operator!=`.
 */
template <typename T, typename U>
__attribute__((always_inline))
constexpr std::enable_if_t<
                        is_dualnum_param<T>::value, bool> operator!=(
                        const T& x0, const dualnum<U>& x1)
{
    return !(x0 == x1);
}

/**@}*/

/**
 * @name Dual number accessors
 */
/**@{*/

/**
 * @brief Dual number real part.
 */
template <typename T>
__attribute__((always_inline))
constexpr T real(const dualnum<T>& x)
{
    return x.real();
}

/**
 * @brief Dual number dual part.
 */
template <typename T> 
__attribute__((always_inline))
constexpr T dual(const dualnum<T>& x)
{
    return x.dual();
}

/**
 * @brief Dual number conjugate.
 */
template <typename T> 
__attribute__((always_inline))
constexpr dualnum<T> conj(const dualnum<T>& x)
{
    return {+x.real(), -x.dual()};
}

/**
 * @brief Dual number norm.
 */
template <typename T> 
__attribute__((always_inline))
constexpr T norm(const dualnum<T>& x)
{
    return x.real() * x.real();
}

/**@}*/

/**@}*/

} // namespace pr

#if !DOXYGEN
#include "dualnum.inl"
#endif // #if !DOXYGEN

#endif // #ifndef PREFABS_DUALNUM_HPP

#ifndef Magnum_Math_ConstantMath_hpp
#define Magnum_Math_ConstantMath_hpp
#include <Corrade/Utility/Macros.h>
#include <Corrade/Utility/StlMath.h>
#include <cstdint>
#include <limits>
#include <type_traits>

namespace Magnum {
namespace ConstantMath {

#if defined CORRADE_CONSTEVAL && CORRADE_CXX_STANDARD >= 201402L && !defined CORRADE_MSVC2015_COMPATIBILITY

namespace Implementation {

template<typename T> using FloatTypeFor = typename std::conditional<std::is_floating_point<T>::value, T, double>::type;

/* |x| mod |y| for finite positive x, y by binary long division. Exact: each
   subtraction has scaledY <= x < 2*scaledY, so Sterbenz's lemma applies. The
   loop tests scaledY <= x - scaledY because 2*scaledY may overflow, which
   isn't a constant expression. */
template<typename T> constexpr T fmodPositive(T x, T y)
{
    T scaledY = y;
    while (scaledY <= x - scaledY)
        scaledY *= T(2);
    for (;;)
    {
        if (x >= scaledY)
            x -= scaledY;
        if (scaledY == y)
            return x;
        scaledY *= T(0.5);
    }
}

template<typename T> struct TwoProduct { T hi, lo; };

/* a*b == hi + lo exactly (Dekker). The Veltkamp split constant is
   2^ceil(p/2) + 1 for a p-bit significand. */
template<typename T> constexpr TwoProduct<T> twoProduct(T a, T b)
{
    T splitter = T(1);
    for (int i = 0; i != (std::numeric_limits<T>::digits + 1)/2; ++i)
        splitter *= T(2);
    splitter += T(1);
    const T aSplit = splitter*a, aHigh = aSplit - (aSplit - a), aLow = a - aHigh;
    const T bSplit = splitter*b, bHigh = bSplit - (bSplit - b), bLow = b - bHigh;
    const T product = a*b;
    return {product, ((aHigh*bHigh - product) + aHigh*bLow + aLow*bHigh) + aLow*bLow};
}

/* m > a*b, exactly. m - hi is exact by Sterbenz's lemma, as a*b is close
   to m. */
template<typename T> constexpr bool greaterThanProduct(T m, T a, T b)
{
    const TwoProduct<T> product = twoProduct(a, b);
    return m - product.hi > product.lo;
}

/* Correctly rounded sqrt(x) for finite x > 0 */
template<typename T> constexpr T sqrtPositive(T x)
{
    /* x = reduced*4^e with reduced in [1, 4), scaling by powers of two is
       exact */
    const T big = T(18446744073709551616.0);
    const T bigRoot = T(4294967296.0);
    T reduced = x, scale = T(1);
    while (reduced >= big)
    {
        reduced /= big;
        scale *= bigRoot;
    }
    while (reduced < T(1))
    {
        reduced *= big;
        scale /= bigRoot;
    }
    while (reduced >= T(4))
    {
        reduced /= T(4);
        scale *= T(2);
    }

    T root = (T(1) + reduced)/T(2);
    for (;;)
    {
        const T next = (root + reduced/root)/T(2);
        if (!(next < root))
            break;
        root = next;
    }

    /* Tuckerman's test: root is sqrt(reduced) rounded to nearest iff
       root*(root - down) < reduced <= root*(root + up), with up and down the
       distances to the neighbouring floats */
    const T eps = std::numeric_limits<T>::epsilon();
    for (;;)
    {
        const T up = root < T(2) ? eps : T(2)*eps;
        const T down = root > T(2) ? T(2)*eps : root > T(1) ? eps : eps/T(2);
        if (greaterThanProduct(reduced, root, root + up))
            root += up;
        else if (!greaterThanProduct(reduced, root, root - down))
            root -= down;
        else
            break;
    }

    return root*scale;
}

#if !defined CORRADE_TARGET_GCC && !defined CORRADE_TARGET_CLANG
template<typename T> struct BitsFor;
template<> struct BitsFor<float> { using type = std::uint32_t; };
template<> struct BitsFor<double> { using type = std::uint64_t; };
/* MSVC's long double is double */
template<> struct BitsFor<long double> { using type = std::uint64_t; };
#endif

}

#define MAGNUM_HAS_CONSTEXPR_SQRT
#define MAGNUM_CONSTEXPR_SQRT constexpr
template<typename U> CORRADE_ALWAYS_INLINE constexpr Implementation::FloatTypeFor<U> sqrt(U value)
{
    using T = Implementation::FloatTypeFor<U>;
    const T floatValue = T(value);
    /* MSVC constant-evaluates < and <= with a NaN operand as true, so NaN is
       tested with != before any of those */
    if (floatValue != floatValue || floatValue == T(0) || floatValue == std::numeric_limits<T>::infinity())
        return floatValue;
    /* Domain error, which std::sqrt() makes a non-constant expression */
    if (floatValue < T(0))
        return std::sqrt(floatValue);
    return Implementation::sqrtPositive(floatValue);
}

#define MAGNUM_HAS_CONSTEXPR_FLOOR
#define MAGNUM_CONSTEXPR_FLOOR constexpr
template<typename U> CORRADE_ALWAYS_INLINE constexpr Implementation::FloatTypeFor<U> floor(U value)
{
    using T = Implementation::FloatTypeFor<U>;
    const T floatValue = T(value), absValue = floatValue < T(0) ? -floatValue : floatValue;
    /* ±0, infinities, NaN and all values from 2^(p - 1) up, which have no
       fractional bits, map to themselves */
    if (floatValue != floatValue || floatValue == T(0) || absValue >= T(1)/std::numeric_limits<T>::epsilon())
        return floatValue;
    /* Exact, fmod() is exact and the difference is an integer <= absValue */
    const T trunc = absValue - Implementation::fmodPositive(absValue, T(1));
    return floatValue > T(0) ? trunc : trunc == absValue ? -trunc : -trunc - T(1);
}

#define MAGNUM_HAS_CONSTEXPR_CEIL
#define MAGNUM_CONSTEXPR_CEIL constexpr
template<typename U> CORRADE_ALWAYS_INLINE constexpr Implementation::FloatTypeFor<U> ceil(U value)
{
    using T = Implementation::FloatTypeFor<U>;
    const T floatValue = T(value), absValue = floatValue < T(0) ? -floatValue : floatValue;
    if (floatValue != floatValue || floatValue == T(0) || absValue >= T(1)/std::numeric_limits<T>::epsilon())
        return floatValue;
    const T trunc = absValue - Implementation::fmodPositive(absValue, T(1));
    return floatValue < T(0) ? -trunc : trunc == absValue ? trunc : trunc + T(1);
}

#define MAGNUM_HAS_CONSTEXPR_COPYSIGN
#define MAGNUM_CONSTEXPR_COPYSIGN constexpr
#if defined CORRADE_TARGET_GCC || defined CORRADE_TARGET_CLANG
CORRADE_ALWAYS_INLINE constexpr float copysign(float x, float y)
{
    return __builtin_copysignf(x, y);
}
CORRADE_ALWAYS_INLINE constexpr double copysign(double x, double y)
{
    return __builtin_copysign(x, y);
}
CORRADE_ALWAYS_INLINE constexpr long double copysign(long double x, long double y)
{
    return __builtin_copysignl(x, y);
}
template<typename U, typename std::enable_if<std::is_integral<U>::value, int>::type = 0> CORRADE_ALWAYS_INLINE constexpr double copysign(U x, U y)
{
    return __builtin_copysign(x, y);
}
#else
template<typename U> CORRADE_ALWAYS_INLINE constexpr Implementation::FloatTypeFor<U> copysign(U xValue, U yValue)
{
    using T = Implementation::FloatTypeFor<U>;
    using Bits = typename Implementation::BitsFor<T>::type;
    const Bits signMask = Bits(1) << (sizeof(Bits)*8 - 1);
    /* MSVC 19.29 and 19.31 crash when a __builtin_bit_cast() operand is a
       temporary */
    const T floatX = T(xValue), floatY = T(yValue);
    const Bits xBits = __builtin_bit_cast(Bits, floatX), yBits = __builtin_bit_cast(Bits, floatY);
    const Bits outBits = (xBits & ~signMask)|(yBits & signMask);
    return __builtin_bit_cast(T, outBits);
}
#endif

#define MAGNUM_HAS_CONSTEXPR_FMOD
#define MAGNUM_CONSTEXPR_FMOD constexpr
template<typename U> CORRADE_ALWAYS_INLINE constexpr Implementation::FloatTypeFor<U> fmod(U xValue, U yValue)
{
    using T = Implementation::FloatTypeFor<U>;
    const T floatX = T(xValue), floatY = T(yValue), inf = std::numeric_limits<T>::infinity();
    if (floatX != floatX)
        return floatX;
    if (floatY != floatY)
        return floatY;
    /* Domain error, which std::fmod() makes a non-constant expression */
    if (floatX == inf || floatX == -inf || floatY == T(0))
        return std::fmod(floatX, floatY);
    if (floatX == T(0) || floatY == inf || floatY == -inf)
        return floatX;
    const T remainder = Implementation::fmodPositive(floatX < T(0) ? -floatX : floatX, floatY < T(0) ? -floatY : floatY);
    return floatX < T(0) ? -remainder : remainder;
}

#else
#define MAGNUM_CONSTEXPR_SQRT
#define MAGNUM_CONSTEXPR_FLOOR
#define MAGNUM_CONSTEXPR_CEIL
#define MAGNUM_CONSTEXPR_COPYSIGN
#define MAGNUM_CONSTEXPR_FMOD
#endif

/* isNan() uses __builtin_isnan(), which GCC 4.8.1+ and Clang 6+
   constant-evaluate even in C++11, and MSVC since 19.52. MSVC 19.51 crashes
   constant-evaluating it. Other compilers use std::isnan(), constexpr with a
   C++23 constexpr <cmath>. The vector overload needs C++14 on top. */
#if defined CORRADE_TARGET_GCC || defined CORRADE_TARGET_CLANG || \
    (defined CORRADE_TARGET_MSVC && _MSC_VER >= 1952) || \
    (defined __cpp_lib_constexpr_cmath && __cpp_lib_constexpr_cmath >= 202202L)
#define MAGNUM_HAS_CONSTEXPR_ISNAN
#endif

} // namespace ConstantMath
} // namespace Magnum

#endif

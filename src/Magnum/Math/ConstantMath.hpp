#pragma once
#include <Corrade/Utility/Macros.h>
#include <cstdint>
#include <type_traits>

namespace Magnum {
namespace ConstantMath {

#ifdef CORRADE_CONSTEVAL

namespace Implementation {
template<typename T> struct IntTypeFor_;
template<> struct IntTypeFor_<float> { using type = std::int32_t; };
template<> struct IntTypeFor_<double> { using type = std::int64_t; };
template<> struct IntTypeFor_<long double> { using type = std::int64_t; };
template<typename T> using IntTypeFor = typename IntTypeFor_<T>::type;
}

#define MAGNUM_HAS_CONSTEXPR_SQRT
#define MAGNUM_CONSTEXPR_SQRT constexpr
template<typename U> CORRADE_ALWAYS_INLINE constexpr auto sqrt(U x)
{
    using T = typename std::conditional<std::is_floating_point<U>::value, U, double>::type;
    T x0(x);
    if (x0 < T(0))
        return std::sqrt(x0);
    auto cur = x0, prev = T(0);
    while (cur != prev)
    {
        prev = cur;
        cur = T(0.5) * (cur + x0 / cur);
    }
    return cur;
}

#define MAGNUM_HAS_CONSTEXPR_FLOOR
#define MAGNUM_CONSTEXPR_FLOOR constexpr
template<typename T, typename std::enable_if<std::is_signed<T>::value>::type** = nullptr> CORRADE_ALWAYS_INLINE constexpr T floor(T x)
{
    using int_ = Implementation::IntTypeFor<T>;
    const auto x0 = int_(x);
    return x0 - int_{1} * (x < T{0} && x != T(x0));
}

#define MAGNUM_HAS_CONSTEXPR_CEIL
#define MAGNUM_CONSTEXPR_CEIL constexpr
template<typename T, typename std::enable_if<std::is_floating_point<T>::value>::type** = nullptr> CORRADE_ALWAYS_INLINE constexpr T ceil(T x)
{
    using int_ = Implementation::IntTypeFor<T>;
    const auto x0 = int_(x);
    return x0 + int_{1} * (x > x0);
}

#define MAGNUM_HAS_CONSTEXPR_COPYSIGN
#define MAGNUM_CONSTEXPR_COPYSIGN constexpr
template<typename T, typename std::enable_if<std::is_floating_point<T>::value>::type** = nullptr> CORRADE_ALWAYS_INLINE constexpr T copysign(T x, T y)
{
    return (x < T{0} ? -x : x) * (y < T{0} ? T{-1} : T{1});
}

#define MAGNUM_HAS_CONSTEXPR_FMOD
#define MAGNUM_CONSTEXPR_FMOD constexpr
template<typename T, typename std::enable_if<std::is_floating_point<T>::value>::type** = nullptr> CORRADE_ALWAYS_INLINE constexpr T fmod(T x, T y)
{
    const T xsign = x < T{0} ? T{-1} : T{1},
            absx = x < T() ? -x : x, absy = y < T{0} ? -y : y,
            div = x/y, absdiv = div < T{0} ? -div : div;
    using int_ = Implementation::IntTypeFor<T>;
    return (xsign) * (absx - T(int_(absdiv)) * absy);
}

#else
#define MAGNUM_CONSTEXPR_SQRT
#define MAGNUM_CONSTEXPR_FLOOR
#define MAGNUM_CONSTEXPR_CEIL
#define MAGNUM_CONSTEXPR_COPYSIGN
#define MAGNUM_CONSTEXPR_FMOD
#endif

} // namespace ConstantMath
} // namespace Magnum

/*
    This file is part of Magnum.

    Copyright © 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019,
                2020, 2021, 2022, 2023, 2024, 2025, 2026
              Vladimír Vondruš <mosra@centrum.cz>
    Copyright © 2026 Stanislaw Halik <sthalik@misaki.pl>

    Permission is hereby granted, free of charge, to any person obtaining a
    copy of this software and associated documentation files (the "Software"),
    to deal in the Software without restriction, including without limitation
    the rights to use, copy, modify, merge, publish, distribute, sublicense,
    and/or sell copies of the Software, and to permit persons to whom the
    Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included
    in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
    THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
    DEALINGS IN THE SOFTWARE.
*/

#include <cfloat>
#include <cmath>
#include <cstdint>
#include <iomanip>
#include <limits>
#include <sstream>
#include <type_traits>
#include <Corrade/Containers/ArrayView.h>
#include <Corrade/TestSuite/Tester.h>
#include <Corrade/Utility/DebugStl.h>
#if defined CORRADE_TARGET_MINGW && defined CORRADE_TARGET_SSE2
#include <Corrade/Utility/IntrinsicsSse2.h>
#endif

#include "Magnum/Math/Angle.h"
#include "Magnum/Math/ConstantMath.hpp"
#include "Magnum/Math/Functions.h"
#include "Magnum/Math/TypeTraits.h"
#include "Magnum/Math/Vector4.h"

/* The run-time tests call the functions with arguments not known at compile
   time. The results are exact only if every floating-point operation runs as
   written, which constant evaluation guarantees regardless of flags. */
#if defined __FAST_MATH__ || defined __ASSOCIATIVE_MATH__ || defined _M_FP_FAST
#define RUNTIME_SKIP_REASON "Value-changing floating-point optimizations are enabled."
#elif defined CORRADE_TARGET_GCC && !defined CORRADE_TARGET_CLANG && (defined __FP_FAST_FMA || defined __FP_FAST_FMAF)
#define RUNTIME_SKIP_REASON "GCC contracts multiplications and additions into FMA."
#elif defined FLT_EVAL_METHOD && FLT_EVAL_METHOD != 0
#define RUNTIME_SKIP_REASON "Floating-point operations are evaluated with excess precision."
#endif

namespace Magnum { namespace Math { namespace Test { namespace {

struct ConstantMathTest: TestSuite::Tester {
    explicit ConstantMathTest();

    template<class T> void sqrt();
    template<class T> void sqrtRuntime();
    template<class T> void floor();
    template<class T> void ceil();
    template<class T> void floorCeilRuntime();
    template<class T> void domainError();
    void integerArgument();

    void functionsSqrt();
    void functionsFloorCeil();
    void vectorLength();
};

typedef Math::Deg<Float> Deg;
typedef Math::Rad<Float> Rad;
typedef Math::Vector3<Float> Vector3;
typedef Math::Vector4<Float> Vector4;

ConstantMathTest::ConstantMathTest() {
    addTests({&ConstantMathTest::sqrt<Float>,
              &ConstantMathTest::sqrt<Double>,
              &ConstantMathTest::sqrt<long double>,
              &ConstantMathTest::sqrtRuntime<Float>,
              &ConstantMathTest::sqrtRuntime<Double>,
              &ConstantMathTest::sqrtRuntime<long double>,
              &ConstantMathTest::floor<Float>,
              &ConstantMathTest::floor<Double>,
              &ConstantMathTest::floor<long double>,
              &ConstantMathTest::ceil<Float>,
              &ConstantMathTest::ceil<Double>,
              &ConstantMathTest::ceil<long double>,
              &ConstantMathTest::floorCeilRuntime<Float>,
              &ConstantMathTest::floorCeilRuntime<Double>,
              &ConstantMathTest::floorCeilRuntime<long double>,
              &ConstantMathTest::domainError<Float>,
              &ConstantMathTest::domainError<Double>,
              &ConstantMathTest::domainError<long double>,
              &ConstantMathTest::integerArgument,

              &ConstantMathTest::functionsSqrt,
              &ConstantMathTest::functionsFloorCeil,
              &ConstantMathTest::vectorLength});
}

template<class T> bool same(T a, T b) {
    return a != a ? b != b : a == b && std::signbit(a) == std::signbit(b);
}

/* Debug prints too few digits to show a difference of one ulp. Not
   Utility::format(), its snprintf() prints long double wrong on MinGW
   Clang. */
template<class T> std::string exact(T value) {
    std::ostringstream out;
    out << std::setprecision(std::numeric_limits<T>::max_digits10) << value;
    return out.str();
}

template<class T> T referenceSqrt(T value) {
    return std::sqrt(value);
}

#if defined CORRADE_TARGET_MINGW && defined CORRADE_TARGET_SSE2
/* MinGW's sqrt() rounds the x87 fsqrt result to 64 bits and then again to
   double, which is off by one ulp for some inputs. GCC calls it at -O0. */
double referenceSqrt(double value) {
    return _mm_cvtsd_f64(_mm_sqrt_sd(_mm_setzero_pd(), _mm_set_sd(value)));
}
#endif

struct Random {
    std::uint64_t operator()() {
        std::uint64_t z = state += 0x9e3779b97f4a7c15ull;
        z = (z ^ (z >> 30))*0xbf58476d1ce4e5b9ull;
        z = (z ^ (z >> 27))*0x94d049bb133111ebull;
        return z ^ (z >> 31);
    }

    std::uint64_t state;
};

/* Built 32 bits at a time so each step is exact */
template<class T> T randomPositive(Random& random, int minExponent, int maxExponent) {
    T significand = T(0);
    for(int bits = std::numeric_limits<T>::digits; bits > 0; bits -= 32) {
        const int n = bits < 32 ? bits : 32;
        significand = significand*T(std::uint64_t{1} << n) + T(random() >> (64 - n));
    }
    return std::ldexp(significand, minExponent + int(random() % std::uint64_t(maxExponent - minExponent + 1)));
}

template<class T> T randomPositive(Random& random) {
    typedef std::numeric_limits<T> L;
    return randomPositive<T>(random, L::min_exponent - 2*L::digits, L::max_exponent - L::digits);
}

template<class T> T randomSign(Random& random, T value) {
    return random() & 1 ? -value : value;
}

#ifdef MAGNUM_HAS_CONSTEXPR_SQRT
template<class T, std::size_t size> struct Results {
    T data[size];
};

template<class F, class T, std::size_t size> constexpr Results<T, size> evaluate(const T(&values)[size]) {
    Results<T, size> out{};
    for(std::size_t i = 0; i != size; ++i)
        out.data[i] = F{}(values[i]);
    return out;
}

struct Sqrt {
    template<class T> constexpr T operator()(T value) const {
        return ConstantMath::sqrt(value);
    }
};

struct Floor {
    template<class T> constexpr T operator()(T value) const {
        return ConstantMath::floor(value);
    }
};

struct Ceil {
    template<class T> constexpr T operator()(T value) const {
        return ConstantMath::ceil(value);
    }
};
#endif

template<class T> void ConstantMathTest::sqrt() {
    setTestCaseTemplateName(TypeTraits<T>::name());

    #ifndef MAGNUM_HAS_CONSTEXPR_SQRT
    CORRADE_SKIP("ConstantMath::sqrt() not supported by the compiler.");
    #else
    typedef std::numeric_limits<T> L;
    /* Between 2 and 20 there are values where Heron's iteration ends one ulp
       below and one ulp above the correctly rounded result, for each type */
    static constexpr T values[]{
        T(1), T(2), T(3), T(4), T(5), T(6), T(7), T(8), T(9), T(10),
        T(11), T(12), T(13), T(14), T(15), T(16), T(17), T(18), T(19), T(20),
        T(0.1), T(0.2), T(0.5), T(0.8), T(1.3), T(1.5), T(3.2), T(12345.678),
        L::epsilon(),
        T(1) + L::epsilon(),
        T(2) - L::epsilon(),
        T(4) - T(2)*L::epsilon(),
        L::min(),
        L::min()/T(2),
        L::denorm_min(),
        T(3)*L::denorm_min(),
        T(6)*L::denorm_min(),
        L::max(),
        T(1)/L::epsilon(),
        T(0.0),
        -T(0.0),
        L::infinity(),
        L::quiet_NaN()
    };
    constexpr Results<T, Containers::arraySize(values)> actual = evaluate<Sqrt>(values);

    for(std::size_t i = 0; i != Containers::arraySize(values); ++i) {
        CORRADE_ITERATION(i << exact(values[i]));
        const T expected = referenceSqrt(values[i]);
        CORRADE_FAIL_IF(!same(actual.data[i], expected),
            "Got" << exact(actual.data[i]) << "but expected" << exact(expected));
    }
    #endif
}

template<class T> void ConstantMathTest::sqrtRuntime() {
    setTestCaseTemplateName(TypeTraits<T>::name());

    #ifndef MAGNUM_HAS_CONSTEXPR_SQRT
    CORRADE_SKIP("ConstantMath::sqrt() not supported by the compiler.");
    #elif defined RUNTIME_SKIP_REASON
    CORRADE_SKIP(RUNTIME_SKIP_REASON);
    #else
    Random random{0x5c1};
    for(std::size_t i = 0; i != 10000; ++i) {
        const T value = randomPositive<T>(random);
        CORRADE_ITERATION(i << exact(value));
        const T actual = ConstantMath::sqrt(value);
        const T expected = referenceSqrt(value);
        CORRADE_FAIL_IF(!same(actual, expected),
            "Got" << exact(actual) << "but expected" << exact(expected));
    }
    #endif
}

template<class T> void ConstantMathTest::floor() {
    setTestCaseTemplateName(TypeTraits<T>::name());

    #ifndef MAGNUM_HAS_CONSTEXPR_FLOOR
    CORRADE_SKIP("ConstantMath::floor() not supported by the compiler.");
    #else
    typedef std::numeric_limits<T> L;
    /* 1/epsilon is 2^(digits - 1), from which on there are no fractional
       bits */
    static constexpr T values[]{
        T(0.0), -T(0.0),
        T(0.25), -T(0.25),
        T(0.5), -T(0.5),
        T(1), -T(1),
        T(1.5), -T(1.5),
        T(2.5), -T(2.5),
        T(123.456), -T(123.456),
        T(1) - L::epsilon()/T(2), -(T(1) - L::epsilon()/T(2)),
        T(1) + L::epsilon(), -(T(1) + L::epsilon()),
        T(1)/L::epsilon() - T(1), -(T(1)/L::epsilon() - T(1)),
        T(1)/L::epsilon() - T(0.5), -(T(1)/L::epsilon() - T(0.5)),
        T(1)/L::epsilon(), -(T(1)/L::epsilon()),
        T(1)/L::epsilon() + T(1), -(T(1)/L::epsilon() + T(1)),
        L::denorm_min(), -L::denorm_min(),
        L::min(), -L::min(),
        L::max(), -L::max(),
        L::infinity(), -L::infinity(),
        L::quiet_NaN()
    };
    constexpr Results<T, Containers::arraySize(values)> actual = evaluate<Floor>(values);

    for(std::size_t i = 0; i != Containers::arraySize(values); ++i) {
        CORRADE_ITERATION(i << exact(values[i]));
        const T expected = std::floor(values[i]);
        CORRADE_FAIL_IF(!same(actual.data[i], expected),
            "Got" << exact(actual.data[i]) << "but std::floor() returns" << exact(expected));
    }
    #endif
}

template<class T> void ConstantMathTest::ceil() {
    setTestCaseTemplateName(TypeTraits<T>::name());

    #ifndef MAGNUM_HAS_CONSTEXPR_CEIL
    CORRADE_SKIP("ConstantMath::ceil() not supported by the compiler.");
    #else
    typedef std::numeric_limits<T> L;
    static constexpr T values[]{
        T(0.0), -T(0.0),
        T(0.25), -T(0.25),
        T(0.5), -T(0.5),
        T(1), -T(1),
        T(1.5), -T(1.5),
        T(2.5), -T(2.5),
        T(123.456), -T(123.456),
        T(1) - L::epsilon()/T(2), -(T(1) - L::epsilon()/T(2)),
        T(1) + L::epsilon(), -(T(1) + L::epsilon()),
        T(1)/L::epsilon() - T(1), -(T(1)/L::epsilon() - T(1)),
        T(1)/L::epsilon() - T(0.5), -(T(1)/L::epsilon() - T(0.5)),
        T(1)/L::epsilon(), -(T(1)/L::epsilon()),
        T(1)/L::epsilon() + T(1), -(T(1)/L::epsilon() + T(1)),
        L::denorm_min(), -L::denorm_min(),
        L::min(), -L::min(),
        L::max(), -L::max(),
        L::infinity(), -L::infinity(),
        L::quiet_NaN()
    };
    constexpr Results<T, Containers::arraySize(values)> actual = evaluate<Ceil>(values);

    for(std::size_t i = 0; i != Containers::arraySize(values); ++i) {
        CORRADE_ITERATION(i << exact(values[i]));
        const T expected = std::ceil(values[i]);
        CORRADE_FAIL_IF(!same(actual.data[i], expected),
            "Got" << exact(actual.data[i]) << "but std::ceil() returns" << exact(expected));
    }
    #endif
}

template<class T> void ConstantMathTest::floorCeilRuntime() {
    setTestCaseTemplateName(TypeTraits<T>::name());

    #if !defined MAGNUM_HAS_CONSTEXPR_FLOOR || !defined MAGNUM_HAS_CONSTEXPR_CEIL
    CORRADE_SKIP("ConstantMath::floor() and ceil() not supported by the compiler.");
    #elif defined RUNTIME_SKIP_REASON
    CORRADE_SKIP(RUNTIME_SKIP_REASON);
    #else
    typedef std::numeric_limits<T> L;
    Random random{0xf1002};
    for(std::size_t i = 0; i != 10000; ++i) {
        /* Every other value is below 2^(digits + 2), where the fractional
           bits are */
        const T value = randomSign(random, i % 2 ?
            randomPositive<T>(random) :
            randomPositive<T>(random, -L::digits - 2, 2));
        CORRADE_ITERATION(i << exact(value));
        const T actualFloor = ConstantMath::floor(value);
        const T expectedFloor = std::floor(value);
        CORRADE_FAIL_IF(!same(actualFloor, expectedFloor),
            "Got" << exact(actualFloor) << "but std::floor() returns" << exact(expectedFloor));
        const T actualCeil = ConstantMath::ceil(value);
        const T expectedCeil = std::ceil(value);
        CORRADE_FAIL_IF(!same(actualCeil, expectedCeil),
            "Got" << exact(actualCeil) << "but std::ceil() returns" << exact(expectedCeil));
    }
    #endif
}

template<class T> void ConstantMathTest::domainError() {
    setTestCaseTemplateName(TypeTraits<T>::name());

    #ifndef MAGNUM_HAS_CONSTEXPR_SQRT
    CORRADE_SKIP("ConstantMath::sqrt() not supported by the compiler.");
    #else
    /* These aren't constant expressions, at run time they go through std:: */
    typedef std::numeric_limits<T> L;
    CORRADE_VERIFY(std::isnan(ConstantMath::sqrt(-T(1))));
    CORRADE_VERIFY(std::isnan(ConstantMath::sqrt(-L::denorm_min())));
    CORRADE_VERIFY(std::isnan(ConstantMath::sqrt(-L::infinity())));
    #endif
}

void ConstantMathTest::integerArgument() {
    #ifndef MAGNUM_HAS_CONSTEXPR_SQRT
    CORRADE_SKIP("ConstantMath not supported by the compiler.");
    #else
    constexpr auto a = ConstantMath::sqrt(16);
    constexpr auto b = ConstantMath::floor(-5);
    constexpr auto c = ConstantMath::ceil(5);
    CORRADE_VERIFY(std::is_same<decltype(a), const double>::value);
    CORRADE_VERIFY(std::is_same<decltype(b), const double>::value);
    CORRADE_VERIFY(std::is_same<decltype(c), const double>::value);
    CORRADE_COMPARE(a, std::sqrt(16));
    CORRADE_COMPARE(b, std::floor(-5));
    CORRADE_COMPARE(c, std::ceil(5));
    #endif
}

void ConstantMathTest::functionsSqrt() {
    #ifndef MAGNUM_HAS_CONSTEXPR_SQRT
    CORRADE_SKIP("Constexpr sqrt() not supported by the compiler.");
    #else
    constexpr Float a = Math::sqrt(2.0f);
    constexpr Double b = Math::sqrt(2.0);
    constexpr Int c = Math::sqrt(17);
    constexpr Float d = Math::sqrtInverted(2.0f);
    constexpr Vector3 e = Math::sqrt(Vector3{1.0f, 2.0f, 0.25f});
    constexpr Vector3 f = Math::sqrtInverted(Vector3{1.0f, 2.0f, 0.25f});
    constexpr Vector3 g = Math::refract(Vector3{0.6f, -0.8f, 0.0f}, Vector3::yAxis(), 1.0f/1.5f);

    /* Arguments not known at compile time go through std::sqrt() */
    CORRADE_COMPARE(a, Math::sqrt(2.0f));
    CORRADE_COMPARE(b, Math::sqrt(2.0));
    CORRADE_COMPARE(c, 4);
    CORRADE_COMPARE(d, Math::sqrtInverted(2.0f));
    CORRADE_COMPARE(e, Math::sqrt(Vector3{1.0f, 2.0f, 0.25f}));
    CORRADE_COMPARE(f, Math::sqrtInverted(Vector3{1.0f, 2.0f, 0.25f}));
    CORRADE_COMPARE(g, Math::refract(Vector3{0.6f, -0.8f, 0.0f}, Vector3::yAxis(), 1.0f/1.5f));
    #endif
}

void ConstantMathTest::functionsFloorCeil() {
    #if !defined MAGNUM_HAS_CONSTEXPR_FLOOR || !defined MAGNUM_HAS_CONSTEXPR_CEIL
    CORRADE_SKIP("Constexpr floor() and ceil() not supported by the compiler.");
    #else
    constexpr Float a = Math::floor(-2.5f);
    constexpr Float b = Math::ceil(-2.5f);
    constexpr Int c = Math::floor(5);
    constexpr Deg d = Math::floor(Deg{2.7f});
    constexpr Rad e = Math::ceil(Rad{-2.7f});
    constexpr Vector3 f = Math::floor(Vector3{1.5f, -1.5f, -0.0f});
    constexpr Vector3 g = Math::ceil(Vector3{1.5f, -1.5f, -0.5f});

    CORRADE_COMPARE(a, Math::floor(-2.5f));
    CORRADE_COMPARE(b, Math::ceil(-2.5f));
    CORRADE_COMPARE(c, 5);
    CORRADE_COMPARE(d, Math::floor(Deg{2.7f}));
    CORRADE_COMPARE(e, Math::ceil(Rad{-2.7f}));
    CORRADE_COMPARE(f, Math::floor(Vector3{1.5f, -1.5f, -0.0f}));
    CORRADE_COMPARE(g, Math::ceil(Vector3{1.5f, -1.5f, -0.5f}));
    CORRADE_VERIFY(std::signbit(f.z()));
    CORRADE_VERIFY(std::signbit(g.z()));
    #endif
}

void ConstantMathTest::vectorLength() {
    #ifndef MAGNUM_HAS_CONSTEXPR_SQRT
    CORRADE_SKIP("Constexpr sqrt() not supported by the compiler.");
    #else
    constexpr Vector3 vector{1.0f, 2.0f, 2.0f};
    constexpr Float a = vector.length();
    constexpr Float b = vector.lengthInverted();
    constexpr Vector3 c = vector.normalized();
    constexpr Vector3 d = vector.resized(6.0f);
    constexpr Vector4 e = Math::planeEquation(Vector3{1.0f, 0.0f, 1.0f}, Vector3{0.0f, 1.0f, 1.0f}, Vector3{0.0f, 0.0f, 1.0f});

    CORRADE_COMPARE(a, vector.length());
    CORRADE_COMPARE(b, vector.lengthInverted());
    CORRADE_COMPARE(c, vector.normalized());
    CORRADE_COMPARE(d, vector.resized(6.0f));
    CORRADE_COMPARE(e, Math::planeEquation(Vector3{1.0f, 0.0f, 1.0f}, Vector3{0.0f, 1.0f, 1.0f}, Vector3{0.0f, 0.0f, 1.0f}));
    #endif
}

}}}}

CORRADE_TEST_MAIN(Magnum::Math::Test::ConstantMathTest)

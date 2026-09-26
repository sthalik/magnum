/*
    This file is part of Magnum.

    Copyright © 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019,
                2020, 2021, 2022, 2023, 2024, 2025, 2026
              Vladimír Vondruš <mosra@centrum.cz>
    Copyright © 2020 janos <janos.meny@googlemail.com>

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

#include <Corrade/Containers/String.h>
#include <Corrade/TestSuite/Tester.h>

#include "Magnum/Math/Functions.h"
#include "Magnum/Math/Time.h"
#include "Magnum/Math/Vector4.h"

namespace Magnum { namespace Math { namespace Test { namespace {

struct FunctionsTest: TestSuite::Tester {
    explicit FunctionsTest();

    template<class T> void popcount();

    void powIntegral();
    void pow();

    void min();
    void max();
    void minmax();
    void clamp();
    void nanPropagation();

    void sign();
    void abs();

    void floor();
    void round();
    void ceil();
    void fmod();
    void fract();

    void binomialCoefficient();
    void binomialCoefficientInvalidInput();
    void binomialCoefficientOverflow();

    void sqrt();
    void sqrtInverted();
    void lerp();
    void lerpLimits();
    void lerpInfinity();
    void lerpBool();
    void lerpInverted();
    void select();
    void selectBool();
    void fma();

    void logIntegral();
    void log2();
    void log();
    void exp();
    template<class T> void div();
    template<class T> void divNegative();
    void isInf();
    void isInfVector();
    void constexprIsInf();
    void isNan();
    void isNanfVector();
    void constexprIsNan();

    void reflect();
    void reflectNotNormalized();
    void refract();
    void refractNotNormalized();

    void trigonometric();
    void trigonometricWithBase();
    template<class T> void sincos();
};

using namespace Literals;

using Magnum::Constants;
using Magnum::Deg;
using Magnum::Rad;
using Magnum::Seconds;
using Magnum::Vector2;
using Magnum::Vector3;
using Magnum::Vector4;

FunctionsTest::FunctionsTest() {
    addRepeatedTests<FunctionsTest>({
        &FunctionsTest::popcount<UnsignedInt>,
        &FunctionsTest::popcount<UnsignedLong>}, 8);

    addTests({&FunctionsTest::powIntegral,
              &FunctionsTest::pow,

              &FunctionsTest::min,
              &FunctionsTest::max,
              &FunctionsTest::minmax,
              &FunctionsTest::clamp,
              &FunctionsTest::nanPropagation,

              &FunctionsTest::sign,
              &FunctionsTest::abs,

              &FunctionsTest::floor,
              &FunctionsTest::round,
              &FunctionsTest::ceil,
              &FunctionsTest::fmod,
              &FunctionsTest::fract,

              &FunctionsTest::binomialCoefficient,
              &FunctionsTest::binomialCoefficientInvalidInput,
              &FunctionsTest::binomialCoefficientOverflow,

              &FunctionsTest::sqrt,
              &FunctionsTest::sqrtInverted,
              &FunctionsTest::lerp,
              &FunctionsTest::lerpLimits,
              &FunctionsTest::lerpInfinity,
              &FunctionsTest::lerpBool,
              &FunctionsTest::lerpInverted,
              &FunctionsTest::select,
              &FunctionsTest::selectBool,
              &FunctionsTest::fma,

              &FunctionsTest::logIntegral,
              &FunctionsTest::log2,
              &FunctionsTest::log,
              &FunctionsTest::exp,
              &FunctionsTest::div<UnsignedByte>,
              &FunctionsTest::div<Byte>,
              &FunctionsTest::div<UnsignedShort>,
              &FunctionsTest::div<Short>,
              &FunctionsTest::div<UnsignedInt>,
              &FunctionsTest::div<Int>,
              &FunctionsTest::div<UnsignedLong>,
              &FunctionsTest::div<Long>,
              &FunctionsTest::divNegative<Byte>,
              &FunctionsTest::divNegative<Short>,
              &FunctionsTest::divNegative<Int>,
              &FunctionsTest::divNegative<Long>,
              &FunctionsTest::isInf,
              &FunctionsTest::isInfVector,
              &FunctionsTest::constexprIsInf,
              &FunctionsTest::isNan,
              &FunctionsTest::isNanfVector,
              &FunctionsTest::constexprIsNan,

              &FunctionsTest::reflect,
              &FunctionsTest::reflectNotNormalized,
              &FunctionsTest::refract,
              &FunctionsTest::refractNotNormalized,

              &FunctionsTest::trigonometric,
              &FunctionsTest::trigonometricWithBase,
              &FunctionsTest::sincos<Float>,
              &FunctionsTest::sincos<Double>,
              #ifndef CORRADE_TARGET_EMSCRIPTEN
              &FunctionsTest::sincos<long double>,
              #endif
              });
}

template<class T> void FunctionsTest::popcount() {
    setTestCaseTemplateName(TypeTraits<T>::name());

    /* Trivial cases */
    CORRADE_COMPARE(Math::popcount(T(0)), 0);
    CORRADE_COMPARE(Math::popcount(~T{}), sizeof(T)*8);

    /* 0x101101011101000110010100 */
    CORRADE_COMPARE(Math::popcount(T(0xb5d194) << testCaseRepeatId()), 12);
}

void FunctionsTest::powIntegral() {
    CORRADE_COMPARE(Math::pow<10>(2ul), 1024ul);
    CORRADE_COMPARE(Math::pow<0>(3ul), 1ul);
    CORRADE_COMPARE(Math::pow<2>(2.0f), 4.0f);

    /* Constant expression */
    constexpr Int a = Math::pow<3>(5);
    CORRADE_COMPARE(a, 125);

    CORRADE_COMPARE(Math::pow<2>(Vector3{2.0f, -3.0f, 1.5f}), (Vector3{4.0f, 9.0f, 2.25f}));

    /* Not testing wrapped types -- what unit should have degrees squared? */
}

void FunctionsTest::pow() {
    CORRADE_COMPARE(Math::pow(2.0f, 0.5f), 1.414213562f);
    CORRADE_COMPARE(Math::pow(Vector3{2.0f, 9.0f, 25.0f}, 0.5f), (Vector3{1.414213562f, 3.0f, 5.0f}));

    /* Not testing wrapped types -- what unit should have degrees squared? */
}

void FunctionsTest::min() {
    CORRADE_COMPARE(Math::min(5, 9), 5);
    CORRADE_COMPARE(Math::min(Vector3i(5, -3, 2), Vector3i(9, -5, 18)), Vector3i(5, -5, 2));
    CORRADE_COMPARE(Math::min(Vector3i{5, -3, 2}, 1), (Vector3i{1, -3, 1}));

    /* Wrapped types */
    CORRADE_COMPARE(Math::min(5.0_degf, 9.0_degf), 5.0_degf);
    CORRADE_COMPARE(Math::min(6.0_sec, 5500.0_msec), 5.5_sec);
}

void FunctionsTest::max() {
    CORRADE_COMPARE(Math::max(5, 9), 9);
    CORRADE_COMPARE(Math::max(Vector3i(5, -3, 2), Vector3i(9, -5, 18)), Vector3i(9, -3, 18));
    CORRADE_COMPARE(Math::max(Vector3i{5, -3, 2}, 3), (Vector3i{5, 3, 3}));

    /* Wrapped types */
    CORRADE_COMPARE(Math::max(5.0_degf, 9.0_degf), 9.0_degf);
    CORRADE_COMPARE(Math::max(6.0_sec, 5500.0_msec), 6000.0_msec);
}

void FunctionsTest::minmax() {
    const auto expectedScalar = Containers::pair(-5.0f, 4.0f);
    CORRADE_COMPARE(Math::minmax(-5.0f, 4.0f), expectedScalar);
    CORRADE_COMPARE(Math::minmax(4.0f, -5.0f), expectedScalar);

    /* Constant expression */
    constexpr Containers::Pair<Float, Float> c = Math::minmax(4.0f, -5.0f);
    CORRADE_COMPARE(c, expectedScalar);

    const Vector3 a{5.0f, -4.0f, 1.0f};
    const Vector3 b{7.0f, -3.0f, 1.0f};
    const Containers::Pair<Vector3, Vector3> expectedVector{{5.0f, -4.0f, 1.0f}, {7.0f, -3.0f, 1.0f}};
    CORRADE_COMPARE((Containers::Pair<Vector3, Vector3>{Math::minmax(a, b)}), expectedVector);
    CORRADE_COMPARE((Containers::Pair<Vector3, Vector3>{Math::minmax(b, a)}), expectedVector);

    /* Wrapped types */
    CORRADE_COMPARE(Math::minmax(4.0_degf, 5.0_degf), Containers::pair(4.0_degf, 5.0_degf));
    CORRADE_COMPARE(Math::minmax(6.0_sec, 5500.0_msec), Containers::pair(5.5_sec, 6.0_sec));
}

void FunctionsTest::clamp() {
    CORRADE_COMPARE(Math::clamp(0.5f, -1.0f, 5.0f), 0.5f);
    CORRADE_COMPARE(Math::clamp(-1.6f, -1.0f, 5.0f), -1.0f);
    CORRADE_COMPARE(Math::clamp(9.5f, -1.0f, 5.0f), 5.0f);

    CORRADE_COMPARE(Math::clamp(Vector3{0.5f, -1.6f, 9.5f},
                                Vector3{-1.0f, 2.0f, 0.3f},
                                Vector3{0.7f, 3.0f, 5.0f}),
        (Vector3{0.5f, 2.0f, 5.0f}));

    CORRADE_COMPARE(Math::clamp(Vector3{0.5f, -1.6f, 9.5f}, -1.0f, 5.0f), (Vector3{0.5f, -1.0f, 5.0f}));

    /* Wrapped types */
    CORRADE_COMPARE(Math::clamp(0.5_degf, 0.75_degf, 1.0_degf), 0.75_degf);
    CORRADE_COMPARE(Math::clamp(0.5_sec, 0.75_sec, 799.9_msec), 0.75_sec);
}

void FunctionsTest::nanPropagation() {
    CORRADE_COMPARE(Math::min(Constants::nan(), 5.0f), Constants::nan());
    CORRADE_COMPARE(Math::min(5.0f, Constants::nan()), 5.0f);
    CORRADE_COMPARE(Math::min(Vector2{Constants::nan(), 6.0f}, Vector2{5.0f})[0], Constants::nan());
    CORRADE_COMPARE(Math::min(Vector2{Constants::nan(), 6.0f}, Vector2{5.0f})[1], 5.0f);

    CORRADE_COMPARE(Math::max(Constants::nan(), 5.0f), Constants::nan());
    CORRADE_COMPARE(Math::max(5.0f, Constants::nan()), 5.0f);
    CORRADE_COMPARE(Math::max(Vector2{Constants::nan(), 4.0f}, Vector2{5.0f})[0], Constants::nan());
    CORRADE_COMPARE(Math::max(Vector2{Constants::nan(), 4.0f}, Vector2{5.0f})[1], 5.0f);

    CORRADE_COMPARE(Math::clamp(Constants::nan(), 2.0f, 6.0f), Constants::nan());
    /* NaN clamp bounds are not propagating the NaN, but those cases are
       usually user error anyway */
    CORRADE_COMPARE(Math::clamp(Vector2{Constants::nan(), 1.0f}, 2.0f, 6.0f)[0], Constants::nan());
    CORRADE_COMPARE(Math::clamp(Vector2{Constants::nan(), 1.0f}, 2.0f, 6.0f)[1], 2.0f);
}

void FunctionsTest::sign() {
    CORRADE_COMPARE(Math::sign(3516), 1);
    CORRADE_COMPARE(Math::sign(0.0f), 0.0f);
    CORRADE_COMPARE(Math::sign(-3.7), -1.0);
    CORRADE_COMPARE(Math::sign(Vector3i(0, -3, 2)), Vector3i(0, -1, 1));

    /* Wrapped types. Returns the underlying type instead of the type itself as
       it should be possible to do e.g. angle*Math::sign(angle) and get an
       angle value back. With returning the type itself such operation wouldn't
       have any definable unit. */
    CORRADE_COMPARE(Math::sign(-3.7_degf), -1.0f);
    CORRADE_COMPARE(Math::sign(-2.5_usec), -1.0f);
    CORRADE_COMPARE(Math::sign(Math::Vector2<Deg>{3.5_degf, -1.9_degf}), (Vector2{1.0f, -1.0f}));
}

void FunctionsTest::abs() {
    CORRADE_COMPARE(Math::abs(-5), 5);
    CORRADE_COMPARE(Math::abs(5), 5);
    CORRADE_COMPARE(Math::abs(Vector3i(5, -3, 2)), Vector3i(5, 3, 2));

    /* Constant expression */
    constexpr Int a = Math::abs(-5);
    constexpr Float b = Math::abs(-5.5f);
    constexpr Deg c = Math::abs(-5.0_degf);
    CORRADE_COMPARE(a, 5);
    CORRADE_COMPARE(b, 5.5f);
    CORRADE_COMPARE(c, 5.0_degf);

    /* Wrapped types */
    CORRADE_COMPARE(Math::abs(-5.0_degf), 5.0_degf);
    CORRADE_COMPARE(Math::abs(-5.0_msec), 5.0_msec);

    constexpr Float negativeNan = -Constants::nan();
    constexpr Double negativeNand = -Math::Constants<Double>::nan();
    CORRADE_VERIFY(std::signbit(negativeNan));
    CORRADE_VERIFY(std::signbit(negativeNand));
    CORRADE_VERIFY(!std::signbit(Math::abs(-0.0f)));
    CORRADE_VERIFY(!std::signbit(Math::abs(-0.0)));
    CORRADE_VERIFY(!std::signbit(Math::abs(negativeNan)));
    CORRADE_VERIFY(!std::signbit(Math::abs(negativeNand)));

    /* In a constant expression only where there's a builtin for it */
    #if defined CORRADE_TARGET_GCC || defined CORRADE_TARGET_CLANG || \
        (defined CORRADE_TARGET_MSVC && _MSC_VER >= 1952)
    constexpr Float zero = Math::abs(-0.0f);
    constexpr Double zerod = Math::abs(-0.0);
    constexpr Float nan = Math::abs(negativeNan);
    constexpr Double nand = Math::abs(negativeNand);
    CORRADE_VERIFY(!std::signbit(zero));
    CORRADE_VERIFY(!std::signbit(zerod));
    CORRADE_VERIFY(!std::signbit(nan));
    CORRADE_VERIFY(!std::signbit(nand));
    #endif
}

void FunctionsTest::floor() {
    CORRADE_COMPARE(Math::floor(0.7f), 0.0f);
    CORRADE_COMPARE(Math::floor(Vector3{2.3f, 0.7f, 1.5f}), (Vector3{2.0f, 0.0f, 1.0f}));

    /* Wrapped types */
    CORRADE_COMPARE(Math::floor(2.7_degf), 2.0_degf);
    /* Nanoseconds are an integer type */
    CORRADE_COMPARE(Math::floor(Seconds{2.7_sec}), Seconds{2.0_sec});
}

void FunctionsTest::round() {
    CORRADE_COMPARE(Math::round(2.3f), 2.0f);
    CORRADE_COMPARE(Math::round(Vector3{2.3f, 0.7f, 1.5f}), (Vector3{2.0f, 1.0f, 2.0f}));

    /* We are working around missing std::round() in Android, thus we must
       test that the behavior is the same on both implementations */
    CORRADE_COMPARE(Math::round(-2.0f), -2.0f);
    CORRADE_COMPARE(Math::round(-1.5f), -2.0f);
    CORRADE_COMPARE(Math::round(-1.3f), -1.0f);
    CORRADE_COMPARE(Math::round(1.3f), 1.0f);
    CORRADE_COMPARE(Math::round(1.5f), 2.0f);
    CORRADE_COMPARE(Math::round(2.0f), 2.0f);

    /* Wrapped types */
    CORRADE_COMPARE(Math::round(2.7_degf), 3.0_degf);
    /* Nanoseconds are an integer type */
    CORRADE_COMPARE(Math::round(Seconds{2.7_sec}), Seconds{3.0_sec});
}

void FunctionsTest::ceil() {
    CORRADE_COMPARE(Math::ceil(2.3f), 3.0f);
    CORRADE_COMPARE(Math::ceil(Vector3{2.3f, 0.7f, 1.5f}), (Vector3{3.0f, 1.0f, 2.0f}));

    /* Wrapped types */
    CORRADE_COMPARE(Math::ceil(2.7_degf), 3.0_degf);
    /* Nanoseconds are an integer type */
    CORRADE_COMPARE(Math::ceil(Seconds{2.7_sec}), Seconds{3.0_sec});
}

void FunctionsTest::binomialCoefficient() {
    CORRADE_COMPARE(Math::binomialCoefficient(1, 1), 1ull);
    CORRADE_COMPARE(Math::binomialCoefficient(1, 0), 1ull);
    CORRADE_COMPARE(Math::binomialCoefficient(19, 11), 75582ull);
    CORRADE_COMPARE(Math::binomialCoefficient(1000, 999), 1000ull);
    CORRADE_COMPARE(Math::binomialCoefficient(0, 0), 1ull);
    CORRADE_COMPARE(Math::binomialCoefficient(32, 11), 129024480ull);
    CORRADE_COMPARE(Math::binomialCoefficient(62, 31), 465428353255261088ull);
}

void FunctionsTest::binomialCoefficientInvalidInput() {
    CORRADE_SKIP_IF_NO_DEBUG_ASSERT();

    Containers::String out;
    Error redirectError{&out};
    Math::binomialCoefficient(15, 16);
    CORRADE_COMPARE(out, "Math::binomialCoefficient(): k can't be greater than n in (15 choose 16)\n");
}

void FunctionsTest::binomialCoefficientOverflow() {
    CORRADE_SKIP_IF_NO_DEBUG_ASSERT();

    Containers::String out;
    Error redirectError{&out};
    Math::binomialCoefficient(63, 31);
    CORRADE_COMPARE(out, "Math::binomialCoefficient(): overflow for (63 choose 31)\n");
}

void FunctionsTest::fmod() {
    CORRADE_COMPARE(Math::fmod(5.1f, 3.0f), 2.1f);
    CORRADE_COMPARE(Math::fmod(Vector3{5.1f, -5.1f, 6.8f}, Vector3{3.0f, -2.0f, 1.1f}), (Vector3{2.1f, -1.1f, 0.2f}));
    CORRADE_COMPARE(Math::fmod(Vector3{5.1f, -5.1f, 6.8f}, 3.0f), (Vector3{2.1f, -2.1f, 0.8f}));

    /* Wrapped types */
    CORRADE_COMPARE(Math::fmod(2.7_degf, 1.3_degf), 0.1_degf);
    /* Nanoseconds are an integer type */
    CORRADE_COMPARE(Math::fmod(Seconds{2.7_sec}, Seconds{1.3_sec}), Seconds{0.1_sec});
}

void FunctionsTest::fract() {
    CORRADE_COMPARE(Math::fract(5.132f), 0.132f);
    /* It works the other way for negative numbers */
    CORRADE_COMPARE(Math::fract(-5.132f), 0.868f);
    CORRADE_COMPARE(Math::fract(Vector3{5.132f, 4.0f, -2.474f}), (Vector3{0.132f, 0.0f, 0.526f}));

    /* Wrapped types */
    CORRADE_COMPARE(Math::fract(2.7_degf), 0.7_degf);
    /* Nanoseconds are an integer type */
    CORRADE_COMPARE(Math::fract(Seconds{2.7_sec}), Seconds{0.7_sec});
}

void FunctionsTest::sqrt() {
    CORRADE_COMPARE(Math::sqrt(16), 4);
    CORRADE_COMPARE(Math::sqrt(Vector3i(256, 1, 0)), Vector3i(16, 1, 0));

    /* Not testing wrapped types -- what unit should have degrees squared? */
}

void FunctionsTest::sqrtInverted() {
    CORRADE_COMPARE(Math::sqrtInverted(16.0f), 0.25f);
    CORRADE_COMPARE(Math::sqrtInverted(Vector3{1.0f, 4.0f, 16.0f}), (Vector3{1.0f, 0.5f, 0.25f}));

    /* Not testing wrapped types -- what unit should have degrees squared? */
}

void FunctionsTest::lerp() {
    /* Floating-point / integral scalar */
    CORRADE_COMPARE(Math::lerp(2.0f, 5.0f, 0.5f), 3.5f);
    CORRADE_COMPARE(Math::lerp(2, 5, 0.5f), 3);

    /* Floating-point vector */
    Vector3 a{-1.0f, 2.0f, 3.0f};
    Vector3 b{3.0f, -2.0f, 11.0f};
    CORRADE_COMPARE(Math::lerp(a, b, 0.25f), (Vector3{0.0f, 1.0f, 5.0f}));

    /* Integer vector */
    Vector3i c(0, 128, 64);
    Vector3i d(16, 0, 32);
    CORRADE_COMPARE(Math::lerp(c, d, 0.25f), Vector3i(4, 96, 56));

    /* Vector as interpolation phase */
    CORRADE_COMPARE(Math::lerp(a, b, Vector3{0.25f, 0.5f, 0.75f}), (Vector3{0.0f, 0.0f, 9.0f}));

    /* Wrapped types */
    CORRADE_COMPARE(Math::lerp(2.0_degf, 5.0_degf, 0.5f), 3.5_degf);
    CORRADE_COMPARE(Math::lerp(2.0_usec, 5.0_usec, 0.5f), 3.5_usec);
}

template<class T, class U> T lerpOptimized(const T& a, const T& b, U t) {
    /* One multiplication and two additions, while `T((U(1) - t)*a + t*b)` is
       two multiplications, addition and subtraction. Doesn't correctly
       preserve boundary values. */
    return t*(b - a) + a;
}

void FunctionsTest::lerpLimits() {
    CORRADE_COMPARE(Math::lerp(1.0e10f, 1.0e-5f, 0.0f), 1.0e10f);
    CORRADE_COMPARE(Math::lerp(1.0e10f, 1.0e-5f, 1.0f), 1.0e-5f);
    CORRADE_COMPARE(Math::lerp(1.0e-5f, 1.0e10f, 0.0f), 1.0e-5f);
    CORRADE_COMPARE(Math::lerp(1.0e-5f, 1.0e10f, 1.0f), 1.0e10f);

    CORRADE_COMPARE(lerpOptimized(1.0e10f, 1.0e-5f, 0.0f), 1.0e10f);
    {
        CORRADE_EXPECT_FAIL("\"Optimized\" version of a lerp doesn't correctly preserve boundary values with wildly different magnitudes.");
        CORRADE_COMPARE(lerpOptimized(1.0e10f, 1.0e-5f, 1.0f), 1.0e-5f);
    }
}

void FunctionsTest::lerpInfinity() {
    CORRADE_COMPARE(Math::lerp(Constants::inf(), 0.0f, 0.0f), Constants::inf());
    CORRADE_COMPARE(Math::lerp(0.0f, Constants::inf(), 1.0f), Constants::inf());
    {
        CORRADE_EXPECT_FAIL("Lerp with infinity doesn't correctly preserve the other boundary value.");
        CORRADE_COMPARE(Math::lerp(Constants::inf(), 0.0f, 1.0f), 0.0f);
        CORRADE_COMPARE(Math::lerp(0.0f, Constants::inf(), 0.0f), 0.0f);
    }

    CORRADE_COMPARE(lerpOptimized(0.0f, Constants::inf(), 1.0f), Constants::inf());
    {
        CORRADE_EXPECT_FAIL("\"Optimized\" version of a lerp doesn't correctly preserve boundary values if an infinity is present.");
        CORRADE_COMPARE(lerpOptimized(Constants::inf(), 0.0f, 0.0f), Constants::inf());
        CORRADE_COMPARE(lerpOptimized(Constants::inf(), 0.0f, 1.0f), 0.0f);
        CORRADE_COMPARE(lerpOptimized(0.0f, Constants::inf(), 0.0f), 0.0f);
    }
}

void FunctionsTest::lerpBool() {
    /* Scalar interpolation phase */
    CORRADE_COMPARE(Math::lerp(Vector3i{1, 2, 3}, Vector3i{5, 6, 7}, true), (Vector3i{5, 6, 7}));
    CORRADE_COMPARE(Math::lerp(BitVector<3>{5}, BitVector<3>{true}, false), BitVector<3>{5});

    /* Constant expression */
    constexpr Vector3i a = Math::lerp(Vector3i{1, 2, 3}, Vector3i{5, 6, 7}, true);
    constexpr BitVector<3> b = Math::lerp(BitVector<3>{5}, BitVector<3>{true}, false);
    constexpr Float c = Math::lerp(2.0f, 5.0f, true);
    CORRADE_COMPARE(a, (Vector3i{5, 6, 7}));
    CORRADE_COMPARE(b, BitVector<3>{5});
    CORRADE_COMPARE(c, 5.0f);

    /* Vector interpolation phase */
    CORRADE_COMPARE(Math::lerp(Vector3i{1, 2, 3}, Vector3i{5, 6, 7}, BitVector<3>(5)), (Vector3i{5, 2, 7}));
    CORRADE_COMPARE(Math::lerp(BitVector<3>{false}, BitVector<3>{true}, BitVector<3>(5)), BitVector<3>{5});

    /* Wrapped types */
    CORRADE_COMPARE(Math::lerp(2.0_degf, 5.0_degf, true), 5.0_degf);
    CORRADE_COMPARE(Math::lerp(2.0_msec, 5.0_msec, true), 5.0_msec);
}

void FunctionsTest::lerpInverted() {
    /* Floating-point scalar */
    CORRADE_COMPARE(Math::lerpInverted(2.0f, 5.0f, 3.5f), 0.5f);

    /* Floating-point vector */
    Vector3 a{-1.0f, 2.0f, 3.0f};
    Vector3 b{3.0f, -2.0f, 11.0f};
    CORRADE_COMPARE(Math::lerpInverted(a, b, Vector3{0.0f, 0.0f, 9.0f}), (Vector3{0.25f, 0.5f, 0.75f}));

    /* Wrapped types */
    CORRADE_COMPARE(Math::lerpInverted(2.0_degf, 5.0_degf, 3.5_degf), 0.5f);
    /* Nanoseconds are an integer type */
    CORRADE_COMPARE(Math::lerpInverted(Seconds{2.0_sec}, Seconds{5.0_sec}, Seconds{3.5_sec}), 0.5f);
    {
        CORRADE_EXPECT_FAIL("This doesn't work correctly with (integer) Nanoseconds.");
        CORRADE_COMPARE(Math::lerpInverted(2.0_sec, 5.0_sec, 3.5_sec), 0.5f);
    }
}

void FunctionsTest::select() {
    /* Floating-point / integral scalar */
    CORRADE_COMPARE(Math::select(2.0f, 5.0f, 0.5f), 2.0f);
    CORRADE_COMPARE(Math::select(2.0f, 5.0f, 1.0f), 5.0f);
    CORRADE_COMPARE(Math::select(2, 5, -0.5f), 2);
    CORRADE_COMPARE(Math::select(2, 5, 1.1f), 5);

    /* Constant expression */
    constexpr Float c = Math::select(2.0f, 5.0f, 1.0f);
    constexpr Int d = Math::select(2, 5, -0.5f);
    CORRADE_COMPARE(c, 5.0f);
    CORRADE_COMPARE(d, 2);

    /* Floating-point vector */
    Vector3 a{-1.0f, 2.0f, 3.0f};
    Vector3 b{3.0f, -2.0f, 11.0f};
    CORRADE_COMPARE(Math::select(a, b, -0.25f), (Vector3{-1.0f, 2.0f, 3.0f}));
    CORRADE_COMPARE(Math::select(a, b, 5.5f), (Vector3{3.0f, -2.0f, 11.0f}));

    /* Vector as interpolation phase */
    CORRADE_COMPARE(Math::select(a, b, Vector3{0.25f, 1.5f, 1.0f}), (Vector3{-1.0f, -2.0f, 11.0f}));

    /* Wrapped types */
    CORRADE_COMPARE(Math::select(2.0_degf, 5.0_degf, 0.5f), 2.0_degf);
    CORRADE_COMPARE(Math::select(2.0_msec, 5.0_msec, 0.5f), 2.0_msec);
}

void FunctionsTest::selectBool() {
    CORRADE_COMPARE(Math::select(true, false, 0.5f), true);
    CORRADE_COMPARE(Math::select(Math::BitVector<4>{0xa}, Math::BitVector<4>{0x5}, 1.1f), Math::BitVector<4>{0x5});
    CORRADE_COMPARE(Math::select(Math::BitVector<4>{0xa}, Math::BitVector<4>{0x5}, Vector4{1.1f, -1.0f, 1.3f, 0.5f}), Math::BitVector<4>{0xf});
}

void FunctionsTest::fma() {
    CORRADE_COMPARE(Math::fma(2.0f, 3.0f, 0.75f), 6.75f);
    CORRADE_COMPARE(Math::fma(Vector3{ 2.0f,  1.5f,  0.5f},
                              Vector3{ 3.0f,  2.0f, -1.0f},
                              Vector3{0.75f, 0.25f,  0.1f}),
        (Vector3{6.75f, 3.25f, -0.4f}));
}

void FunctionsTest::logIntegral() {
    CORRADE_COMPARE(Math::log(2, 256), 8ul);
    CORRADE_COMPARE(Math::log(256, 2), 0ul);
}

void FunctionsTest::log2() {
    CORRADE_COMPARE(Math::log2(2153), 11);
}

void FunctionsTest::log() {
    CORRADE_COMPARE(Math::log(2.0f), 0.693147f);
}

void FunctionsTest::exp() {
    CORRADE_COMPARE(Math::exp(0.693147f), 2.0f);
}

template<class T> void FunctionsTest::div() {
    setTestCaseTemplateName(TypeTraits<T>::name());

    Containers::Pair<T, T> div = Math::div(T(57), T(6));
    CORRADE_COMPARE(div.first(), T(9));
    CORRADE_COMPARE(div.second(), T(3));

    /* Constant expression */
    constexpr Containers::Pair<T, T> a = Math::div(T(57), T(6));
    CORRADE_COMPARE(a.first(), T(9));
    CORRADE_COMPARE(a.second(), T(3));
}

template<class T> void FunctionsTest::divNegative() {
    setTestCaseTemplateName(TypeTraits<T>::name());

    Containers::Pair<T, T> a = Math::div(T(-57), T(6));
    Containers::Pair<T, T> b = Math::div(T(57), T(-6));
    CORRADE_COMPARE(a.first(), T(-9));
    CORRADE_COMPARE(a.second(), T(-3));
    CORRADE_COMPARE(b.first(), T(-9));
    CORRADE_COMPARE(b.second(), T(3));

    /* Constant expression */
    constexpr Containers::Pair<T, T> c = Math::div(T(-57), T(6));
    constexpr Containers::Pair<T, T> d = Math::div(T(57), T(-6));
    CORRADE_COMPARE(c.first(), T(-9));
    CORRADE_COMPARE(c.second(), T(-3));
    CORRADE_COMPARE(d.first(), T(-9));
    CORRADE_COMPARE(d.second(), T(3));
}

void FunctionsTest::isInf() {
    CORRADE_VERIFY(Math::isInf(Constants::inf()));
    CORRADE_VERIFY(Math::isInf(-Constants::inf()));
    CORRADE_VERIFY(!Math::isInf(Constants::nan()));
    CORRADE_VERIFY(!Math::isInf(5.3f));
    CORRADE_VERIFY(!Math::isInf(5));

    /* Wrapped types */
    CORRADE_VERIFY(Math::isInf(-Rad(Constants::inf())));
    CORRADE_VERIFY(Math::isInf(Seconds{-Constants::inf()}));
    CORRADE_VERIFY(!Math::isInf(5.3_degf));
    /* Nanoseconds are an integer type */
    CORRADE_VERIFY(!Math::isInf(Seconds{5.3_msec}));
}

void FunctionsTest::isInfVector() {
    CORRADE_COMPARE(Math::isInf(Vector3{0.3f, -Constants::inf(), 1.0f}), Math::BitVector<3>{0x02});
    CORRADE_COMPARE(Math::isInf(Vector3{0.3f, 1.0f, -Constants::nan()}), Math::BitVector<3>{0x00});
    CORRADE_COMPARE(Math::isInf(Vector3i{5, -2, 9}), Math::BitVector<3>{0x00});
}

void FunctionsTest::constexprIsInf() {
    #ifndef MAGNUM_HAS_CONSTEXPR_ISINF
    CORRADE_SKIP("Constexpr isInf() not supported by the compiler.");
    #else
    constexpr bool a = Math::isInf(Constants::inf());
    constexpr bool b = Math::isInf(-Constants::inf());
    constexpr bool c = Math::isInf(Constants::nan());
    constexpr bool d = Math::isInf(5.3f);
    constexpr bool e = Math::isInf(5);
    constexpr bool f = Math::isInf(-Rad(Constants::inf()));
    CORRADE_VERIFY(a);
    CORRADE_VERIFY(b);
    CORRADE_VERIFY(!c);
    CORRADE_VERIFY(!d);
    CORRADE_VERIFY(!e);
    CORRADE_VERIFY(f);
    #endif
}

void FunctionsTest::isNan() {
    CORRADE_VERIFY(!Math::isNan(Constants::inf()));
    CORRADE_VERIFY(!Math::isNan(-Constants::inf()));
    CORRADE_VERIFY(Math::isNan(Constants::nan()));
    CORRADE_VERIFY(!Math::isNan(5.3f));
    CORRADE_VERIFY(!Math::isNan(5));

    /* Wrapped types */
    CORRADE_VERIFY(Math::isNan(-Rad(Constants::nan())));
    CORRADE_VERIFY(Math::isNan(Seconds(Constants::nan())));
    CORRADE_VERIFY(!Math::isNan(5.3_degf));
    CORRADE_VERIFY(!Math::isNan(Seconds{5.3_usec}));
}

void FunctionsTest::isNanfVector() {
    CORRADE_COMPARE(Math::isNan(Vector3{0.3f, 1.0f, -Constants::nan()}), Math::BitVector<3>{0x04});
    CORRADE_COMPARE(Math::isNan(Vector3{0.3f, -Constants::inf(), 1.0f}), Math::BitVector<3>{0x00});
    CORRADE_COMPARE(Math::isNan(Vector3i{5, -2, 9}), Math::BitVector<3>{0x00});
}

void FunctionsTest::constexprIsNan() {
    #ifndef MAGNUM_HAS_CONSTEXPR_ISNAN
    CORRADE_SKIP("Constexpr isNan() not supported by the compiler.");
    #else
    constexpr bool a = Math::isNan(Constants::nan());
    constexpr bool b = Math::isNan(5.3f);
    constexpr bool c = Math::isNan(-Constants::nan());
    constexpr bool d = Math::isNan(5);
    CORRADE_VERIFY(a);
    CORRADE_VERIFY(!b);
    CORRADE_VERIFY(c);
    CORRADE_VERIFY(!d);
    #endif
}

void FunctionsTest::reflect() {
    /* Reflection along Y will simply flip the Y component */
    CORRADE_COMPARE(Math::reflect(
        Vector3{1.0f, 2.0f, 3.0f},
        Vector3::yAxis()),
        (Vector3{1.0f, -2.0f, 3.0f}));

    CORRADE_COMPARE(Math::reflect(
        Vector3{2.0f, 1.0f, 1.0f},
        Vector3{1.0f, -1.0f, 1.0f}.normalized()),
        (Vector3{0.666667f, 2.33333f, -0.333333f}));
}

void FunctionsTest::reflectNotNormalized() {
    CORRADE_SKIP_IF_NO_DEBUG_ASSERT();

    Containers::String out;
    Error redirectError{&out};
    Math::reflect(Vector3{}, Vector3{1.0f});
    CORRADE_COMPARE(out,
        "Math::reflect(): normal Vector(1, 1, 1) is not normalized\n");
}

void FunctionsTest::refract() {
    CORRADE_COMPARE(Math::refract(
        Vector3{1.0f, 0.0f, 1.0f}.normalized(),
        Vector3{0.0f, 0.0f, -1.0f}, 1.0f/1.5f),
        (Vector3{0.471405f, 0.0f, 0.881917f}));
    CORRADE_COMPARE(Math::refract(
        Vector3{4.0f, 1.0f, 1.0f}.normalized(),
        Vector3{0.0f, -2.0f, -1.0f}.normalized(), 1.0f/1.5f),
        (Vector3{0.628539f, 0.661393f, 0.409264f}));

    /* Total absorption */
    CORRADE_COMPARE(Math::refract(
        Vector3{1.0f, 0.1f, 0.0f}.normalized(),
        Vector3::yAxis(), 1.5f),
        Vector3{0.0f});
}

void FunctionsTest::refractNotNormalized() {
    CORRADE_SKIP_IF_NO_DEBUG_ASSERT();

    Containers::String out;
    Error redirectError{&out};
    Math::refract(Vector3{}, Vector3{1.0f}, 0.0f);
    CORRADE_COMPARE(out,
        "Math::refract(): vectors Vector(0, 0, 0) and Vector(1, 1, 1) are not normalized\n");
}

void FunctionsTest::trigonometric() {
    CORRADE_COMPARE(Math::sin(30.0_degf), 0.5f);
    CORRADE_COMPARE(Math::sin(Rad(Constants::pi()/6)), 0.5f);
    CORRADE_COMPARE_AS(Math::asin(0.5f), 30.0_degf, Deg);

    CORRADE_COMPARE(Math::cos(60.0_degf), 0.5f);
    CORRADE_COMPARE(Math::cos(Rad(Constants::pi()/3)), 0.5f);
    CORRADE_COMPARE_AS(Math::acos(0.5f), 60.0_degf, Deg);

    CORRADE_COMPARE(Math::sincos(30.0_degf).first(), 0.5f);
    CORRADE_COMPARE(Math::sincos(30.0_degf).second(), 0.8660254037844386f);
    CORRADE_COMPARE(Math::sincos(Rad(Constants::pi()/6)).first(), 0.5f);
    CORRADE_COMPARE(Math::sincos(Rad(Constants::pi()/6)).second(), 0.8660254037844386f);

    CORRADE_COMPARE(Math::tan(45.0_degf), 1.0f);
    CORRADE_COMPARE(Math::tan(Rad(Constants::pi()/4)), 1.0f);
    CORRADE_COMPARE_AS(Math::atan(1.0f), 45.0_degf, Deg);
}

void FunctionsTest::trigonometricWithBase() {
    /* Verify that the functions can be called with Unit<Deg, T> and Unit<Rad, T> */
    CORRADE_VERIFY(std::is_same<decltype(2*15.0_degf), Unit<Math::Deg, Float>>::value);
    CORRADE_VERIFY(std::is_same<decltype(2*Rad(Constants::pi()/12)), Unit<Math::Rad, Float>>::value);

    CORRADE_COMPARE(Math::sin(2*15.0_degf), 0.5f);
    CORRADE_COMPARE(Math::sin(2*Rad(Constants::pi()/12)), 0.5f);

    CORRADE_COMPARE(Math::cos(2*30.0_degf), 0.5f);
    CORRADE_COMPARE(Math::cos(2*Rad(Constants::pi()/6)), 0.5f);

    CORRADE_COMPARE(Math::sincos(2*15.0_degf).first(), 0.5f);
    CORRADE_COMPARE(Math::sincos(2*15.0_degf).second(), 0.8660254037844386f);
    CORRADE_COMPARE(Math::sincos(2*Rad(Constants::pi()/12)).first(), 0.5f);
    CORRADE_COMPARE(Math::sincos(2*Rad(Constants::pi()/12)).second(), 0.8660254037844386f);

    CORRADE_COMPARE(Math::tan(2*22.5_degf), 1.0f);
    CORRADE_COMPARE(Math::tan(2*Rad(Constants::pi()/8)), 1.0f);
}

template<class T> void FunctionsTest::sincos() {
    setTestCaseTemplateName(TypeTraits<T>::name());

    /* For GCC's __builtin_sincos this verifies that all specializations are
       correct */
    CORRADE_COMPARE(Math::sincos(Math::Deg<T>(T(30.0))).first(), T(0.5));
    CORRADE_COMPARE(Math::sincos(Math::Deg<T>(T(30.0))).second(), T(0.866025403784438647l));
}

}}}}

CORRADE_TEST_MAIN(Magnum::Math::Test::FunctionsTest)

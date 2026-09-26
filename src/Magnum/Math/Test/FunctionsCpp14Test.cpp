/*
    This file is part of Magnum.

    Copyright © 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019,
                2020, 2021, 2022, 2023, 2024, 2025, 2026
              Vladimír Vondruš <mosra@centrum.cz>

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

#include <Corrade/TestSuite/Tester.h>

#include "Magnum/Math/Functions.h"
#include "Magnum/Math/Vector3.h"

namespace Magnum { namespace Math { namespace Test { namespace {

struct FunctionsCpp14Test: TestSuite::Tester {
    explicit FunctionsCpp14Test();

    void constexprMinVector();
    void constexprMaxVector();
    void constexprMinmaxVector();
    void constexprClampVector();
    void constexprSign();
    void constexprAbsVector();
    void constexprLerp();
    void constexprLerpBoolVector();
    void constexprLerpInverted();
    void constexprReflect();

    void constexprLerpBoolBitVector();
    void constexprFmod();
    void constexprCopysign();
    void constexprIsNanVector();
};

using Magnum::Constants;
using Magnum::Vector3;

FunctionsCpp14Test::FunctionsCpp14Test() {
    addTests({&FunctionsCpp14Test::constexprMinVector,
              &FunctionsCpp14Test::constexprMaxVector,
              &FunctionsCpp14Test::constexprMinmaxVector,
              &FunctionsCpp14Test::constexprClampVector,
              &FunctionsCpp14Test::constexprSign,
              &FunctionsCpp14Test::constexprAbsVector,
              &FunctionsCpp14Test::constexprLerp,
              &FunctionsCpp14Test::constexprLerpBoolVector,
              &FunctionsCpp14Test::constexprLerpInverted,
              &FunctionsCpp14Test::constexprReflect});

    addTests({&FunctionsCpp14Test::constexprLerpBoolBitVector});

    addTests({&FunctionsCpp14Test::constexprFmod,
              &FunctionsCpp14Test::constexprCopysign,
              &FunctionsCpp14Test::constexprIsNanVector});
}

#define CMP(actual, expected)                                   \
    do {                                                        \
        static_assert(Math::abs(actual - expected) < 1e-8, ""); \
        CORRADE_COMPARE(actual, expected);                      \
    } while (false)

void FunctionsCpp14Test::constexprMinVector() {
    constexpr Vector3i a = Math::min(Vector3i{5, -3, 2}, Vector3i{9, -5, 18});
    constexpr Vector3i b = Math::min(Vector3i{5, -3, 2}, 1);
    CORRADE_COMPARE(a, (Vector3i{5, -5, 2}));
    CORRADE_COMPARE(b, (Vector3i{1, -3, 1}));
}

void FunctionsCpp14Test::constexprMaxVector() {
    constexpr Vector3i a = Math::max(Vector3i{5, -3, 2}, Vector3i{9, -5, 18});
    constexpr Vector3i b = Math::max(Vector3i{5, -3, 2}, 3);
    CORRADE_COMPARE(a, (Vector3i{9, -3, 18}));
    CORRADE_COMPARE(b, (Vector3i{5, 3, 3}));
}

void FunctionsCpp14Test::constexprMinmaxVector() {
    constexpr Containers::Pair<Vector3, Vector3> a = Math::minmax(Vector3{5.0f, -4.0f, 1.0f}, Vector3{7.0f, -3.0f, 1.0f});
    constexpr Containers::Pair<Vector3, Vector3> b = Math::minmax(Vector3{7.0f, -3.0f, 1.0f}, Vector3{5.0f, -4.0f, 1.0f});
    CORRADE_COMPARE(a.first(), (Vector3{5.0f, -4.0f, 1.0f}));
    CORRADE_COMPARE(a.second(), (Vector3{7.0f, -3.0f, 1.0f}));
    CORRADE_COMPARE(b.first(), (Vector3{5.0f, -4.0f, 1.0f}));
    CORRADE_COMPARE(b.second(), (Vector3{7.0f, -3.0f, 1.0f}));
}

void FunctionsCpp14Test::constexprClampVector() {
    constexpr Vector3 a = Math::clamp(Vector3{0.5f, -1.6f, 9.5f},
                                      Vector3{-1.0f, 2.0f, 0.3f},
                                      Vector3{0.7f, 3.0f, 5.0f});
    constexpr Vector3 b = Math::clamp(Vector3{0.5f, -1.6f, 9.5f}, -1.0f, 5.0f);
    CORRADE_COMPARE(a, (Vector3{0.5f, 2.0f, 5.0f}));
    CORRADE_COMPARE(b, (Vector3{0.5f, -1.0f, 5.0f}));
}

void FunctionsCpp14Test::constexprSign() {
    constexpr Int a = Math::sign(3516);
    constexpr Float b = Math::sign(0.0f);
    constexpr Double c = Math::sign(-3.7);
    constexpr Vector3i d = Math::sign(Vector3i{0, -3, 2});
    CORRADE_COMPARE(a, 1);
    CORRADE_COMPARE(b, 0.0f);
    CORRADE_COMPARE(c, -1.0);
    CORRADE_COMPARE(d, (Vector3i{0, -1, 1}));
}

void FunctionsCpp14Test::constexprAbsVector() {
    constexpr Vector3i a = Math::abs(Vector3i{5, -3, 2});
    constexpr Vector3 b = Math::abs(Vector3{-5.5f, 3.0f, -0.25f});
    CORRADE_COMPARE(a, (Vector3i{5, 3, 2}));
    CORRADE_COMPARE(b, (Vector3{5.5f, 3.0f, 0.25f}));
}

void FunctionsCpp14Test::constexprLerp() {
    constexpr Float a = Math::lerp(2.0f, 5.0f, 0.5f);
    constexpr Int b = Math::lerp(2, 5, 0.5f);
    constexpr Vector3 c = Math::lerp(Vector3{-1.0f, 2.0f, 3.0f}, Vector3{3.0f, -2.0f, 11.0f}, 0.25f);
    constexpr Vector3i d = Math::lerp(Vector3i{0, 128, 64}, Vector3i{16, 0, 32}, 0.25f);
    constexpr Vector3 e = Math::lerp(Vector3{-1.0f, 2.0f, 3.0f},
                                     Vector3{3.0f, -2.0f, 11.0f},
                                     Vector3{0.25f, 0.5f, 0.75f});
    CORRADE_COMPARE(a, 3.5f);
    CORRADE_COMPARE(b, 3);
    CORRADE_COMPARE(c, (Vector3{0.0f, 1.0f, 5.0f}));
    CORRADE_COMPARE(d, (Vector3i{4, 96, 56}));
    CORRADE_COMPARE(e, (Vector3{0.0f, 0.0f, 9.0f}));
}

void FunctionsCpp14Test::constexprLerpBoolVector() {
    constexpr Vector3i a = Math::lerp(Vector3i{1, 2, 3}, Vector3i{5, 6, 7}, BitVector<3>{5});
    CORRADE_COMPARE(a, (Vector3i{5, 2, 7}));
}

void FunctionsCpp14Test::constexprLerpInverted() {
    constexpr Float a = Math::lerpInverted(2.0f, 5.0f, 3.5f);
    constexpr Vector3 b = Math::lerpInverted(Vector3{-1.0f, 2.0f, 3.0f},
                                             Vector3{3.0f, -2.0f, 11.0f},
                                             Vector3{0.0f, 0.0f, 9.0f});
    CORRADE_COMPARE(a, 0.5f);
    CORRADE_COMPARE(b, (Vector3{0.25f, 0.5f, 0.75f}));
}

void FunctionsCpp14Test::constexprReflect() {
    constexpr Vector3 a = Math::reflect(Vector3{1.0f, 2.0f, 3.0f}, Vector3::yAxis());
    CORRADE_COMPARE(a, (Vector3{1.0f, -2.0f, 3.0f}));
}

void FunctionsCpp14Test::constexprLerpBoolBitVector() {
    /* Same condition as MAGNUM_BITVECTOR_SET_CONSTEXPR14 in BitVector.h */
    #if defined(CORRADE_TARGET_GCC) && !defined(CORRADE_TARGET_CLANG) && __GNUC__*100 + __GNUC_MINOR__ < 602
    CORRADE_SKIP("Constexpr BitVector::set() not supported by the compiler.");
    #else
    constexpr BitVector<3> a = Math::lerp(BitVector<3>{false}, BitVector<3>{true}, BitVector<3>{5});
    constexpr BitVector<3> b = Math::lerp(BitVector<3>{5}, BitVector<3>{2}, BitVector<3>{3});
    CORRADE_COMPARE(a, BitVector<3>{5});
    CORRADE_COMPARE(b, BitVector<3>{6});
    #endif
}

void FunctionsCpp14Test::constexprFmod() {
    #ifndef MAGNUM_HAS_CONSTEXPR_FMOD
    CORRADE_SKIP("Constexpr fmod() not supported by the compiler.");
    #else
    constexpr auto a = Math::fmod(7., 3.);       CMP(a, 1.);
    constexpr auto b = Math::fmod(0.75, 0.5);    CMP(b, 0.25);
    constexpr auto c = Math::fmod(0.5, 1.);      CMP(c, 0.5);
    constexpr auto d = Math::fmod(1./60, 1./61); CMP(d, 0.0002732240437158459);
    constexpr auto e = Math::fmod(0.5, 1.);      CMP(e, 0.5);
    constexpr auto f = Math::fmod(-0.5, 1.);     CMP(f, -0.5);
    constexpr auto g = Math::fmod(-0.5, -1.);    CMP(g, -0.5);
    constexpr auto h = Math::fmod(0.5, -1.);     CMP(h, 0.5);

    constexpr auto i = Math::fmod(Vector2d(0.15, 0.92), Vector2d(0.125, 0.9));
    CMP(i.x(), 0.025); CMP(i.y(), 0.02);

    constexpr auto j = Math::fmod(Vector2d(0.13, 0.14), 0.11);
    CMP(j.x(), 0.02); CMP(j.y(), 0.03);
    #endif
}

void FunctionsCpp14Test::constexprCopysign() {
    #ifndef MAGNUM_HAS_CONSTEXPR_COPYSIGN
    CORRADE_SKIP("Constexpr copysign() not supported by the compiler.");
    #else
    constexpr auto a = Math::copysign( 1.,  2.); CMP(a,  1.);
    constexpr auto b = Math::copysign(-3.,  4.); CMP(b,  3.);
    constexpr auto c = Math::copysign( 5., -6.); CMP(c, -5.);
    constexpr auto d = Math::copysign(-7., -8.); CMP(d, -7.);
    constexpr auto e = Math::copysign(Vector2d(-1., 2.), Vector2d(3., -4.));
    CMP(e.x(), 1.); CMP(e.y(), -2.);
    #endif
}

void FunctionsCpp14Test::constexprIsNanVector() {
    #if defined(CORRADE_TARGET_GCC) && !defined(CORRADE_TARGET_CLANG) && __GNUC__*100 + __GNUC_MINOR__ < 602
    CORRADE_SKIP("Constexpr BitVector::set() not supported by the compiler.");
    #elif !defined MAGNUM_HAS_CONSTEXPR_ISNAN
    CORRADE_SKIP("Constexpr isNan() not supported by the compiler.");
    #else
    constexpr BitVector<3> a = Math::isNan(Vector3{0.3f, 1.0f, -Constants::nan()});
    constexpr BitVector<3> b = Math::isNan(Vector3i{5, -2, 9});
    CORRADE_COMPARE(a, BitVector<3>{0x04});
    CORRADE_COMPARE(b, BitVector<3>{0x00});
    #endif
}

}}}}

CORRADE_TEST_MAIN(Magnum::Math::Test::FunctionsCpp14Test)

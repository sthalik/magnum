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

#include "Magnum/Math/FunctionsBatch.h"
#include "Magnum/Math/Vector2.h"

namespace Magnum { namespace Math { namespace Test { namespace {

struct FunctionsBatchCpp14Test: TestSuite::Tester {
    explicit FunctionsBatchCpp14Test();

    void constexprMinMax();
    void constexprMinMaxNan();
    void constexprMinmaxArray();
    void constexprMinmaxVector();
    void constexprMinmaxNanVector();
};

using Magnum::Constants;
using Magnum::Vector2;

FunctionsBatchCpp14Test::FunctionsBatchCpp14Test() {
    addTests({&FunctionsBatchCpp14Test::constexprMinMax,
              &FunctionsBatchCpp14Test::constexprMinMaxNan,
              &FunctionsBatchCpp14Test::constexprMinmaxArray,
              &FunctionsBatchCpp14Test::constexprMinmaxVector,
              &FunctionsBatchCpp14Test::constexprMinmaxNanVector});
}

void FunctionsBatchCpp14Test::constexprMinMax() {
    constexpr Int a = Math::min({5, -2, 9});
    constexpr Int b = Math::max({5, -2, 9});
    CORRADE_COMPARE(a, -2);
    CORRADE_COMPARE(b, 9);
}

void FunctionsBatchCpp14Test::constexprMinMaxNan() {
    #ifndef MAGNUM_HAS_CONSTEXPR_ISNAN
    CORRADE_SKIP("Constexpr isNan() not supported by the compiler.");
    #else
    constexpr Float a = Math::min({Constants::nan(), 1.0f, -3.0f});
    constexpr Float b = Math::max({Constants::nan(), 1.0f, -3.0f});
    constexpr Float c = Math::min({Constants::nan(), Constants::nan()});
    CORRADE_COMPARE(a, -3.0f);
    CORRADE_COMPARE(b, 1.0f);
    CORRADE_VERIFY(Math::isNan(c));

    /* cl's constant evaluator treats both nan < x and x < nan as true */
    #if !defined CORRADE_TARGET_MSVC || defined CORRADE_TARGET_CLANG_CL
    constexpr Float d = Math::min({1.0f, Constants::nan(), -3.0f});
    constexpr Float e = Math::max({1.0f, Constants::nan(), -3.0f});
    CORRADE_COMPARE(d, -3.0f);
    CORRADE_COMPARE(e, 1.0f);
    #endif
    #endif
}

constexpr Containers::Pair<Int, Int> minmaxMutableArray() {
    Int data[]{5, -7, 11};
    return Math::minmax(data);
}

constexpr Containers::Pair<Int, Int> minmaxMutableArrayView() {
    Int data[]{5, -7, 11};
    return Math::minmax(Containers::ArrayView<Int>{data});
}

void FunctionsBatchCpp14Test::constexprMinmaxArray() {
    constexpr Int data[]{5, -7, 11};
    constexpr auto a = Math::minmax(data);
    constexpr auto b = Math::minmax(Containers::ArrayView<const Int>{data});
    constexpr auto c = minmaxMutableArray();
    constexpr auto d = minmaxMutableArrayView();
    CORRADE_COMPARE(a.first(), -7);
    CORRADE_COMPARE(a.second(), 11);
    CORRADE_COMPARE(b, a);
    CORRADE_COMPARE(c, a);
    CORRADE_COMPARE(d, a);
}

void FunctionsBatchCpp14Test::constexprMinmaxVector() {
    #if defined(CORRADE_TARGET_GCC) && !defined(CORRADE_TARGET_CLANG) && __GNUC__*100 + __GNUC_MINOR__ < 602
    CORRADE_SKIP("Constexpr BitVector::set() not supported by the compiler.");
    #elif !defined MAGNUM_HAS_CONSTEXPR_ISNAN
    CORRADE_SKIP("Constexpr isNan() not supported by the compiler.");
    #else
    constexpr auto a = Math::minmax({Vector2{5.0f, -3.0f}, Vector2{-7.0f, 11.0f}});
    constexpr auto b = Math::minmax(std::initializer_list<Vector2>{});
    constexpr Vector2 data[]{Vector2{5.0f, -3.0f}, Vector2{-7.0f, 11.0f}};
    constexpr auto c = Math::minmax(Containers::ArrayView<const Vector2>{data});
    constexpr auto d = Math::minmax(Containers::ArrayView<Vector2>{});
    constexpr auto e = Math::minmax(data);
    CORRADE_COMPARE(a.first(), (Vector2{-7.0f, -3.0f}));
    CORRADE_COMPARE(a.second(), (Vector2{5.0f, 11.0f}));
    CORRADE_COMPARE(b.first(), Vector2{});
    CORRADE_COMPARE(b.second(), Vector2{});
    CORRADE_COMPARE(c.first(), a.first());
    CORRADE_COMPARE(c.second(), a.second());
    CORRADE_COMPARE(d.first(), Vector2{});
    CORRADE_COMPARE(d.second(), Vector2{});
    CORRADE_COMPARE(e.first(), a.first());
    CORRADE_COMPARE(e.second(), a.second());
    #endif
}

void FunctionsBatchCpp14Test::constexprMinmaxNanVector() {
    #if defined(CORRADE_TARGET_GCC) && !defined(CORRADE_TARGET_CLANG) && __GNUC__*100 + __GNUC_MINOR__ < 602
    CORRADE_SKIP("Constexpr BitVector::set() not supported by the compiler.");
    #elif !defined MAGNUM_HAS_CONSTEXPR_ISNAN
    CORRADE_SKIP("Constexpr isNan() not supported by the compiler.");
    #else
    constexpr auto a = Math::minmax({Vector2{Constants::nan(), 2.0f},
                                    Vector2{1.0f, Constants::nan()}});
    constexpr auto b = Math::minmax({Vector2{Constants::nan(), 2.0f},
                                    Vector2{Constants::nan(), -3.0f}});
    {
        #if defined(CORRADE_TARGET_MSVC) && !defined(CORRADE_TARGET_CLANG_CL)
        CORRADE_EXPECT_FAIL("MSVC's constant evaluator treats comparisons with NaNs as true.");
        #endif
        CORRADE_COMPARE(a.first(), (Vector2{1.0f, 2.0f}));
        CORRADE_COMPARE(a.second(), (Vector2{1.0f, 2.0f}));
    }
    CORRADE_VERIFY(Math::isNan(b.first().x()));
    CORRADE_VERIFY(Math::isNan(b.second().x()));
    CORRADE_COMPARE(b.first().y(), -3.0f);
    CORRADE_COMPARE(b.second().y(), 2.0f);
    #endif
}

}}}}

CORRADE_TEST_MAIN(Magnum::Math::Test::FunctionsBatchCpp14Test)

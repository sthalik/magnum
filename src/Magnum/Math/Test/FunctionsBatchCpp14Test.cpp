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

namespace Magnum { namespace Math { namespace Test { namespace {

struct FunctionsBatchCpp14Test: TestSuite::Tester {
    explicit FunctionsBatchCpp14Test();

    void constexprMinMax();
    void constexprMinMaxNan();
};

using Magnum::Constants;

FunctionsBatchCpp14Test::FunctionsBatchCpp14Test() {
    addTests({&FunctionsBatchCpp14Test::constexprMinMax,
              &FunctionsBatchCpp14Test::constexprMinMaxNan});
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

}}}}

CORRADE_TEST_MAIN(Magnum::Math::Test::FunctionsBatchCpp14Test)

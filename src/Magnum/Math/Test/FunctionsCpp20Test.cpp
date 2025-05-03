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

struct FunctionsCpp20Test: TestSuite::Tester {
    explicit FunctionsCpp20Test();

    void minMaxClampConstexpr();
    void signAbsConstexpr();
    void lerpBitVectorConstexpr();
};

using Magnum::Vector3;
using Magnum::Vector3i;

FunctionsCpp20Test::FunctionsCpp20Test() {
    addTests({&FunctionsCpp20Test::minMaxClampConstexpr,
              &FunctionsCpp20Test::signAbsConstexpr,
              &FunctionsCpp20Test::lerpBitVectorConstexpr});
}

void FunctionsCpp20Test::minMaxClampConstexpr() {
    constexpr Vector3i a = Math::min(Vector3i{5, -3, 2}, Vector3i{9, -5, 18});
    constexpr Vector3i b = Math::min(Vector3i{5, -3, 2}, 3);
    constexpr Vector3i c = Math::max(Vector3i{5, -3, 2}, Vector3i{9, -5, 18});
    constexpr Vector3i d = Math::max(Vector3i{5, -3, 2}, 3);
    constexpr Vector3 e = Math::clamp(Vector3{0.5f, -1.6f, 9.5f},
                                      Vector3{-1.0f, 2.0f, 0.3f},
                                      Vector3{0.7f, 3.0f, 5.0f});
    constexpr Vector3 f = Math::clamp(Vector3{0.5f, -1.6f, 9.5f}, -1.0f, 5.0f);
    CORRADE_COMPARE(a, (Vector3i{5, -5, 2}));
    CORRADE_COMPARE(b, (Vector3i{3, -3, 2}));
    CORRADE_COMPARE(c, (Vector3i{9, -3, 18}));
    CORRADE_COMPARE(d, (Vector3i{5, 3, 3}));
    CORRADE_COMPARE(e, (Vector3{0.5f, 2.0f, 5.0f}));
    CORRADE_COMPARE(f, (Vector3{0.5f, -1.0f, 5.0f}));
}

void FunctionsCpp20Test::signAbsConstexpr() {
    constexpr Vector3i a = Math::sign(Vector3i{0, -3, 2});
    constexpr Vector3 b = Math::abs(Vector3{5.0f, -3.0f, 2.0f});
    CORRADE_COMPARE(a, (Vector3i{0, -1, 1}));
    CORRADE_COMPARE(b, (Vector3{5.0f, 3.0f, 2.0f}));
}

void FunctionsCpp20Test::lerpBitVectorConstexpr() {
    constexpr Vector3i a = Math::lerp(Vector3i{1, 2, 3}, Vector3i{5, 6, 7}, BitVector<3>{5});
    CORRADE_COMPARE(a, (Vector3i{5, 2, 7}));
}

}}}}

CORRADE_TEST_MAIN(Magnum::Math::Test::FunctionsCpp20Test)

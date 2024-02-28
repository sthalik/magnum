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
#include "Magnum/Math/Vector2.h"

namespace Magnum { namespace Math { namespace Test { namespace {

struct FunctionsCpp14Test: TestSuite::Tester {
    explicit FunctionsCpp14Test();

    void constexprFmod();
    void constexprCopysign();
};

FunctionsCpp14Test::FunctionsCpp14Test() {
    addTests({&FunctionsCpp14Test::constexprFmod,
              &FunctionsCpp14Test::constexprCopysign});
}

#define CMP(actual, expected)                                   \
    do {                                                        \
        static_assert(Math::abs(actual - expected) < 1e-8, ""); \
        CORRADE_COMPARE(actual, expected);                      \
    } while (false)

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

}}}}

CORRADE_TEST_MAIN(Magnum::Math::Test::FunctionsCpp14Test)

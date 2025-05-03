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

#include "Magnum/Math/Vector3.h"

namespace Magnum { namespace Math { namespace Test { namespace {

struct Vector3Cpp14Test: TestSuite::Tester {
    explicit Vector3Cpp14Test();

    void accessConstexpr();
    void compoundAssignmentConstexpr();
    void compoundAssignmentIntegralConstexpr();
    void projectedConstexpr();
};

/* What's a typedef and not a using differs from the typedefs in root Magnum
   namespace, or is not present there at all */
using Magnum::Vector3;
using Magnum::Vector3i;

Vector3Cpp14Test::Vector3Cpp14Test() {
    addTests({&Vector3Cpp14Test::accessConstexpr,
              &Vector3Cpp14Test::compoundAssignmentConstexpr,
              &Vector3Cpp14Test::compoundAssignmentIntegralConstexpr,
              &Vector3Cpp14Test::projectedConstexpr});
}

constexpr Vector3 populate() {
    Vector3 a;
    a.x() = 6.0f;
    a.y() = 3.0f;
    a.z() = 2.0f;
    a.r() *= 0.5f;
    a.g() -= 1.0f;
    a.b() /= 2.0f;
    return a;
}

void Vector3Cpp14Test::accessConstexpr() {
    constexpr Vector3 a = populate();
    CORRADE_COMPARE(a, (Vector3{3.0f, 2.0f, 1.0f}));
}

constexpr Vector3 compoundAssignment() {
    Vector3 a{1.0f, 2.0f, 3.0f};
    ((((((a += Vector3{1.0f}) -= Vector3{0.5f}) *= 2.0f) /= 0.5f)
        *= Vector3{0.5f, 0.25f, 2.0f}) /= Vector3{3.0f, 0.5f, 7.0f});
    return a;
}

void Vector3Cpp14Test::compoundAssignmentConstexpr() {
    constexpr Vector3 a = compoundAssignment();
    CORRADE_COMPARE(a, (Vector3{1.0f, 5.0f, 4.0f}));
}

constexpr Vector3i compoundAssignmentIntegral() {
    Vector3i a{32, 10, 6};
    (((((a *= -1.5f) /= -0.75f) *= Vector3{0.5f, -0.25f, 1.5f})
        /= Vector3{2.0f, -0.5f, 3.0f}) %= 7);
    ((((((a %= Vector3i{3, 2, 4}) <<= 2) >>= 1) |= Vector3i{1, 8, 3})
        &= Vector3i{6, 9, 14}) ^= Vector3i{5, 1, 10});
    return a;
}

void Vector3Cpp14Test::compoundAssignmentIntegralConstexpr() {
    constexpr Vector3i a = compoundAssignmentIntegral();
    CORRADE_COMPARE(a, (Vector3i{1, 9, 12}));
}

void Vector3Cpp14Test::projectedConstexpr() {
    constexpr Vector3 a = Vector3{1.0f, 2.0f, 3.0f}.projected(Vector3{1.0f, -1.0f, 0.5f});
    constexpr Vector3 b = Vector3{1.0f, 2.0f, 3.0f}.projectedOntoNormalized(Vector3{0.0f, 0.6f, 0.8f});
    CORRADE_COMPARE(a, (Vector3{0.222222f, -0.222222f, 0.111111f}));
    CORRADE_COMPARE(b, (Vector3{0.0f, 2.16f, 2.88f}));
}

}}}}

CORRADE_TEST_MAIN(Magnum::Math::Test::Vector3Cpp14Test)

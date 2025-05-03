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

#include "Magnum/Math/Vector.h"

namespace Magnum { namespace Math { namespace Test { namespace {

struct VectorCpp14Test: TestSuite::Tester {
    explicit VectorCpp14Test();

    void accessConstexpr();
    void compareConstexpr();
    void isZeroConstexpr();
    void isNormalizedConstexpr();
    void addSubtractConstexpr();
    void multiplyDivideConstexpr();
    void multiplyDivideIntegralConstexpr();
    void moduloBitwiseConstexpr();
    void dotConstexpr();
    void projectedConstexpr();
    void sumProductConstexpr();
    void minMaxIntegralConstexpr();
};

/* What's a typedef and not a using differs from the typedefs in root Magnum
   namespace, or is not present there at all */
typedef Vector<2, Float> Vector2;
typedef Vector<3, Float> Vector3;
typedef Vector<4, Float> Vector4;
typedef Vector<3, Int> Vector3i;
typedef Vector<4, Int> Vector4i;

VectorCpp14Test::VectorCpp14Test() {
    addTests({&VectorCpp14Test::accessConstexpr,
              &VectorCpp14Test::compareConstexpr,
              &VectorCpp14Test::isZeroConstexpr,
              &VectorCpp14Test::isNormalizedConstexpr,
              &VectorCpp14Test::addSubtractConstexpr,
              &VectorCpp14Test::multiplyDivideConstexpr,
              &VectorCpp14Test::multiplyDivideIntegralConstexpr,
              &VectorCpp14Test::moduloBitwiseConstexpr,
              &VectorCpp14Test::dotConstexpr,
              &VectorCpp14Test::projectedConstexpr,
              &VectorCpp14Test::sumProductConstexpr,
              &VectorCpp14Test::minMaxIntegralConstexpr});
}

constexpr Vector2 populate() {
    Vector2 a;
    a[0] = 3.0f;
    a.data()[1] = 2.0f;
    return a;
}

void VectorCpp14Test::accessConstexpr() {
    constexpr Vector2 a = populate();
    CORRADE_COMPARE(a, (Vector2{3.0f, 2.0f}));
}

void VectorCpp14Test::compareConstexpr() {
    constexpr bool a = Vector4{1.0f, -3.5f, 5.0f, -10.0f} == Vector4{1.0f + TypeTraits<Float>::epsilon()/2, -3.5f, 5.0f, -10.0f};
    constexpr bool b = Vector4{1.0f, -3.5f, 5.0f, -10.0f} == Vector4{1.0f, -3.5f, 5.0f - TypeTraits<Float>::epsilon()*10, -10.0f};
    constexpr bool c = Vector4{1.0f, -3.5f, 5.0f, -10.0f} != Vector4{1.0f + TypeTraits<Float>::epsilon()/2, -3.5f, 5.0f, -10.0f};
    constexpr bool d = Vector4{1.0f, -3.5f, 5.0f, -10.0f} != Vector4{1.0f, -3.5f, 5.0f - TypeTraits<Float>::epsilon()*10, -10.0f};
    constexpr bool e = Vector4i{1, -3, 5, -10} == Vector4i{1, -3, 5, -10};
    constexpr bool f = Vector4i{1, -3, 5, -10} != Vector4i{1, -3, 5, -10};
    CORRADE_VERIFY(a);
    CORRADE_VERIFY(!b);
    CORRADE_VERIFY(!c);
    CORRADE_VERIFY(d);
    CORRADE_VERIFY(e);
    CORRADE_VERIFY(!f);
}

void VectorCpp14Test::isZeroConstexpr() {
    constexpr bool a = Vector3{0.0f, TypeTraits<Float>::epsilon()*0.9f, 0.0f}.isZero();
    constexpr bool b = Vector3{0.0f, TypeTraits<Float>::epsilon(), 0.0f}.isZero();
    constexpr bool c = Vector3i{0, 0, 0}.isZero();
    constexpr bool d = Vector3i{0, 1, 0}.isZero();
    CORRADE_VERIFY(a);
    CORRADE_VERIFY(!b);
    CORRADE_VERIFY(c);
    CORRADE_VERIFY(!d);
}

void VectorCpp14Test::isNormalizedConstexpr() {
    constexpr bool a = Vector3{0.0f, 1.0f, 0.0f}.isNormalized();
    constexpr bool b = Vector3{1.0f, 2.0f, -1.0f}.isNormalized();
    CORRADE_VERIFY(a);
    CORRADE_VERIFY(!b);
}

constexpr Vector4 addSubtract() {
    Vector4 a{1.0f, -3.0f, 5.0f, -10.0f};
    a += Vector4{7.5f, 33.0f, -15.0f, 0.0f};
    a -= Vector4{0.5f, 1.0f, 2.0f, 4.0f};
    return a;
}

void VectorCpp14Test::addSubtractConstexpr() {
    constexpr Vector4 a = addSubtract();
    CORRADE_COMPARE(a, (Vector4{8.0f, 29.0f, -12.0f, -14.0f}));
}

constexpr Vector4 multiplyDivide() {
    Vector4 a{1.0f, 2.0f, 3.0f, 4.0f};
    a *= -1.5f;
    a /= 0.5f;
    a *= Vector4{2.0f, -1.0f, 0.5f, 0.25f};
    a /= Vector4{-2.0f, 3.0f, 1.5f, -1.0f};
    return a;
}

void VectorCpp14Test::multiplyDivideConstexpr() {
    constexpr Vector4 a = multiplyDivide();
    CORRADE_COMPARE(a, (Vector4{3.0f, 2.0f, -3.0f, 3.0f}));
}

constexpr Vector4i multiplyDivideIntegral() {
    Vector4i a{32, 10, -6, 2};
    a *= -1.5f;
    a /= -0.75f;
    a *= Vector4{0.5f, -0.25f, 1.5f, 2.0f};
    a /= Vector4{2.0f, -0.5f, 3.0f, 0.25f};
    a *= 2;
    a /= 4;
    a *= Vector4i{1, 2, 3, -1};
    a /= Vector4i{2, 5, 3, -4};
    return a;
}

void VectorCpp14Test::multiplyDivideIntegralConstexpr() {
    constexpr Vector4i a = multiplyDivideIntegral();
    CORRADE_COMPARE(a, (Vector4i{4, 2, -3, 4}));
}

constexpr Vector4i moduloBitwise() {
    Vector4i a{85, 31, 50, 7};
    a %= 20;
    a %= Vector4i{3, 4, 6, 5};
    a <<= 2;
    a >>= 1;
    a |= Vector4i{1, 1, 3, 12};
    a &= Vector4i{6, 3, 14, 7};
    a ^= Vector4i{5, 1, 10, 15};
    return a;
}

void VectorCpp14Test::moduloBitwiseConstexpr() {
    constexpr Vector4i a = moduloBitwise();
    CORRADE_COMPARE(a, (Vector4i{1, 2, 0, 11}));
}

void VectorCpp14Test::dotConstexpr() {
    constexpr Float a = Math::dot(Vector4{1.0f, 0.5f, 0.75f, 1.5f}, Vector4{2.0f, 4.0f, 1.0f, 7.0f});
    constexpr Float b = Vector4{1.0f, 2.0f, 3.0f, 4.0f}.dot();
    CORRADE_COMPARE(a, 15.25f);
    CORRADE_COMPARE(b, 30.0f);
}

void VectorCpp14Test::projectedConstexpr() {
    constexpr Vector3 a = Vector3{1.0f, 2.0f, 3.0f}.projected(Vector3{1.0f, -1.0f, 0.5f});
    constexpr Vector3 b = Vector3{1.0f, 2.0f, 3.0f}.projectedOntoNormalized(Vector3{0.0f, 0.6f, 0.8f});
    CORRADE_COMPARE(a, (Vector3{0.222222f, -0.222222f, 0.111111f}));
    CORRADE_COMPARE(b, (Vector3{0.0f, 2.16f, 2.88f}));
}

void VectorCpp14Test::sumProductConstexpr() {
    constexpr Float a = Vector4{1.0f, 2.0f, 3.0f, 4.0f}.sum();
    constexpr Float b = Vector4{1.0f, 2.0f, 3.0f, 4.0f}.product();
    constexpr Int c = Vector4i{1, 2, 3, -4}.sum();
    constexpr Int d = Vector4i{1, 2, 3, -4}.product();
    CORRADE_COMPARE(a, 10.0f);
    CORRADE_COMPARE(b, 24.0f);
    CORRADE_COMPARE(c, 2);
    CORRADE_COMPARE(d, -24);
}

void VectorCpp14Test::minMaxIntegralConstexpr() {
    constexpr Int a = Vector3i{1, -2, 3}.min();
    constexpr Int b = Vector3i{1, -2, 3}.max();
    constexpr Containers::Pair<Int, Int> c = Vector3i{1, -2, 3}.minmax();
    CORRADE_COMPARE(a, -2);
    CORRADE_COMPARE(b, 3);
    CORRADE_COMPARE(c, Containers::pair(-2, 3));
}

}}}}

CORRADE_TEST_MAIN(Magnum::Math::Test::VectorCpp14Test)

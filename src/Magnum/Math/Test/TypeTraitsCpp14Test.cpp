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

#include <limits>
#include <Corrade/TestSuite/Tester.h>

#include "Magnum/Magnum.h"
#include "Magnum/Math/TypeTraits.h"

namespace Magnum { namespace Math { namespace Test { namespace {

struct TypeTraitsCpp14Test: TestSuite::Tester {
    explicit TypeTraitsCpp14Test();

    template<class T> void equalsConstexpr();
    template<class T> void equalsZeroConstexpr();
    void equalConstexpr();
};

TypeTraitsCpp14Test::TypeTraitsCpp14Test() {
    addTests({&TypeTraitsCpp14Test::equalsConstexpr<Float>,
              &TypeTraitsCpp14Test::equalsConstexpr<Double>,
              &TypeTraitsCpp14Test::equalsConstexpr<long double>,

              &TypeTraitsCpp14Test::equalsZeroConstexpr<Float>,
              &TypeTraitsCpp14Test::equalsZeroConstexpr<Double>,
              &TypeTraitsCpp14Test::equalsZeroConstexpr<long double>,

              &TypeTraitsCpp14Test::equalConstexpr});
}

template<class T> void TypeTraitsCpp14Test::equalsConstexpr() {
    setTestCaseTemplateName(TypeTraits<T>::name());

    constexpr bool a = TypeTraits<T>::equals(T(0) + TypeTraits<T>::epsilon()/T(2), T(0));
    constexpr bool b = TypeTraits<T>::equals(T(0) + TypeTraits<T>::epsilon()*T(2), T(0));
    constexpr bool c = TypeTraits<T>::equals(T(1) + TypeTraits<T>::epsilon()/T(2), T(1));
    constexpr bool d = TypeTraits<T>::equals(T(1) + TypeTraits<T>::epsilon()*T(3), T(1));
    constexpr bool e = TypeTraits<T>::equals(T(25) + TypeTraits<T>::epsilon()*T(2), T(25));
    constexpr bool f = TypeTraits<T>::equals(T(25) + TypeTraits<T>::epsilon()*T(75), T(25));
    constexpr bool g = TypeTraits<T>::equals(std::numeric_limits<T>::infinity(), std::numeric_limits<T>::infinity());
    CORRADE_VERIFY(a);
    CORRADE_VERIFY(!b);
    CORRADE_VERIFY(c);
    CORRADE_VERIFY(!d);
    CORRADE_VERIFY(e);
    CORRADE_VERIFY(!f);
    CORRADE_VERIFY(g);
}

template<class T> void TypeTraitsCpp14Test::equalsZeroConstexpr() {
    setTestCaseTemplateName(TypeTraits<T>::name());

    constexpr bool a = TypeTraits<T>::equalsZero(T(0), T(1));
    constexpr bool b = TypeTraits<T>::equalsZero(TypeTraits<T>::epsilon()/T(2), T(1));
    constexpr bool c = TypeTraits<T>::equalsZero(TypeTraits<T>::epsilon()*T(2), T(1));
    constexpr bool d = TypeTraits<T>::equalsZero(TypeTraits<T>::epsilon()*T(1000), T(1000));
    constexpr bool e = TypeTraits<T>::equalsZero(TypeTraits<T>::epsilon()*T(3000), T(1000));
    CORRADE_VERIFY(a);
    CORRADE_VERIFY(b);
    CORRADE_VERIFY(!c);
    CORRADE_VERIFY(d);
    CORRADE_VERIFY(!e);
}

void TypeTraitsCpp14Test::equalConstexpr() {
    constexpr bool a = Math::equal(1.0f + TypeTraits<Float>::epsilon()/2.0f, 1.0f);
    constexpr bool b = Math::equal(1.0f + TypeTraits<Float>::epsilon()*3.0f, 1.0f);
    constexpr bool c = Math::notEqual(1.0f + TypeTraits<Float>::epsilon()/2.0f, 1.0f);
    constexpr bool d = Math::notEqual(1.0f + TypeTraits<Float>::epsilon()*3.0f, 1.0f);
    CORRADE_VERIFY(a);
    CORRADE_VERIFY(!b);
    CORRADE_VERIFY(!c);
    CORRADE_VERIFY(d);
}

}}}}

CORRADE_TEST_MAIN(Magnum::Math::Test::TypeTraitsCpp14Test)

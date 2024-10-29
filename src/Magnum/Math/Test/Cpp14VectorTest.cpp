/*
    This file is part of Magnum.

    Copyright © 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019,
                2020, 2021, 2022, 2023, 2024
              Vladimír Vondruš <mosra@centrum.cz>
    Copyright © 2024 Stanislaw Halik <sthalik@misaki.pl>

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

#include <cstdlib>
#include <sstream>
#include <Corrade/TestSuite/Tester.h>
#include <Corrade/TestSuite/Compare/Numeric.h>
//#include <Corrade/Utility/DebugStl.h>

#include "Magnum/Math/Vector.h"
#include "Magnum/Math/Vector2.h"
#include "Magnum/Math/Vector3.h"
#include "Magnum/Math/Vector4.h"

namespace Magnum { namespace Math {

//namespace Implementation {}

namespace Test { namespace {

struct Cpp14VectorTest: TestSuite::Tester {
    explicit Cpp14VectorTest();
    void basic2();
    void basic3();
    void basic4();
    void accessors2();
    void accessors3();
    void accessors4();
};

Cpp14VectorTest::Cpp14VectorTest() {
    addTests({
        &Cpp14VectorTest::basic2,
        &Cpp14VectorTest::basic3,
        &Cpp14VectorTest::basic4,
        &Cpp14VectorTest::accessors2,
        &Cpp14VectorTest::accessors3,
        &Cpp14VectorTest::accessors4,
    });
}

template<class Vec> constexpr Vec setX(Vec v) { v.x() = 11; return v; }
template<class Vec> constexpr Vec setY(Vec v) { v.y() = 22; return v; }
template<class Vec> constexpr Vec setZ(Vec v) { v.z() = 33; return v; }
template<class Vec> constexpr Vec setW(Vec v) { v.w() = 44; return v; }

template<class Vec, class Num> constexpr Vec setN(Vec v, std::size_t i, Num val) { v[i] = val; return v; }

void Cpp14VectorTest::basic2() {
    constexpr Vector2i v{1, 2};
    CORRADE_VERIFY(v.x() == 1);
    CORRADE_VERIFY(v.y() == 2);
    constexpr auto vx = setN(v, 0, 11);
    CORRADE_VERIFY(vx.x() == 11);
    CORRADE_VERIFY(vx.y() == 2);
    constexpr auto vy = setN(v, 1, 22);
    CORRADE_VERIFY(vy.x() == 1);
    CORRADE_VERIFY(vy.y() == 22);
}

void Cpp14VectorTest::basic3() {
    constexpr Vector3i v{1, 2, 3};
    CORRADE_VERIFY(v.x() == 1);
    CORRADE_VERIFY(v.y() == 2);
    CORRADE_VERIFY(v.z() == 3);
    {
        constexpr auto vx = setN(v, 0, 11);
        CORRADE_VERIFY(vx.x() == 11);
        CORRADE_VERIFY(vx.y() == 2);
        CORRADE_VERIFY(vx.z() == 3);
    }
    {
        constexpr auto vy = setN(v, 1, 22);
        CORRADE_VERIFY(vy.x() == 1);
        CORRADE_VERIFY(vy.y() == 22);
        CORRADE_VERIFY(vy.z() == 3);
    }
    {
        constexpr auto vz = setN(v, 2, 33);
        CORRADE_VERIFY(vz.x() == 1);
        CORRADE_VERIFY(vz.y() == 2);
        CORRADE_VERIFY(vz.z() == 33);
    }
}

void Cpp14VectorTest::basic4() {
    constexpr Vector4i v{1, 2, 3, 4};
    CORRADE_VERIFY(v.x() == 1);
    CORRADE_VERIFY(v.y() == 2);
    CORRADE_VERIFY(v.z() == 3);
    CORRADE_VERIFY(v.w() == 4);

    {
        constexpr auto vx = setN(v, 0, 11);
        CORRADE_VERIFY(vx.x() == 11);
        CORRADE_VERIFY(vx.y() == 2);
        CORRADE_VERIFY(vx.z() == 3);
        CORRADE_VERIFY(vx.w() == 4);
    }
    {
        constexpr auto vy = setN(v, 1, 22);
        CORRADE_VERIFY(vy.x() == 1);
        CORRADE_VERIFY(vy.y() == 22);
        CORRADE_VERIFY(vy.z() == 3);
        CORRADE_VERIFY(vy.w() == 4);
    }
    {
        constexpr auto vz = setN(v, 2, 33);
        CORRADE_VERIFY(vz.x() == 1);
        CORRADE_VERIFY(vz.y() == 2);
        CORRADE_VERIFY(vz.z() == 33);
        CORRADE_VERIFY(vz.w() == 4);
    }
    {
        constexpr auto vw = setN(v, 3, 44);
        CORRADE_VERIFY(vw.x() == 1);
        CORRADE_VERIFY(vw.y() == 2);
        CORRADE_VERIFY(vw.z() == 3);
        CORRADE_VERIFY(vw.w() == 44);
    }
}

void Cpp14VectorTest::accessors2() {
    constexpr Vector2i v{1, 2};
    CORRADE_VERIFY(v.x() == 1);
    CORRADE_VERIFY(v.y() == 2);
    {
        constexpr auto vx = setX(v);
        CORRADE_VERIFY(vx.x() == 11);
        CORRADE_VERIFY(vx.y() == 2);
    }
    {
        constexpr auto vy = setY(v);
        CORRADE_VERIFY(vy.x() == 1);
        CORRADE_VERIFY(vy.y() == 22);
    }
}

void Cpp14VectorTest::accessors3() {
    constexpr Vector3i v{1, 2, 3};
    CORRADE_VERIFY(v.x() == 1);
    CORRADE_VERIFY(v.y() == 2);
    CORRADE_VERIFY(v.z() == 3);
    {
        constexpr auto vx = setX(v);
        CORRADE_VERIFY(vx.x() == 11);
        CORRADE_VERIFY(vx.y() == 2);
        CORRADE_VERIFY(vx.z() == 3);
    }
    {
        constexpr auto vy = setY(v);
        CORRADE_VERIFY(vy.x() == 1);
        CORRADE_VERIFY(vy.y() == 22);
        CORRADE_VERIFY(vy.z() == 3);
    }
    {
        constexpr auto vz = setZ(v);
        CORRADE_VERIFY(vz.x() == 1);
        CORRADE_VERIFY(vz.y() == 2);
        CORRADE_VERIFY(vz.z() == 33);
    }
}

void Cpp14VectorTest::accessors4() {
    constexpr Vector4i v{1, 2, 3, 4};
    CORRADE_VERIFY(v.x() == 1);
    CORRADE_VERIFY(v.y() == 2);
    CORRADE_VERIFY(v.z() == 3);
    CORRADE_VERIFY(v.w() == 4);

    {
        constexpr auto vx = setX(v);
        CORRADE_VERIFY(vx.x() == 11);
        CORRADE_VERIFY(vx.y() == 2);
        CORRADE_VERIFY(vx.z() == 3);
        CORRADE_VERIFY(vx.w() == 4);
    }
    {
        constexpr auto vy = setY(v);
        CORRADE_VERIFY(vy.x() == 1);
        CORRADE_VERIFY(vy.y() == 22);
        CORRADE_VERIFY(vy.z() == 3);
        CORRADE_VERIFY(vy.w() == 4);
    }
    {
        constexpr auto vz = setZ(v);
        CORRADE_VERIFY(vz.x() == 1);
        CORRADE_VERIFY(vz.y() == 2);
        CORRADE_VERIFY(vz.z() == 33);
        CORRADE_VERIFY(vz.w() == 4);
    }
    {
        constexpr auto vw = setW(v);
        CORRADE_VERIFY(vw.x() == 1);
        CORRADE_VERIFY(vw.y() == 2);
        CORRADE_VERIFY(vw.z() == 3);
        CORRADE_VERIFY(vw.w() == 44);
    }
    {
        constexpr auto v_ = setW(setZ(setY(setX(v))));
        CORRADE_VERIFY(v_.x() == 11);
        CORRADE_VERIFY(v_.y() == 22);
        CORRADE_VERIFY(v_.z() == 33);
        CORRADE_VERIFY(v_.w() == 44);
    }
}

}}}}

CORRADE_TEST_MAIN(Magnum::Math::Test::Cpp14VectorTest)

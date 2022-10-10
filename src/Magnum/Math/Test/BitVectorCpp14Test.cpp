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

#include "Magnum/Math/BitVector.h"

namespace Magnum { namespace Math { namespace Test { namespace {

struct BitVectorCpp14Test: TestSuite::Tester {
    explicit BitVectorCpp14Test();

    void accessConstexpr();
    void setConstexpr();
    void convertBoolConstexpr();
    void allConstexpr();
    void resetConstexpr();
    void compareConstexpr();
    void noneConstexpr();
    void anyConstexpr();
    void bitInverseConstexpr();
    void bitAndOrXorConstexpr();
};

/* What's a typedef and not a using differs from the typedefs in root Magnum
   namespace, or is not present there at all */
typedef Math::BitVector<19> BitVector19;

BitVectorCpp14Test::BitVectorCpp14Test() {
    addTests({&BitVectorCpp14Test::accessConstexpr,
              &BitVectorCpp14Test::setConstexpr,
              &BitVectorCpp14Test::convertBoolConstexpr,
              &BitVectorCpp14Test::allConstexpr,
              &BitVectorCpp14Test::resetConstexpr,
              &BitVectorCpp14Test::compareConstexpr,
              &BitVectorCpp14Test::noneConstexpr,
              &BitVectorCpp14Test::anyConstexpr,
              &BitVectorCpp14Test::bitInverseConstexpr,
              &BitVectorCpp14Test::bitAndOrXorConstexpr});
}

/* Same condition as MAGNUM_BITVECTOR_SET_CONSTEXPR14 in BitVector.h */
#if defined(CORRADE_TARGET_GCC) && !defined(CORRADE_TARGET_CLANG) && __GNUC__*100 + __GNUC_MINOR__ < 602
#define SKIP_CONSTEXPR_SET
#endif

constexpr BitVector19 populate() {
    BitVector19 a;
    a.data()[2] = 0xee;
    a.data()[0] = 0xc0;
    a.data()[1] = 0xff;
    return a;
}

void BitVectorCpp14Test::accessConstexpr() {
    constexpr BitVector19 a = populate();
    CORRADE_COMPARE(a, (BitVector19{0xc0, 0xff, 0xee}));
}

#ifndef SKIP_CONSTEXPR_SET
constexpr BitVector19 setBit() {
    BitVector19 a{0x08, 0x03, 0x04};
    a.set(15);
    return a;
}
#endif

void BitVectorCpp14Test::setConstexpr() {
    #ifdef SKIP_CONSTEXPR_SET
    CORRADE_SKIP("Constexpr set() not supported by the compiler.");
    #else
    constexpr BitVector19 a = setBit();
    CORRADE_COMPARE(a, (BitVector19{0x08, 0x83, 0x04}));
    #endif
}

void BitVectorCpp14Test::convertBoolConstexpr() {
    constexpr bool a = bool(BitVector19{0xff, 0xff, 0x07});
    constexpr bool b = bool(BitVector19{0xff, 0xff, 0x04});
    CORRADE_VERIFY(a);
    CORRADE_VERIFY(!b);
}

void BitVectorCpp14Test::allConstexpr() {
    constexpr bool a = BitVector19{0xff, 0xff, 0x07}.all();
    constexpr bool b = BitVector19{0xff, 0xff, 0x0f}.all();
    constexpr bool c = BitVector19{0xff, 0xff, 0x04}.all();
    CORRADE_VERIFY(a);
    CORRADE_VERIFY(b);
    CORRADE_VERIFY(!c);
}

#ifndef SKIP_CONSTEXPR_SET
constexpr BitVector19 resetBit() {
    BitVector19 a{0x08, 0x83, 0x04};
    a.reset(15);
    return a;
}

constexpr BitVector19 setBitValue(bool value) {
    BitVector19 a{0x08, 0x03, 0x04};
    a.set(15, value);
    a.set(3, value);
    return a;
}
#endif

void BitVectorCpp14Test::resetConstexpr() {
    #ifdef SKIP_CONSTEXPR_SET
    CORRADE_SKIP("Constexpr reset() and set() not supported by the compiler.");
    #else
    constexpr BitVector19 a = resetBit();
    constexpr BitVector19 b = setBitValue(true);
    constexpr BitVector19 c = setBitValue(false);
    CORRADE_COMPARE(a, (BitVector19{0x08, 0x03, 0x04}));
    CORRADE_COMPARE(b, (BitVector19{0x08, 0x83, 0x04}));
    CORRADE_COMPARE(c, (BitVector19{0x00, 0x03, 0x04}));
    #endif
}

void BitVectorCpp14Test::compareConstexpr() {
    constexpr bool a = BitVector19{0xa5, 0x5f, 0x07} == BitVector19{0xa5, 0x5f, 0x07};
    constexpr bool b = BitVector19{0xa5, 0x5f, 0x07} != BitVector19{0xa5, 0x98, 0x07};
    constexpr bool c = BitVector19{0xa5, 0x5f, 0x07} != BitVector19{0xa5, 0x5f, 0x06};
    constexpr bool d = BitVector19{0xa5, 0x5f, 0x07} != BitVector19{0xa5, 0x5f, 0x07};
    CORRADE_VERIFY(a);
    CORRADE_VERIFY(b);
    CORRADE_VERIFY(c);
    CORRADE_VERIFY(!d);
}

void BitVectorCpp14Test::noneConstexpr() {
    constexpr bool a = BitVector19{0x00, 0x00, 0x00}.none();
    constexpr bool b = BitVector19{0x00, 0x00, 0x08}.none();
    constexpr bool c = BitVector19{0x00, 0x00, 0x04}.none();
    CORRADE_VERIFY(a);
    CORRADE_VERIFY(b);
    CORRADE_VERIFY(!c);
}

void BitVectorCpp14Test::anyConstexpr() {
    constexpr bool a = BitVector19{0x00, 0x01, 0x00}.any();
    constexpr bool b = BitVector19{0x00, 0x00, 0x04}.any();
    constexpr bool c = BitVector19{0x00, 0x00, 0x08}.any();
    CORRADE_VERIFY(a);
    CORRADE_VERIFY(b);
    CORRADE_VERIFY(!c);
}

void BitVectorCpp14Test::bitInverseConstexpr() {
    constexpr BitVector19 a = ~BitVector19{0xa5, 0x5f, 0x03};
    constexpr BitVector19 b = !BitVector19{0xa5, 0x5f, 0x03};
    CORRADE_COMPARE(a, (BitVector19{0x5a, 0xa0, 0x04}));
    CORRADE_COMPARE(b, (BitVector19{0x5a, 0xa0, 0x04}));
}

constexpr BitVector19 andOrXorAssign() {
    BitVector19 a{0xa5, 0x5f, 0x03};
    a &= BitVector19{0x37, 0xf3, 0x06};
    a |= BitVector19{0x80, 0x04, 0x01};
    a ^= BitVector19{0xff, 0x0f, 0x01};
    return a;
}

void BitVectorCpp14Test::bitAndOrXorConstexpr() {
    constexpr BitVector19 a = BitVector19{0xa5, 0x5f, 0x03} & BitVector19{0x37, 0xf3, 0x06};
    constexpr BitVector19 b = BitVector19{0xa5, 0x5f, 0x03} && BitVector19{0x37, 0xf3, 0x06};
    constexpr BitVector19 c = BitVector19{0xa5, 0x5f, 0x03} | BitVector19{0x37, 0xf3, 0x06};
    constexpr BitVector19 d = BitVector19{0xa5, 0x5f, 0x03} || BitVector19{0x37, 0xf3, 0x06};
    constexpr BitVector19 e = BitVector19{0xa5, 0x5f, 0x03} ^ BitVector19{0x37, 0xf3, 0x06};
    constexpr BitVector19 f = andOrXorAssign();
    CORRADE_COMPARE(a, (BitVector19{0x25, 0x53, 0x02}));
    CORRADE_COMPARE(b, (BitVector19{0x25, 0x53, 0x02}));
    CORRADE_COMPARE(c, (BitVector19{0xb7, 0xff, 0x07}));
    CORRADE_COMPARE(d, (BitVector19{0xb7, 0xff, 0x07}));
    CORRADE_COMPARE(e, (BitVector19{0x92, 0xac, 0x05}));
    CORRADE_COMPARE(f, (BitVector19{0x5a, 0x58, 0x02}));
}

}}}}

CORRADE_TEST_MAIN(Magnum::Math::Test::BitVectorCpp14Test)

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

#include <cstdint>
#include <functional>
#include <limits>
#include <type_traits>
#include <utility>
#include <Corrade/Containers/ArrayView.h> /* arraySize() */
#include <Corrade/Containers/String.h>
#include <Corrade/TestSuite/Tester.h>
#include <Corrade/Utility/Format.h>

#include "Magnum/Math/BitVector.h"
#include "Magnum/Math/Color.h"
#include "Magnum/Math/Functions.h"
#include "Magnum/Math/Matrix.h"
#include "Magnum/Math/StrictWeakOrdering.h"

namespace Magnum { namespace Math { namespace Test { namespace {

struct BitVectorUnpackedTest: TestSuite::Tester {
    explicit BitVectorUnpackedTest();

    void resetFailures();

    template<std::size_t size> void storage();
    template<std::size_t size> void construct();
    template<std::size_t size> void constructConstexpr();
    template<std::size_t size> void access();
    template<std::size_t size> void bitwise();
    template<std::size_t size> void compare();
    template<std::size_t size> void allAnyNone();
    template<std::size_t size> void lerp();
    void operationsConstexpr();

    template<std::size_t size> void compareVector();
    template<std::size_t size> void compareVectorMasks();
    template<std::size_t cols, std::size_t rows> void compareMatrix();
    void compareVectorConstexpr();

    template<std::size_t size> void constantOperands();
    template<std::size_t size> void constantOperandsOneLane();
    template<std::size_t size> void constantBytesOneLane();
    void constantOperandsMatrix();

    template<std::size_t size> void constantLaneReductions();
    template<std::size_t size> void runtimeLaneAgainstConstants();
    template<std::size_t size> void runtimeLaneLoopMasks();

    template<std::size_t size> void equalVector();
    template<std::size_t size> void isNanIsInfVector();
    template<std::size_t size> void lerpVector();
    void lerpVectorReturnType();

    template<std::size_t size> void debug();
    template<std::size_t size> void strictWeakOrdering();
};

#define TEST_SIZE(test, size) &BitVectorUnpackedTest::test<size>,
#define TEST_SIZES(test)                                                    \
    TEST_SIZE(test, 1) TEST_SIZE(test, 2) TEST_SIZE(test, 3)                \
    TEST_SIZE(test, 4) TEST_SIZE(test, 5) TEST_SIZE(test, 6)                \
    TEST_SIZE(test, 7) TEST_SIZE(test, 8) TEST_SIZE(test, 9)                \
    TEST_SIZE(test, 15) TEST_SIZE(test, 16) TEST_SIZE(test, 17)             \
    TEST_SIZE(test, 24) TEST_SIZE(test, 32) TEST_SIZE(test, 33)             \
    TEST_SIZE(test, 80)

BitVectorUnpackedTest::BitVectorUnpackedTest() {
    addTests<BitVectorUnpackedTest>({
        TEST_SIZES(storage)
        TEST_SIZES(construct)
        TEST_SIZES(constructConstexpr)
        TEST_SIZES(access)
        TEST_SIZES(bitwise)
        TEST_SIZES(compare)
        TEST_SIZES(allAnyNone)
        TEST_SIZES(lerp)
        &BitVectorUnpackedTest::operationsConstexpr,

        TEST_SIZES(compareVector)
        TEST_SIZES(compareVectorMasks)
        &BitVectorUnpackedTest::compareMatrix<2, 2>,
        &BitVectorUnpackedTest::compareMatrix<2, 3>,
        &BitVectorUnpackedTest::compareMatrix<3, 2>,
        &BitVectorUnpackedTest::compareMatrix<2, 4>,
        &BitVectorUnpackedTest::compareMatrix<4, 2>,
        &BitVectorUnpackedTest::compareMatrix<3, 3>,
        &BitVectorUnpackedTest::compareMatrix<3, 4>,
        &BitVectorUnpackedTest::compareMatrix<4, 3>,
        &BitVectorUnpackedTest::compareMatrix<4, 4>,
        &BitVectorUnpackedTest::compareVectorConstexpr,

        TEST_SIZES(constantOperands)
        TEST_SIZES(constantOperandsOneLane)
        TEST_SIZES(constantBytesOneLane)
        &BitVectorUnpackedTest::constantOperandsMatrix,

        TEST_SIZES(constantLaneReductions)
        TEST_SIZES(runtimeLaneAgainstConstants)
        TEST_SIZES(runtimeLaneLoopMasks)

        TEST_SIZES(equalVector)
        TEST_SIZES(isNanIsInfVector)
        TEST_SIZES(lerpVector)
        &BitVectorUnpackedTest::lerpVectorReturnType,

        TEST_SIZES(debug)
        TEST_SIZES(strictWeakOrdering)
        }, &BitVectorUnpackedTest::resetFailures,
           &BitVectorUnpackedTest::resetFailures);
}

#undef TEST_SIZES
#undef TEST_SIZE

constexpr std::size_t NoLane = ~std::size_t{};

std::size_t failures;

volatile std::uint64_t volatileZero = 0;
volatile Float volatileHalf = 0.5f;
volatile Float volatileTwo = 2.0f;
volatile Int volatileTwoInt = 2;

/* Lets the test cases that MSVC 19.52 and 19.53 miscompile outside Debug builds expect a failure. Later versions
   are untested and expect it too */
#if defined(CORRADE_TARGET_MSVC) && !defined(CORRADE_TARGET_CLANG_CL) && _MSC_VER >= 1952 && !defined(CORRADE_IS_DEBUG_BUILD)
#define MSVC_CONSTANT_ALL_BUG
#endif
/* Lets the test case that GCC 16.2 miscompiles with SSE4.1 or later outside Debug builds expect a failure. Later
   versions are untested and expect it too */
#if defined(CORRADE_TARGET_GCC) && !defined(CORRADE_TARGET_CLANG) && (__GNUC__ > 16 || (__GNUC__ == 16 && __GNUC_MINOR__ >= 2)) && defined(__SSE4_1__) && !defined(CORRADE_IS_DEBUG_BUILD)
#define GCC_MASK_NOT_EQUAL_BUG
#endif

template<std::size_t size> constexpr bool unpackedAt16() {
    return size == 16 && size <= Implementation::BitVectorMaxUnpackedSize;
}

void BitVectorUnpackedTest::resetFailures() {
    failures = 0;
}

CORRADE_NEVER_INLINE void fail(const char* what, std::size_t size, std::size_t lane) {
    if(++failures > 4)
        return;
    Debug d;
    d << "   " << what << "failed at size" << size;
    if(lane != NoLane)
        d << "lane" << lane;
}

#define CHECK(what, lane, expression, expected)                             \
    do {                                                                    \
        if(bool(expression) != bool(expected))                              \
            fail(what, size, lane);                                         \
    } while(false)

/* MurmurHash3's 64-bit finalizer */
constexpr std::uint64_t mixStep(std::uint64_t x) { return x ^ (x >> 33); }
constexpr std::uint64_t mix(std::uint64_t x) {
    return mixStep(mixStep(mixStep(x)*0xff51afd7ed558ccdull)*0xc4ceb9fe1a85ec53ull);
}

constexpr std::uint64_t pow3(std::size_t n) {
    return n == 0 ? 1 : 3*pow3(n - 1);
}

/* Bits of byte `i` that hold lanes of a bit vector of `size` lanes */
constexpr unsigned usedBits(std::size_t size, std::size_t i) {
    return i*8 + 8 <= size ? 0xffu : 0xffu >> (i*8 + 8 - size);
}

/* Byte `i` of a bit vector of `size` lanes that are all `value` except
   `lane`, with the bits past `size` set if `padding` */
constexpr UnsignedByte laneByte(std::size_t size, std::size_t lane, bool value, bool padding, std::size_t i) {
    return UnsignedByte((((value ? 0xffu : 0u) ^ (i == lane/8 ? 1u << lane%8 : 0u)) & usedBits(size, i)) | (padding ? 0xffu & ~usedBits(size, i) : 0u));
}

template<std::size_t size, std::size_t ...i> constexpr BitVector<size> laneBytes(std::size_t lane, bool value, bool padding, Containers::Implementation::Sequence<i...>) {
    return BitVector<size>{laneByte(size, lane, value, padding, i)...};
}

template<std::size_t size> constexpr BitVector<size> laneBytes(std::size_t lane, bool value, bool padding) {
    return laneBytes<size>(lane, value, padding, typename Containers::Implementation::GenerateSequence<BitVector<size>::DataSize>::Type{});
}

template<std::size_t size, std::size_t ...i> BitVector<size> fromBytes(const UnsignedByte* bytes, Containers::Implementation::Sequence<i...>) {
    return BitVector<size>{bytes[i]...};
}

/* Every lane up to `maxSize` lanes and at 16, the first, middle and last
   otherwise */
template<std::size_t size, std::size_t maxSize, bool = (size <= maxSize || size == 16)> struct Lanes {
    typedef typename Containers::Implementation::GenerateSequence<size>::Type Type;
};
template<std::size_t size, std::size_t maxSize> struct Lanes<size, maxSize, false> {
    typedef Containers::Implementation::Sequence<0, size/2, size - 1> Type;
};

template<class T, class = void> struct HasData: std::false_type {};
template<class T> struct HasData<T, decltype(void(std::declval<T&>().data()))>: std::true_type {};

/* Sets the bits past `size` in the last byte, which every operation has to
   ignore */
template<std::size_t size> void setPadding(BitVector<size>& v, std::true_type) {
    v.data()[BitVector<size>::DataSize - 1] |= UnsignedByte(0xffu & ~usedBits(size, BitVector<size>::DataSize - 1));
}
template<std::size_t size> void setPadding(BitVector<size>&, std::false_type) {}

template<std::size_t size> struct Bits {
    bool value[size];
};

template<std::size_t size> Bits<size> bitsFromInteger(std::uint64_t x) {
    Bits<size> out{};
    for(std::size_t i = 0; i != size; ++i)
        out.value[i] = (x >> i%64) & 1;
    return out;
}

template<std::size_t size> Bits<size> bitsRandom(std::uint64_t seed) {
    Bits<size> out{};
    for(std::size_t i = 0; i != size; ++i)
        out.value[i] = (mix(seed*131 + i + 7) >> 9) & 1;
    return out;
}

/* Every lane `value` except `lane`, which is the opposite */
template<std::size_t size> Bits<size> bitsExcept(bool value, std::size_t lane) {
    Bits<size> out{};
    for(std::size_t i = 0; i != size; ++i)
        out.value[i] = value != (i == lane);
    return out;
}

template<std::size_t size> BitVector<size> bitVector(const Bits<size>& bits, bool padding) {
    BitVector<size> out{ZeroInit};
    for(std::size_t i = 0; i != size; ++i)
        out.set(i, bits.value[i]);
    if(padding)
        setPadding(out, HasData<BitVector<size>>{});
    return out;
}

template<std::size_t size> UnsignedByte packedByte(const Bits<size>& bits, std::size_t byte) {
    UnsignedByte out = 0;
    for(std::size_t i = 0; i != 8 && byte*8 + i != size; ++i)
        out |= UnsignedByte(bits.value[byte*8 + i] << i);
    return out;
}

/* Every lane `value` except `lane`, `lane + 64`, `lane + 128` and so on, which
   are the opposite */
template<std::size_t size> Bits<size> bitsExceptModulo64(bool value, std::size_t lane) {
    Bits<size> out{};
    for(std::size_t i = 0; i != size; ++i)
        out.value[i] = value != (i % 64 == lane);
    return out;
}

constexpr std::size_t lanesBelow64(std::size_t size) {
    return size < 64 ? size : 64;
}

/* Calls f(a, b, t, paddingA, paddingB) with every `a` up to exhaustiveSize
   lanes. Above, with each pattern of bitsExceptModulo64() and random ones. */
template<std::size_t size, std::size_t exhaustiveSize, class F> void forEachTriple(F f) {
    const std::uint64_t z = volatileZero;
    if(size <= exhaustiveSize) {
        for(std::uint64_t x = 0; x != std::uint64_t{1} << (size <= exhaustiveSize ? size : 0); ++x) {
            const Bits<size> a = bitsFromInteger<size>(x ^ z);
            Bits<size> flipped = a;
            flipped.value[x % size] = !flipped.value[x % size];
            const bool pa = x & 1, pb = x & 2;
            f(a, a, bitsRandom<size>(x), pa, pb);
            f(a, flipped, bitsRandom<size>(x + 1), pb, pa);
            f(a, bitsRandom<size>(x + 2), bitsFromInteger<size>(~x), pa, !pb);
        }
        return;
    }

    const Bits<size> ones = bitsExcept<size>(true, NoLane), zeros = bitsExcept<size>(false, NoLane);
    for(std::size_t lane = 0; lane != lanesBelow64(size); ++lane) {
        const bool pa = lane & 1, pb = lane & 2;
        const Bits<size> one = bitsExceptModulo64<size>(false, lane), but = bitsExceptModulo64<size>(true, lane);
        f(but, ones, bitsRandom<size>(lane ^ z), pa, pb);
        f(one, zeros, bitsRandom<size>(lane + 3), pa, pb);
        f(ones, but, one, pa, pb);
        f(zeros, one, but, pa, pb);
        f(but, but, one, pa, pb);
    }
    for(std::uint64_t k = 0; k != 64; ++k)
        f(bitsRandom<size>(k ^ z), bitsRandom<size>(k + 100), bitsRandom<size>(k + 200), k & 1, k & 2);
    for(int p = 0; p != 4; ++p) {
        f(ones, ones, ones, p & 1, p & 2);
        f(zeros, zeros, zeros, p & 1, p & 2);
    }
}

template<std::size_t size> void BitVectorUnpackedTest::storage() {
    setTestCaseTemplateName(Utility::format("{}", size));

    CORRADE_COMPARE(sizeof(BitVector<size>), size <= Implementation::BitVectorMaxUnpackedSize ? size : (size + 7)/8);
    CORRADE_COMPARE(alignof(BitVector<size>), 1);
    CORRADE_COMPARE(std::size_t(BitVector<size>::DataSize), (size + 7)/8);
    CORRADE_COMPARE(HasData<BitVector<size>>::value, size > Implementation::BitVectorMaxUnpackedSize);
    CORRADE_VERIFY(std::is_standard_layout<BitVector<size>>::value);
    #ifndef CORRADE_NO_STD_IS_TRIVIALLY_TRAITS
    CORRADE_VERIFY(std::is_trivially_copyable<BitVector<size>>::value);
    #endif
}

template<std::size_t size> void BitVectorUnpackedTest::construct() {
    setTestCaseTemplateName(Utility::format("{}", size));

    const BitVector<size> defaulted, zeroInit{ZeroInit}, ones{true}, zeros{false};
    for(std::size_t i = 0; i != size; ++i) {
        CHECK("default", i, defaulted[i], false);
        CHECK("ZeroInit", i, zeroInit[i], false);
        CHECK("{true}", i, ones[i], true);
        CHECK("{false}", i, zeros[i], false);
    }
    CHECK("default none", NoLane, defaulted.none(), true);
    CHECK("ZeroInit none", NoLane, zeroInit.none(), true);
    CHECK("{true} all", NoLane, ones.all(), true);
    CHECK("{false} none", NoLane, zeros.none(), true);

    /* Zeros, ones, random bytes, then one lane set or reset at every lane */
    for(std::size_t k = 0; k != 18 + 2*size; ++k) {
        UnsignedByte bytes[BitVector<size>::DataSize], padded[BitVector<size>::DataSize];
        for(std::size_t j = 0; j != BitVector<size>::DataSize; ++j) {
            bytes[j] = k == 0 ? 0 : k == 1 ? 0xff : k < 18 ?
                UnsignedByte(mix(k*977 + j + volatileZero) >> 13) :
                laneByte(size, (k - 18)/2, (k - 18) % 2, false, j);
            padded[j] = bytes[j];
        }
        padded[BitVector<size>::DataSize - 1] |= UnsignedByte(0xffu & ~usedBits(size, BitVector<size>::DataSize - 1));
        const BitVector<size> a = fromBytes<size>(bytes, typename Containers::Implementation::GenerateSequence<BitVector<size>::DataSize>::Type{});
        const BitVector<size> b = fromBytes<size>(padded, typename Containers::Implementation::GenerateSequence<BitVector<size>::DataSize>::Type{});
        bool all = true, any = false;
        for(std::size_t i = 0; i != size; ++i) {
            const bool expected = (bytes[i/8] >> i%8) & 1;
            CHECK("bytes", i, a[i], expected);
            CHECK("bytes with padding", i, b[i], expected);
            all = all && expected;
            any = any || expected;
        }
        CHECK("bytes ==", NoLane, a == b, true);
        CHECK("bytes !=", NoLane, a != b, false);
        CHECK("bytes ^ none", NoLane, (a ^ b).none(), true);
        CHECK("bytes all", NoLane, b.all(), all);
        CHECK("bytes any", NoLane, b.any(), any);
        CHECK("bytes none", NoLane, b.none(), !any);
        CHECK("bytes ~ none", NoLane, (~b).none(), all);
    }

    CORRADE_COMPARE(failures, 0);
}

template<std::size_t size, std::size_t ...i> constexpr BitVector<size> constexprBytes(std::uint64_t seed, bool padding, Containers::Implementation::Sequence<i...>) {
    return BitVector<size>{UnsignedByte((mix(seed + i) & usedBits(size, i)) | (padding ? 0xffu & ~usedBits(size, i) : 0u))...};
}

template<std::size_t size> constexpr bool constexprBytesMatch(const BitVector<size>& v, std::uint64_t seed, std::size_t i) {
    return i == size || (v[i] == bool((mix(seed + i/8) >> i%8) & 1) && constexprBytesMatch(v, seed, i + 1));
}

template<std::size_t size> void BitVectorUnpackedTest::constructConstexpr() {
    setTestCaseTemplateName(Utility::format("{}", size));

    constexpr BitVector<size> defaulted, zeroInit{ZeroInit}, ones{true}, zeros{false};
    constexpr bool defaultedLast = defaulted[size - 1];
    constexpr bool zeroInitLast = zeroInit[size - 1];
    constexpr bool onesFirst = ones[0];
    constexpr bool onesLast = ones[size - 1];
    constexpr bool zerosLast = zeros[size - 1];
    CORRADE_VERIFY(!defaultedLast);
    CORRADE_VERIFY(!zeroInitLast);
    CORRADE_VERIFY(onesFirst);
    CORRADE_VERIFY(onesLast);
    CORRADE_VERIFY(!zerosLast);

    constexpr BitVector<size> a = constexprBytes<size>(17, false, typename Containers::Implementation::GenerateSequence<BitVector<size>::DataSize>::Type{});
    constexpr BitVector<size> b = constexprBytes<size>(17, true, typename Containers::Implementation::GenerateSequence<BitVector<size>::DataSize>::Type{});
    constexpr bool aMatches = constexprBytesMatch(a, 17, 0);
    constexpr bool bMatches = constexprBytesMatch(b, 17, 0);
    CORRADE_VERIFY(aMatches);
    CORRADE_VERIFY(bMatches);
}

template<std::size_t size> void BitVectorUnpackedTest::access() {
    setTestCaseTemplateName(Utility::format("{}", size));

    for(std::uint64_t k = 0; k != 8; ++k) {
        const Bits<size> bits = k == 0 ? bitsExcept<size>(false, NoLane) : k == 1 ? bitsExcept<size>(true, NoLane) : bitsRandom<size>(k ^ volatileZero);
        const BitVector<size> a = bitVector(bits, k & 1);
        for(std::size_t i = 0; i != size; ++i) {
            CHECK("[]", i, a[i], bits.value[i]);

            Bits<size> setBits = bits, resetBits = bits;
            setBits.value[i] = true;
            resetBits.value[i] = false;

            BitVector<size> d = a;
            CHECK("set() []", i, d.set(i)[i], true);
            CHECK("set() ==", i, d == bitVector(setBits, false), true);
            CHECK("reset() []", i, d.reset(i)[i], false);
            CHECK("reset() ==", i, d == bitVector(resetBits, false), true);
            CHECK("set(true) ==", i, d.set(i, true) == bitVector(setBits, false), true);
            CHECK("set(false) ==", i, d.set(i, false) == bitVector(resetBits, false), true);
            CHECK("set(same) ==", i, d.set(i, bits.value[i]) == a, true);
            CHECK("set(opposite) !=", i, d.set(i, !bits.value[i]) != a, true);
            CHECK("set(opposite) []", i, d[i], !bits.value[i]);
        }
    }

    CORRADE_COMPARE(failures, 0);
}

template<std::size_t size> void BitVectorUnpackedTest::bitwise() {
    setTestCaseTemplateName(Utility::format("{}", size));

    forEachTriple<size, 12>([](const Bits<size>& ra, const Bits<size>& rb, const Bits<size>&, bool pa, bool pb) {
        const BitVector<size> a = bitVector(ra, pa), b = bitVector(rb, pb);
        BitVector<size> andEq = a, orEq = a, xorEq = a;
        andEq &= b;
        orEq |= b;
        xorEq ^= b;
        const BitVector<size> and_ = a & b, or_ = a | b, xor_ = a ^ b, land = a && b, lor = a || b, inv = ~a, not_ = !a;
        for(std::size_t i = 0; i != size; ++i) {
            const bool x = ra.value[i], y = rb.value[i];
            CHECK("&", i, and_[i], x && y);
            CHECK("&=", i, andEq[i], x && y);
            CHECK("&&", i, land[i], x && y);
            CHECK("|", i, or_[i], x || y);
            CHECK("|=", i, orEq[i], x || y);
            CHECK("||", i, lor[i], x || y);
            CHECK("^", i, xor_[i], x != y);
            CHECK("^=", i, xorEq[i], x != y);
            CHECK("~", i, inv[i], !x);
            CHECK("!", i, not_[i], !x);
        }
    });

    CORRADE_COMPARE(failures, 0);
}

template<std::size_t size> void BitVectorUnpackedTest::compare() {
    setTestCaseTemplateName(Utility::format("{}", size));

    forEachTriple<size, 16>([](const Bits<size>& ra, const Bits<size>& rb, const Bits<size>&, bool pa, bool pb) {
        const BitVector<size> a = bitVector(ra, pa), b = bitVector(rb, pb), c = bitVector(ra, !pa);
        bool equal = true;
        for(std::size_t i = 0; i != size; ++i)
            equal = equal && ra.value[i] == rb.value[i];
        CHECK("==", NoLane, a == b, equal);
        CHECK("!=", NoLane, a != b, !equal);
        CHECK("== swapped", NoLane, b == a, equal);
        CHECK("!= swapped", NoLane, b != a, !equal);
        CHECK("== itself", NoLane, a == a, true);
        CHECK("!= itself", NoLane, a != a, false);
        CHECK("== other padding", NoLane, a == c, true);
        CHECK("!= other padding", NoLane, a != c, false);
    });

    CORRADE_COMPARE(failures, 0);
}

template<std::size_t size> void BitVectorUnpackedTest::allAnyNone() {
    setTestCaseTemplateName(Utility::format("{}", size));

    forEachTriple<size, 16>([](const Bits<size>& ra, const Bits<size>& rb, const Bits<size>&, bool pa, bool pb) {
        const BitVector<size> a = bitVector(ra, pa), b = bitVector(rb, pb);
        bool all = true, any = false, andAll = true, orAll = true, xorAny = false;
        for(std::size_t i = 0; i != size; ++i) {
            all = all && ra.value[i];
            any = any || ra.value[i];
            andAll = andAll && ra.value[i] && rb.value[i];
            orAll = orAll && (ra.value[i] || rb.value[i]);
            xorAny = xorAny || ra.value[i] != rb.value[i];
        }
        CHECK("all", NoLane, a.all(), all);
        CHECK("bool", NoLane, bool(a), all);
        CHECK("any", NoLane, a.any(), any);
        CHECK("none", NoLane, a.none(), !any);
        CHECK("~ none", NoLane, (~a).none(), all);
        CHECK("! any", NoLane, (!a).any(), !all);
        CHECK("& all", NoLane, (a & b).all(), andAll);
        CHECK("| all", NoLane, (a | b).all(), orAll);
        CHECK("^ any", NoLane, (a ^ b).any(), xorAny);
        CHECK("^ none", NoLane, (a ^ b).none(), !xorAny);
        CHECK("| ~ all", NoLane, (a | ~a).all(), true);
        CHECK("& ~ none", NoLane, (a & ~a).none(), true);
    });

    CORRADE_COMPARE(failures, 0);
}

template<std::size_t size> void BitVectorUnpackedTest::lerp() {
    setTestCaseTemplateName(Utility::format("{}", size));

    forEachTriple<size, 12>([](const Bits<size>& ra, const Bits<size>& rb, const Bits<size>& rt, bool pa, bool pb) {
        const BitVector<size> a = bitVector(ra, pa), b = bitVector(rb, pb), t = bitVector(rt, pa != pb);
        const BitVector<size> l = Math::lerp(a, b, t);
        Bits<size> rl;
        bool all = true, any = false;
        for(std::size_t i = 0; i != size; ++i) {
            rl.value[i] = rt.value[i] ? rb.value[i] : ra.value[i];
            CHECK("lerp", i, l[i], rl.value[i]);
            all = all && rl.value[i];
            any = any || rl.value[i];
        }
        CHECK("lerp all", NoLane, l.all(), all);
        CHECK("lerp any", NoLane, l.any(), any);
        CHECK("lerp ==", NoLane, l == bitVector(rl, false), true);
        CHECK("lerp == padded", NoLane, l == bitVector(rl, true), true);
        CHECK("lerp != padded", NoLane, l != bitVector(rl, true), false);
    });

    CORRADE_COMPARE(failures, 0);
}

/* Same condition as MAGNUM_BITVECTOR_SET_CONSTEXPR14 in BitVector.h */
#if defined(CORRADE_TARGET_GCC) && !defined(CORRADE_TARGET_CLANG) && __GNUC__*100 + __GNUC_MINOR__ < 602
#define SKIP_CONSTEXPR_SET
#endif

#if CORRADE_CXX_STANDARD >= 201402 && !defined(SKIP_CONSTEXPR_SET)
#define CHECK_CONSTEXPR(...)                                                \
    do {                                                                    \
        if(!(__VA_ARGS__))                                                  \
            return int(size)*10000 + __LINE__;                              \
    } while(false)

template<int value> using Constant = std::integral_constant<int, value>;

constexpr std::uint64_t sample(std::uint64_t k) {
    return k == 0 ? 0 : k == 1 ? ~std::uint64_t{} : mix(k*0x9e3779b97f4a7c15ull);
}

/* Up to 16 lanes the bits of x, above a hash of x and the lane */
constexpr bool referenceBit(std::size_t size, std::uint64_t x, std::size_t i) {
    return size <= 16 ? (x >> i) & 1 : x == 0 ? false : x == ~std::uint64_t{} ? true : (mix(x + i*0x2545f4914f6cdd1dull) >> 17) & 1;
}

template<std::size_t size> constexpr BitVector<size> referenceVector(std::uint64_t x) {
    BitVector<size> v;
    for(std::size_t i = 0; i != size; ++i)
        v.set(i, referenceBit(size, x, i));
    return v;
}

template<std::size_t size> constexpr int bitOpsConstexpr(std::uint64_t x, std::uint64_t y) {
    const BitVector<size> a = referenceVector<size>(x), b = referenceVector<size>(y);
    BitVector<size> andEq = a, orEq = a, xorEq = a;
    andEq &= b;
    orEq |= b;
    xorEq ^= b;
    const BitVector<size> and_ = a & b, or_ = a | b, xor_ = a ^ b, land = a && b, lor = a || b, inv = ~a, not_ = !a;
    bool all = true, any = false, equal = true;
    for(std::size_t i = 0; i != size; ++i) {
        const bool ra = referenceBit(size, x, i), rb = referenceBit(size, y, i);
        CHECK_CONSTEXPR(a[i] == ra);
        CHECK_CONSTEXPR(and_[i] == (ra && rb) && andEq[i] == (ra && rb) && land[i] == (ra && rb));
        CHECK_CONSTEXPR(or_[i] == (ra || rb) && orEq[i] == (ra || rb) && lor[i] == (ra || rb));
        CHECK_CONSTEXPR(xor_[i] == (ra != rb) && xorEq[i] == (ra != rb));
        CHECK_CONSTEXPR(inv[i] == !ra && not_[i] == !ra);
        all = all && ra;
        any = any || ra;
        equal = equal && ra == rb;
    }
    CHECK_CONSTEXPR(a.all() == all && bool(a) == all && a.any() == any && a.none() == !any);
    CHECK_CONSTEXPR((a == b) == equal && (a != b) == !equal && a == a && !(a != a));
    CHECK_CONSTEXPR((~a).all() == !any && (a | ~a).all() && (a & ~a).none());
    CHECK_CONSTEXPR(Math::lerp(a, b, a ^ b) == b);
    for(std::size_t i = 0; i != size; ++i) {
        BitVector<size> d = a;
        CHECK_CONSTEXPR(d.set(i)[i]);
        CHECK_CONSTEXPR(!d.reset(i)[i]);
        CHECK_CONSTEXPR(d.set(i, referenceBit(size, x, i)) == a);
        CHECK_CONSTEXPR(d.set(i, !referenceBit(size, x, i)) != a);
        CHECK_CONSTEXPR(d[i] == !referenceBit(size, x, i));
    }
    return 0;
}

template<std::size_t size, std::size_t ...i> constexpr BitVector<size> bytesConstexpr(std::uint64_t x, bool padding, Containers::Implementation::Sequence<i...>) {
    return BitVector<size>{UnsignedByte(((x >> (i%8)*8) & usedBits(size, i)) | (padding ? 0xffu & ~usedBits(size, i) : 0u))...};
}

template<std::size_t size> constexpr int bytesConstexpr(std::uint64_t x) {
    const BitVector<size> a = bytesConstexpr<size>(x, false, typename Containers::Implementation::GenerateSequence<BitVector<size>::DataSize>::Type{});
    const BitVector<size> b = bytesConstexpr<size>(x, true, typename Containers::Implementation::GenerateSequence<BitVector<size>::DataSize>::Type{});
    bool all = true, any = false;
    for(std::size_t i = 0; i != size; ++i) {
        CHECK_CONSTEXPR(a[i] == bool((x >> i%64) & 1));
        CHECK_CONSTEXPR(b[i] == bool((x >> i%64) & 1));
        all = all && ((x >> i%64) & 1);
        any = any || ((x >> i%64) & 1);
    }
    CHECK_CONSTEXPR(b == a && !(b != a) && b.all() == all && b.any() == any && b.none() == !any);
    CHECK_CONSTEXPR((~b).none() == all && (b ^ a).none());
    CHECK_CONSTEXPR(BitVector<size>{true}.all() && BitVector<size>{false}.none() && BitVector<size>{}.none() && BitVector<size>{ZeroInit}.none());
    return 0;
}

template<std::size_t size> constexpr int reduceConstexpr() {
    for(std::size_t k = 0; k != size; ++k) {
        BitVector<size> t{true}, o;
        t.reset(k);
        o.set(k);
        CHECK_CONSTEXPR(!t.all() && t.any() == (size > 1) && t.none() == (size == 1));
        CHECK_CONSTEXPR(o.any() && !o.none() && o.all() == (size == 1));
        CHECK_CONSTEXPR(t == ~o && !(t != ~o) && !(t == o) && t != o);
    }
    CHECK_CONSTEXPR(BitVector<size>{true}.all() && !BitVector<size>{}.any());
    return 0;
}

template<std::size_t size, class T> constexpr Vector<size, T> vectorFromBase3Constexpr(std::uint64_t x) {
    Vector<size, T> v;
    for(std::size_t i = 0; i != size; ++i)
        v[i] = T(int((x/pow3(i)) % 3) - 1);
    return v;
}

template<std::size_t size, class T> constexpr Vector<size, T> rampConstexpr(int start, int step) {
    Vector<size, T> v;
    for(std::size_t i = 0; i != size; ++i)
        v[i] = T(start + int(i)*step);
    return v;
}

template<std::size_t size> constexpr BitVector<size> everyThirdConstexpr(std::size_t phase) {
    BitVector<size> v;
    for(std::size_t i = 0; i != size; ++i)
        v.set(i, (i + phase) % 3 == 0);
    return v;
}

/* Vectors first to last - 1, numbered in base 3, each against every vector */
template<std::size_t size, class T> constexpr int compareVectorsConstexpr(std::uint64_t first, std::uint64_t last) {
    for(std::uint64_t x = first; x != last; ++x) for(std::uint64_t y = 0; y != pow3(size); ++y) {
        const Vector<size, T> a = vectorFromBase3Constexpr<size, T>(x), b = vectorFromBase3Constexpr<size, T>(y);
        const BitVector<size> lt = a < b, le = a <= b, ge = a >= b, gt = a > b;
        const Vector<size, T> l = Math::lerp(a, b, lt), fromMask{lt};
        const BitVector<size> ml = Math::lerp(lt, ge, le);
        for(std::size_t i = 0; i != size; ++i) {
            CHECK_CONSTEXPR(lt[i] == (a[i] < b[i]) && le[i] == (a[i] <= b[i]) && ge[i] == (a[i] >= b[i]) && gt[i] == (a[i] > b[i]));
            CHECK_CONSTEXPR(l[i] == (a[i] < b[i] ? b[i] : a[i]) && fromMask[i] == T(a[i] < b[i]));
            CHECK_CONSTEXPR(ml[i] == (le[i] ? ge[i] : lt[i]));
        }
        CHECK_CONSTEXPR((a < b && b < a).none() && bool(a <= b || a > b));
        CHECK_CONSTEXPR((a >= b).all() == !(a < b).any() && bool(a > b) == (b < a).all());
    }
    return 0;
}

template<std::size_t size, class T> constexpr int lerpMasksConstexpr() {
    const Vector<size, T> a = vectorFromBase3Constexpr<size, T>(5), b = vectorFromBase3Constexpr<size, T>(pow3(size) - 7);
    for(std::uint64_t t = 0; t != (1u << size); ++t) {
        const Vector<size, T> l = Math::lerp(a, b, referenceVector<size>(t));
        for(std::size_t i = 0; i != size; ++i)
            CHECK_CONSTEXPR(l[i] == ((t >> i) & 1 ? b[i] : a[i]));
    }
    return 0;
}

template<std::size_t size, class T> constexpr int compareRampsConstexpr() {
    const Vector<size, T> a = rampConstexpr<size, T>(0, 1), b = rampConstexpr<size, T>(int(size) - 1, -1);
    const BitVector<size> lt = a < b, le = a <= b, ge = a >= b, gt = a > b;
    for(std::size_t i = 0; i != size; ++i)
        CHECK_CONSTEXPR(lt[i] == (a[i] < b[i]) && le[i] == (a[i] <= b[i]) && ge[i] == (a[i] >= b[i]) && gt[i] == (a[i] > b[i]));
    CHECK_CONSTEXPR(!(a < a).any() && (a <= a).all() && (a >= a).all() && (a > a).none());
    return 0;
}

template<std::size_t size, class T> constexpr int lerpRampsConstexpr() {
    const Vector<size, T> a = rampConstexpr<size, T>(1, 2), b = rampConstexpr<size, T>(-5, 3);
    for(std::size_t p = 0; p != 3; ++p) {
        const BitVector<size> t = everyThirdConstexpr<size>(p);
        const Vector<size, T> l = Math::lerp(a, b, t);
        const BitVector<size> m = Math::lerp(everyThirdConstexpr<size>(p + 1), everyThirdConstexpr<size>(p + 2), t);
        for(std::size_t i = 0; i != size; ++i) {
            CHECK_CONSTEXPR(l[i] == (t[i] ? b[i] : a[i]));
            CHECK_CONSTEXPR(m[i] == (t[i] ? everyThirdConstexpr<size>(p + 2)[i] : everyThirdConstexpr<size>(p + 1)[i]));
        }
    }
    return 0;
}
#endif

void BitVectorUnpackedTest::operationsConstexpr() {
    #if CORRADE_CXX_STANDARD < 201402 || defined(SKIP_CONSTEXPR_SET)
    CORRADE_SKIP("C++14 constexpr or constexpr set() not supported by the compiler.");
    #else
    /* Each one a separate constant evaluation, to stay under the step
       limits. A nonzero value is size*10000 + the line of the check. */
    const int results[]{
        Constant<bitOpsConstexpr<1>(0, 0)>::value,
        Constant<bitOpsConstexpr<1>(1, 0)>::value,
        Constant<bitOpsConstexpr<1>(0, 1)>::value,
        Constant<bitOpsConstexpr<1>(1, 1)>::value,
        Constant<bitOpsConstexpr<2>(1, 2)>::value,
        Constant<bitOpsConstexpr<2>(3, 2)>::value,
        Constant<bitOpsConstexpr<3>(5, 2)>::value,
        Constant<bitOpsConstexpr<3>(7, 6)>::value,
        Constant<bitOpsConstexpr<4>(9, 6)>::value,
        Constant<bitOpsConstexpr<4>(15, 15)>::value,
        Constant<bitOpsConstexpr<5>(sample(2), sample(3))>::value,
        Constant<bitOpsConstexpr<6>(sample(4), sample(1))>::value,
        Constant<bitOpsConstexpr<7>(sample(5), sample(5))>::value,
        Constant<bitOpsConstexpr<8>(sample(6), sample(7))>::value,
        Constant<bitOpsConstexpr<8>(sample(1), sample(0))>::value,
        Constant<bitOpsConstexpr<9>(sample(8), sample(9))>::value,
        Constant<bitOpsConstexpr<9>(sample(1), sample(1))>::value,
        Constant<bitOpsConstexpr<12>(sample(10), sample(11))>::value,
        Constant<bitOpsConstexpr<15>(sample(12), sample(13))>::value,
        Constant<bitOpsConstexpr<16>(sample(14), sample(15))>::value,
        Constant<bitOpsConstexpr<16>(sample(1), sample(16))>::value,
        Constant<bitOpsConstexpr<17>(sample(17), sample(18))>::value,
        Constant<bitOpsConstexpr<19>(sample(19), sample(19))>::value,
        Constant<bitOpsConstexpr<24>(sample(20), sample(21))>::value,
        Constant<bitOpsConstexpr<33>(sample(22), sample(23))>::value,
        Constant<bitOpsConstexpr<80>(sample(24), sample(25))>::value,
        Constant<bytesConstexpr<1>(sample(2))>::value,
        Constant<bytesConstexpr<3>(sample(3))>::value,
        Constant<bytesConstexpr<8>(sample(4))>::value,
        Constant<bytesConstexpr<9>(sample(5))>::value,
        Constant<bytesConstexpr<16>(sample(6))>::value,
        Constant<bytesConstexpr<17>(sample(7))>::value,
        Constant<bytesConstexpr<24>(sample(8))>::value,
        Constant<bytesConstexpr<80>(sample(9))>::value,
        Constant<reduceConstexpr<1>()>::value,
        Constant<reduceConstexpr<3>()>::value,
        Constant<reduceConstexpr<4>()>::value,
        Constant<reduceConstexpr<5>()>::value,
        Constant<reduceConstexpr<8>()>::value,
        Constant<reduceConstexpr<9>()>::value,
        Constant<reduceConstexpr<11>()>::value,
        Constant<reduceConstexpr<12>()>::value,
        Constant<reduceConstexpr<16>()>::value,
        Constant<reduceConstexpr<17>()>::value,
        Constant<reduceConstexpr<24>()>::value,
        Constant<reduceConstexpr<32>()>::value,
    };
    for(std::size_t i = 0; i != Containers::arraySize(results); ++i) {
        CORRADE_ITERATION(i);
        CORRADE_COMPARE(results[i], 0);
    }
    #endif
}

template<std::size_t size, class T, class F> void forEachVectorPair(F f) {
    const std::uint64_t z = volatileZero;
    if(size <= 4) {
        for(std::uint64_t x = 0; x != pow3(size <= 4 ? size : 0); ++x) for(std::uint64_t y = 0; y != pow3(size <= 4 ? size : 0); ++y) {
            Vector<size, T> a, b;
            for(std::size_t i = 0; i != size; ++i) {
                a[i] = T(int(((x ^ z)/pow3(i)) % 3) - 1);
                b[i] = T(int(((y ^ z)/pow3(i)) % 3) - 1);
            }
            f(a, b);
        }
        return;
    }

    /* Includes values that differ by less than the fuzzy compare epsilon */
    const double values[]{-1.0, 0.0, 1.0, 1.0000001, 1.001, 2.5};
    for(std::uint64_t r = 0; r != (size <= 64 ? 300 : 40); ++r) {
        Vector<size, T> a, b;
        for(std::size_t i = 0; i != size; ++i) {
            a[i] = T(values[mix(z + r*977 + i) % 6]);
            b[i] = T(values[mix(z + r*977 + i + 50000) % 6]);
        }
        if(r % 7 == 0)
            b = a;
        if(r % 7 == 1) {
            b = a;
            b[r % size] = T(values[(r + 1) % 6]);
        }
        f(a, b);
    }
    for(std::size_t lane = 0; lane != lanesBelow64(size); ++lane) {
        Vector<size, T> a{T(1 + z)};
        const Vector<size, T> b{T(1 + z)};
        for(std::size_t i = lane; i < size; i += 64)
            a[i] = T(2 + z);
        f(a, b);
        f(b, a);
    }
}

template<std::size_t size> struct CompareVectorCheck {
    template<class T> void operator()(const Vector<size, T>& a, const Vector<size, T>& b) const {
        const BitVector<size> lt = a < b, le = a <= b, ge = a >= b, gt = a > b;
        const Vector<size, T> fromMask{lt};
        bool ltAny = false, leAll = true;
        for(std::size_t i = 0; i != size; ++i) {
            CHECK("<", i, lt[i], a[i] < b[i]);
            CHECK("<=", i, le[i], a[i] <= b[i]);
            CHECK(">=", i, ge[i], a[i] >= b[i]);
            CHECK(">", i, gt[i], a[i] > b[i]);
            CHECK("Vector(BitVector)", i, fromMask[i] == T(a[i] < b[i]), true);
            ltAny = ltAny || a[i] < b[i];
            leAll = leAll && a[i] <= b[i];
        }
        CHECK("< any", NoLane, lt.any(), ltAny);
        CHECK("< none", NoLane, (a < b).none(), !ltAny);
        CHECK("<= all", NoLane, (a <= b).all(), leAll);
        CHECK("bool(<=)", NoLane, bool(a <= b), leAll);
        CHECK(">= all", NoLane, (a >= b).all(), !ltAny);
        CHECK("> all", NoLane, bool(a > b), (b < a).all());
        CHECK("< && >=", NoLane, ((a < b) && (a >= b)).none(), true);
        CHECK("< || >=", NoLane, ((a < b) || (a >= b)).all(), true);
        CHECK("< && swapped <", NoLane, (a < b && b < a).none(), true);
        CHECK("<= || >", NoLane, bool(a <= b || a > b), true);
    }
};

template<std::size_t size> void BitVectorUnpackedTest::compareVector() {
    setTestCaseTemplateName(Utility::format("{}", size));

    forEachVectorPair<size, Int>(CompareVectorCheck<size>{});
    forEachVectorPair<size, Float>(CompareVectorCheck<size>{});
    forEachVectorPair<size, Double>(CompareVectorCheck<size>{});

    /* MSVC 19.52 and 19.53 miscompile this at 16 */
    #ifdef MSVC_CONSTANT_ALL_BUG
    CORRADE_EXPECT_FAIL_IF(unpackedAt16<size>(), "MSVC 19.52 and 19.53 compute all() of 16 known lanes as false.");
    #endif
    CORRADE_COMPARE(failures, 0);
}

template<std::size_t size> CORRADE_NEVER_INLINE void compareVectorMasks(const Bits<size>& p, const Bits<size>& q, Int seed) {
    Vector<size, Int> a, b, c;
    for(std::size_t i = 0; i != size; ++i) {
        a[i] = Int(p.value[i]) ^ seed;
        b[i] = Int(q.value[i]) ^ seed;
        c[i] = seed;
    }
    const BitVector<size> mp = a > c, mq = b > c;
    bool all = true, any = false, equal = true;
    for(std::size_t i = 0; i != size; ++i) {
        all = all && p.value[i];
        any = any || p.value[i];
        equal = equal && p.value[i] == q.value[i];
    }
    CHECK("all", NoLane, mp.all(), all);
    CHECK("any", NoLane, mp.any(), any);
    CHECK("none", NoLane, mp.none(), !any);
    CHECK("==", NoLane, mp == mq, equal);
    CHECK("!=", NoLane, mp != mq, !equal);
    CHECK("^ any", NoLane, (mp ^ mq).any(), !equal);
    const BitVector<size> and_ = mp & mq, or_ = mp | mq, xor_ = mp ^ mq, inv = ~mp;
    for(std::size_t i = 0; i != size; ++i) {
        CHECK("&", i, and_[i], p.value[i] && q.value[i]);
        CHECK("|", i, or_[i], p.value[i] || q.value[i]);
        CHECK("^", i, xor_[i], p.value[i] != q.value[i]);
        CHECK("~", i, inv[i], !p.value[i]);
    }
}

template<std::size_t size> void BitVectorUnpackedTest::compareVectorMasks() {
    setTestCaseTemplateName(Utility::format("{}", size));

    const std::uint64_t z = volatileZero;
    const Int seed = Int(z);
    if(size <= 6) {
        for(std::uint64_t p = 0; p != std::uint64_t{1} << (size <= 6 ? size : 0); ++p) for(std::uint64_t q = 0; q != std::uint64_t{1} << (size <= 6 ? size : 0); ++q)
            Test::compareVectorMasks<size>(bitsFromInteger<size>(p ^ z), bitsFromInteger<size>(q ^ z), seed);
    } else {
        /* Every p up to 12 lanes, otherwise zeros, ones and one lane set or
           reset at every lane up to 64 lanes, or at three above */
        const std::size_t laneCount = size <= 64 ? size : 3;
        const std::size_t lanes[]{0, size/2, size - 1};
        const std::size_t count = size <= 12 ? std::size_t{1} << (size <= 12 ? size : 0) : 2 + 2*laneCount + 16;
        for(std::size_t k = 0; k != count; ++k) {
            const std::size_t lane = size <= 64 ? (k - 2)/2 : lanes[((k - 2)/2) % 3];
            const Bits<size> p = size <= 12 ? bitsFromInteger<size>(k ^ z) :
                k < 2 ? bitsExcept<size>(k == 1, NoLane) :
                k < 2 + 2*laneCount ? bitsExcept<size>(k % 2, lane) :
                bitsRandom<size>(k ^ z);
            Bits<size> flipped = p;
            flipped.value[k % size] = !flipped.value[k % size];
            Test::compareVectorMasks<size>(p, p, seed);
            Test::compareVectorMasks<size>(p, flipped, seed);
            Test::compareVectorMasks<size>(p, bitsExcept<size>(false, NoLane), seed);
            Test::compareVectorMasks<size>(p, bitsExcept<size>(true, NoLane), seed);
            for(std::uint64_t r = 0; r != 4; ++r)
                Test::compareVectorMasks<size>(p, bitsRandom<size>(k*4 + r + 1000), seed);
        }
    }

    CORRADE_COMPARE(failures, 0);
}

/* A matrix with every element equal and then one at run time */
template<std::size_t cols, std::size_t rows> CORRADE_NEVER_INLINE void compareMatrixRuntimeElement(Float lo) {
    constexpr std::size_t size = cols*rows;
    RectangularMatrix<cols, rows, Float> a{ZeroInit}, b{ZeroInit};
    for(std::size_t i = 0; i != cols; ++i) for(std::size_t j = 0; j != rows; ++j)
        a[i][j] = b[i][j] = 1.0f;
    CHECK("all(<=)", NoLane, (a <= b).all(), true);
    CHECK("any(<)", NoLane, (a < b).any(), false);
    a[cols - 1][rows - 1] = lo;
    CHECK("all(>=) run-time", NoLane, (a >= b).all(), false);
    CHECK("any(<) run-time", NoLane, (a < b).any(), true);
    CHECK("none(>) run-time", NoLane, (a > b).none(), true);
}

template<std::size_t cols, std::size_t rows> void BitVectorUnpackedTest::compareMatrix() {
    setTestCaseTemplateName({Utility::format("{}", cols), Utility::format("{}", rows)});

    constexpr std::size_t size = cols*rows;
    const std::uint64_t z = volatileZero;
    for(std::uint64_t x = 0; x != 200; ++x) {
        RectangularMatrix<cols, rows, Float> a, b;
        for(std::size_t c = 0; c != cols; ++c) for(std::size_t r = 0; r != rows; ++r) {
            a[c][r] = Float(int(mix(x*16 + c*4 + r + 1) % 3) - 1);
            b[c][r] = Float(int(mix(x*16 + c*4 + r + 1 + z + 977) % 3) - 1);
        }
        const BitVector<size> lt = a < b, le = a <= b, ge = a >= b, gt = a > b;
        for(std::size_t c = 0; c != cols; ++c) for(std::size_t r = 0; r != rows; ++r) {
            CHECK("<", c*rows + r, lt[c*rows + r], a[c][r] < b[c][r]);
            CHECK("<=", c*rows + r, le[c*rows + r], a[c][r] <= b[c][r]);
            CHECK(">=", c*rows + r, ge[c*rows + r], a[c][r] >= b[c][r]);
            CHECK(">", c*rows + r, gt[c*rows + r], a[c][r] > b[c][r]);
        }
    }
    compareMatrixRuntimeElement<cols, rows>(volatileHalf);

    /* MSVC 19.52 and 19.53 miscompile this at 4x4 */
    #ifdef MSVC_CONSTANT_ALL_BUG
    CORRADE_EXPECT_FAIL_IF(unpackedAt16<size>(), "MSVC 19.52 and 19.53 compute all() of 16 known lanes as false.");
    #endif
    CORRADE_COMPARE(failures, 0);
}

void BitVectorUnpackedTest::compareVectorConstexpr() {
    #if CORRADE_CXX_STANDARD < 201402 || defined(SKIP_CONSTEXPR_SET)
    CORRADE_SKIP("C++14 constexpr or constexpr set() not supported by the compiler.");
    #else
    const int results[]{
        Constant<compareVectorsConstexpr<2, Int>(0, 9)>::value,
        Constant<compareVectorsConstexpr<2, Float>(0, 9)>::value,
        Constant<compareVectorsConstexpr<3, Int>(0, 9)>::value,
        Constant<compareVectorsConstexpr<3, Int>(9, 18)>::value,
        Constant<compareVectorsConstexpr<3, Int>(18, 27)>::value,
        Constant<compareVectorsConstexpr<3, Float>(0, 9)>::value,
        Constant<compareVectorsConstexpr<3, Float>(18, 27)>::value,
        Constant<compareVectorsConstexpr<4, Int>(0, 3)>::value,
        Constant<compareVectorsConstexpr<4, Int>(39, 42)>::value,
        Constant<compareVectorsConstexpr<4, Float>(78, 81)>::value,
        Constant<lerpMasksConstexpr<2, Int>()>::value,
        Constant<lerpMasksConstexpr<3, Float>()>::value,
        Constant<lerpMasksConstexpr<4, Int>()>::value,
        Constant<compareRampsConstexpr<1, Int>()>::value,
        Constant<compareRampsConstexpr<2, Float>()>::value,
        Constant<compareRampsConstexpr<5, Int>()>::value,
        Constant<compareRampsConstexpr<5, Float>()>::value,
        Constant<compareRampsConstexpr<8, Float>()>::value,
        Constant<compareRampsConstexpr<9, Int>()>::value,
        Constant<compareRampsConstexpr<16, Int>()>::value,
        Constant<compareRampsConstexpr<17, Float>()>::value,
        Constant<compareRampsConstexpr<32, Int>()>::value,
        Constant<lerpRampsConstexpr<1, Int>()>::value,
        Constant<lerpRampsConstexpr<2, Float>()>::value,
        Constant<lerpRampsConstexpr<5, Int>()>::value,
        Constant<lerpRampsConstexpr<8, Int>()>::value,
        Constant<lerpRampsConstexpr<9, Float>()>::value,
        Constant<lerpRampsConstexpr<16, Float>()>::value,
        Constant<lerpRampsConstexpr<17, Int>()>::value,
        Constant<lerpRampsConstexpr<32, Float>()>::value,
    };
    for(std::size_t i = 0; i != Containers::arraySize(results); ++i) {
        CORRADE_ITERATION(i);
        CORRADE_COMPARE(results[i], 0);
    }
    #endif
}

template<std::size_t size> struct EqualVectorCheck {
    template<class T> void operator()(const Vector<size, T>& a, const Vector<size, T>& b) const {
        const BitVector<size> eq = equal(a, b), ne = notEqual(a, b);
        bool all = true;
        for(std::size_t i = 0; i != size; ++i) {
            const bool e = TypeTraits<T>::equals(a[i], b[i]);
            CHECK("equal", i, eq[i], e);
            CHECK("notEqual", i, ne[i], !e);
            all = all && e;
        }
        CHECK("equal all", NoLane, equal(a, b).all(), all);
        CHECK("notEqual any", NoLane, notEqual(a, b).any(), !all);
        CHECK("notEqual none", NoLane, notEqual(a, b).none(), all);
        CHECK("equal == ~notEqual", NoLane, equal(a, b) == ~notEqual(a, b), true);
    }
};

template<std::size_t size> void BitVectorUnpackedTest::equalVector() {
    setTestCaseTemplateName(Utility::format("{}", size));

    forEachVectorPair<size, Int>(EqualVectorCheck<size>{});
    forEachVectorPair<size, Float>(EqualVectorCheck<size>{});
    forEachVectorPair<size, Double>(EqualVectorCheck<size>{});

    CORRADE_COMPARE(failures, 0);
}

#if defined(__FAST_MATH__) || (defined(__FINITE_MATH_ONLY__) && __FINITE_MATH_ONLY__) || defined(_M_FP_FAST)
#define SKIP_NAN_INF
#endif

template<std::size_t size> void BitVectorUnpackedTest::isNanIsInfVector() {
    setTestCaseTemplateName(Utility::format("{}", size));

    #ifdef SKIP_NAN_INF
    CORRADE_SKIP("NaN and infinity can't be tested with fast math.");
    #else
    const Float z = Float(volatileZero);
    const Float nan = z + std::numeric_limits<Float>::quiet_NaN();
    const Float inf = z + std::numeric_limits<Float>::infinity();
    CHECK("isNan finite", NoLane, isNan(Vector<size, Float>{1.0f + z}).none(), true);
    CHECK("isInf finite", NoLane, isInf(Vector<size, Float>{1.0f + z}).none(), true);
    CHECK("isNan all", NoLane, isNan(Vector<size, Float>{nan}).all(), true);
    CHECK("isInf all", NoLane, isInf(Vector<size, Float>{inf}).all(), true);

    /* A NaN at lane k and -inf at lane j, every j up to 33 lanes, otherwise
       three j for every k */
    for(std::size_t k = 0; k != size; ++k) for(std::size_t m = 0; m != (size <= 33 ? size : 3); ++m) {
        const std::size_t j = size <= 33 ? m : m == 0 ? k : m == 1 ? (k + 1) % size : size - 1 - k;
        Vector<size, Float> v{1.0f + z};
        v[k] = nan;
        if(j != k)
            v[j] = -inf;
        const BitVector<size> n = isNan(v), f = isInf(v);
        for(std::size_t i = 0; i != size; ++i) {
            CHECK("isNan", i, n[i], i == k);
            CHECK("isInf", i, f[i], i == j && j != k);
        }
        CHECK("isNan any", k, n.any(), true);
        CHECK("isNan all", k, n.all(), size == 1);
        CHECK("isInf any", k, f.any(), j != k);
    }

    CORRADE_COMPARE(failures, 0);
    #endif
}

template<std::size_t size> struct LerpVectorCheck {
    template<class T> void operator()(const Vector<size, T>& a, const Vector<size, T>& b) const {
        const BitVector<size> lt = a < b, le = a <= b, ge = a >= b;
        const Vector<size, T> l = Math::lerp(a, b, lt);
        const BitVector<size> m = Math::lerp(lt, ge, le);
        for(std::size_t i = 0; i != size; ++i) {
            CHECK("lerp(<)", i, l[i] == (a[i] < b[i] ? b[i] : a[i]), true);
            CHECK("lerp(<, >=, <=)", i, m[i], le[i] ? ge[i] : lt[i]);
        }
    }
};

template<std::size_t size> void BitVectorUnpackedTest::lerpVector() {
    setTestCaseTemplateName(Utility::format("{}", size));

    forEachVectorPair<size, Int>(LerpVectorCheck<size>{});
    forEachVectorPair<size, Float>(LerpVectorCheck<size>{});

    /* Every mask up to 8 lanes, otherwise one lane set or reset at every lane
       and random ones */
    const std::uint64_t z = volatileZero;
    Vector<size, Int> ai, bi;
    Vector<size, Float> af, bf;
    for(std::size_t i = 0; i != size; ++i) {
        ai[i] = Int(i + z);
        bi[i] = -Int(i + z) - 1;
        af[i] = Float(i + z) + 0.5f;
        bf[i] = -Float(i + z);
    }
    const std::size_t count = size <= 8 ? std::size_t{1} << (size <= 8 ? size : 0) : 2*size + 16;
    for(std::size_t k = 0; k != count; ++k) {
        const Bits<size> bits = size <= 8 ? bitsFromInteger<size>(k ^ z) :
            k < 2*size ? bitsExcept<size>(k % 2, k/2) : bitsRandom<size>(k ^ z);
        const BitVector<size> t = bitVector(bits, k & 1);
        const Vector<size, Int> li = Math::lerp(ai, bi, t);
        const Vector<size, Float> lf = Math::lerp(af, bf, t);
        for(std::size_t i = 0; i != size; ++i) {
            CHECK("lerp Int", i, li[i] == (bits.value[i] ? bi[i] : ai[i]), true);
            CHECK("lerp Float", i, lf[i] == (bits.value[i] ? bf[i] : af[i]), true);
        }
    }

    CORRADE_COMPARE(failures, 0);
}

void BitVectorUnpackedTest::lerpVectorReturnType() {
    CORRADE_VERIFY((std::is_same<decltype(Math::lerp(std::declval<Vector<1, Float>>(), std::declval<Vector<1, Float>>(), std::declval<BitVector<1>>())), Vector<1, Float>>::value));
    CORRADE_VERIFY((std::is_same<decltype(Math::lerp(std::declval<Vector<5, Int>>(), std::declval<Vector<5, Int>>(), std::declval<BitVector<5>>())), Vector<5, Int>>::value));
    CORRADE_VERIFY((std::is_same<decltype(Math::lerp(std::declval<Color3<Float>>(), std::declval<Color3<Float>>(), std::declval<BitVector<3>>())), Vector3<Float>>::value));
    CORRADE_VERIFY((std::is_same<decltype(Math::lerp(std::declval<Color4<Float>>(), std::declval<Color4<Float>>(), std::declval<BitVector<4>>())), Vector4<Float>>::value));
    CORRADE_VERIFY((std::is_same<decltype(Math::lerp(std::declval<Vector<32, Int>>(), std::declval<Vector<32, Int>>(), std::declval<BitVector<32>>())), Vector<32, Int>>::value));
    CORRADE_VERIFY((std::is_same<decltype(Math::lerp(std::declval<BitVector<16>>(), std::declval<BitVector<16>>(), std::declval<BitVector<16>>())), BitVector<16>>::value));
    CORRADE_VERIFY((std::is_same<decltype(equal(std::declval<Vector<5, Float>>(), std::declval<Vector<5, Float>>())), BitVector<5>>::value));
    CORRADE_VERIFY((std::is_same<decltype(notEqual(std::declval<Vector<5, Int>>(), std::declval<Vector<5, Int>>())), BitVector<5>>::value));

    const std::uint64_t seed = volatileZero;
    const Color3<Float> a{Float(seed % 3), 0.5f, 1.0f}, b{0.25f, Float(seed % 5), 2.0f};
    const Color4<Float> c{1.0f, 2.0f, 3.0f, Float(seed % 7)}, d{4.0f, 5.0f, 6.0f, 7.0f};
    for(std::uint64_t m = 0; m != 16; ++m) {
        CORRADE_ITERATION(m);
        BitVector<3> t3;
        BitVector<4> t4;
        for(std::size_t i = 0; i != 3; ++i)
            t3.set(i, ((m ^ seed) >> i) & 1);
        for(std::size_t i = 0; i != 4; ++i)
            t4.set(i, ((m ^ seed) >> i) & 1);
        const Vector3<Float> l3 = Math::lerp(a, b, t3);
        const Vector4<Float> l4 = Math::lerp(c, d, t4);
        for(std::size_t i = 0; i != 3; ++i) {
            CORRADE_ITERATION(i);
            CORRADE_COMPARE(l3[i], t3[i] ? b[i] : a[i]);
        }
        for(std::size_t i = 0; i != 4; ++i) {
            CORRADE_ITERATION(i);
            CORRADE_COMPARE(l4[i], t4[i] ? d[i] : c[i]);
        }
    }
}

std::size_t append(char* out, std::size_t n, const char* string) {
    for(; *string; ++string)
        out[n++] = *string;
    return n;
}

template<std::size_t size> void BitVectorUnpackedTest::debug() {
    setTestCaseTemplateName(Utility::format("{}", size));

    for(std::uint64_t k = 0; k != 12; ++k) {
        CORRADE_ITERATION(k);
        const Bits<size> bits = k == 0 ? bitsExcept<size>(false, NoLane) :
            k == 1 ? bitsExcept<size>(true, NoLane) :
            k == 2 ? bitsExcept<size>(false, 0) :
            k == 3 ? bitsExcept<size>(false, size - 1) :
            bitsRandom<size>(k ^ volatileZero);
        const BitVector<size> v = bitVector(bits, k & 1);

        char expected[16 + 13*BitVector<size>::DataSize];
        std::size_t n = append(expected, 0, "BitVector(0b");
        for(std::size_t byte = 0; byte != BitVector<size>::DataSize; ++byte) {
            const std::size_t count = byte*8 + 8 <= size ? 8 : size - byte*8;
            for(std::size_t i = 0; i != count; ++i)
                expected[n++] = bits.value[byte*8 + count - i - 1] ? '1' : '0';
            n = append(expected, n, byte + 1 == BitVector<size>::DataSize ? ")\n" : ", 0b");
        }

        Containers::String out;
        Debug{&out} << v;
        CORRADE_COMPARE(out, (Containers::String{expected, n}));
    }
}

template<std::size_t size> bool lessByPackedBytes(const Bits<size>& a, const Bits<size>& b) {
    for(std::size_t i = 0; i != BitVector<size>::DataSize; ++i) {
        const UnsignedByte ai = packedByte(a, i), bi = packedByte(b, i);
        if(ai != bi)
            return ai < bi;
    }
    return false;
}

template<std::size_t size> void BitVectorUnpackedTest::strictWeakOrdering() {
    setTestCaseTemplateName(Utility::format("{}", size));

    const std::uint64_t z = volatileZero;
    StrictWeakOrdering o;
    for(std::uint64_t i = 0; i != 40; ++i) for(std::uint64_t j = 0; j != 40; ++j) {
        const std::uint64_t k = j ^ (i & 3);
        const Bits<size> ra = i < 2 ? bitsExcept<size>(i == 1, NoLane) : bitsRandom<size>(i ^ z);
        const Bits<size> rb = k < 2 ? bitsExcept<size>(k == 1, NoLane) : bitsRandom<size>(k ^ z);
        const BitVector<size> a = bitVector(ra, i & 1), b = bitVector(rb, j & 1);
        CHECK("<", NoLane, o(a, b), lessByPackedBytes(ra, rb));
        CHECK("< itself", NoLane, o(a, a), false);
        CHECK("< other padding", NoLane, o(a, bitVector(ra, !(i & 1))), false);
    }

    /* Two vectors that differ only in the lane */
    for(std::size_t lane = 0; lane != size; ++lane) {
        const Bits<size> ra = bitsRandom<size>(lane + 50 + z);
        Bits<size> rb = ra;
        rb.value[lane] = !rb.value[lane];
        const BitVector<size> a = bitVector(ra, lane & 1), b = bitVector(rb, lane & 2);
        CHECK("< one lane", lane, o(a, b), !ra.value[lane]);
        CHECK("> one lane", lane, o(b, a), ra.value[lane]);
    }

    CORRADE_COMPARE(failures, 0);
}

/* Every operand a compile-time constant */
template<std::size_t size> CORRADE_NEVER_INLINE void constantOperands() {
    const BitVector<size> t{true}, f{false}, d{};
    CHECK("all", NoLane, t.all(), true);
    CHECK("bool", NoLane, bool(t), true);
    CHECK("any", NoLane, t.any(), true);
    CHECK("none", NoLane, t.none(), false);
    CHECK("false none", NoLane, f.none(), true);
    CHECK("false any", NoLane, f.any(), false);
    CHECK("false all", NoLane, f.all(), false);
    CHECK("default none", NoLane, d.none(), true);
    CHECK("~false all", NoLane, (~f).all(), true);
    CHECK("!false all", NoLane, (!f).all(), true);
    CHECK("~true none", NoLane, (~t).none(), true);
    CHECK("true == true", NoLane, t == t, true);
    CHECK("false == false", NoLane, f == f, true);
    CHECK("true != true", NoLane, t != t, false);
    CHECK("true != false", NoLane, t != f, true);
    CHECK("true == false", NoLane, t == f, false);
    CHECK("& all", NoLane, (t & t).all(), true);
    CHECK("&& all", NoLane, (t && t).all(), true);
    CHECK("| all", NoLane, (f | t).all(), true);
    CHECK("|| all", NoLane, (f || t).all(), true);
    CHECK("^ all", NoLane, (t ^ f).all(), true);
    CHECK("^ none", NoLane, (t ^ t).none(), true);
    CHECK("& none", NoLane, (t & f).none(), true);
    CHECK("lerp all", NoLane, Math::lerp(f, t, t).all(), true);
    CHECK("lerp none", NoLane, Math::lerp(t, f, t).none(), true);
    BitVector<size> s = f;
    for(std::size_t i = 0; i != size; ++i)
        s.set(i);
    CHECK("set() all", NoLane, s.all(), true);
    CHECK("set() ==", NoLane, s == t, true);
    BitVector<size> r = t;
    r.reset(size - 1);
    CHECK("reset() all", NoLane, r.all(), false);
    CHECK("reset() any", NoLane, r.any(), size > 1);
    CHECK("reset() !=", NoLane, r != t, true);
    const BitVector<size> t2{true}, f2{false};
    CHECK("true == true2", NoLane, t == t2, true);
    CHECK("false == false2", NoLane, f == f2, true);
    CHECK("true != true2", NoLane, t != t2, false);
    CHECK("false != false2", NoLane, f != f2, false);
    CHECK("true == false2", NoLane, t == f2, false);
    CHECK("false2 != true", NoLane, f2 != t, true);
    const Vector<size, Float> one{1.0f}, two{2.0f};
    const BitVector<size> ct = one <= one, cf = two <= one, lt = one < two;
    CHECK("<= all", NoLane, ct.all(), true);
    CHECK("bool(<=)", NoLane, bool(ct), true);
    CHECK("<= any", NoLane, ct.any(), true);
    CHECK("<= none", NoLane, ct.none(), false);
    CHECK("false <= none", NoLane, cf.none(), true);
    CHECK("false <= any", NoLane, cf.any(), false);
    CHECK("false <= all", NoLane, cf.all(), false);
    CHECK("< all", NoLane, lt.all(), true);
    CHECK("bool(<)", NoLane, bool(lt), true);
    CHECK("<= == true", NoLane, ct == t, true);
    CHECK("< == <=", NoLane, lt == ct, true);
    CHECK("false <= == false", NoLane, cf == f, true);
    CHECK("<= != false <=", NoLane, ct != cf, true);
    CHECK("< != true2", NoLane, lt != t2, false);
    CHECK("<= & true2", NoLane, (ct & t2).all(), true);
    CHECK("false <= | <", NoLane, (cf | lt).all(), true);
    CHECK("~false <=", NoLane, (~cf).all(), true);
    CHECK("<= ^ false <=", NoLane, (ct ^ cf).all(), true);
    CHECK("< && <=", NoLane, (lt && ct).all(), true);
    CHECK("equal all", NoLane, equal(one, one).all(), true);
    CHECK("notEqual all", NoLane, notEqual(one, two).all(), true);
    CHECK("notEqual any", NoLane, notEqual(one, one).any(), false);
    CHECK("> all", NoLane, (two > one).all(), true);
}

template<std::size_t size> void BitVectorUnpackedTest::constantOperands() {
    setTestCaseTemplateName(Utility::format("{}", size));

    Test::constantOperands<size>();

    /* MSVC 19.52 and 19.53 miscompile this at 16 with -fp:fast */
    #if defined(MSVC_CONSTANT_ALL_BUG) && defined(_M_FP_FAST)
    CORRADE_EXPECT_FAIL_IF(unpackedAt16<size>(), "MSVC 19.52 and 19.53 compute all() of 16 known lanes as false.");
    #endif
    CORRADE_COMPARE(failures, 0);
}

/* Every operand a compile-time constant, all lanes equal except `lane` */
template<std::size_t size, std::size_t lane> CORRADE_NEVER_INLINE void constantOperandsOneLane() {
    BitVector<size> but{true};
    but.reset(lane);
    BitVector<size> one;
    one.set(lane);
    const BitVector<size> but2 = ~one, every{true}, none{false};
    CHECK("but all", lane, but.all(), false);
    CHECK("but bool", lane, bool(but), false);
    CHECK("but any", lane, but.any(), size > 1);
    CHECK("but none", lane, but.none(), size == 1);
    CHECK("one any", lane, one.any(), true);
    CHECK("one none", lane, one.none(), false);
    CHECK("one all", lane, one.all(), size == 1);
    CHECK("but == but2", lane, but == but2, true);
    CHECK("but != but2", lane, but != but2, false);
    CHECK("but == one", lane, but == one, false);
    CHECK("but != one", lane, but != one, true);
    CHECK("but == every", lane, but == every, false);
    CHECK("but != every", lane, but != every, true);
    CHECK("one == none", lane, one == none, false);
    CHECK("one != none", lane, one != none, true);
    CHECK("| all", lane, (but | one).all(), true);
    CHECK("& none", lane, (but & one).none(), true);
    CHECK("^ all", lane, (but ^ one).all(), true);
    CHECK("^ == every", lane, (but ^ one) == every, true);
    CHECK("~one all", lane, (~one).all(), false);
    CHECK("lerp all", lane, Math::lerp(but, every, one).all(), true);
    CHECK("lerp none", lane, Math::lerp(one, none, one).none(), true);
    CHECK("lerp == but", lane, Math::lerp(every, none, one) == but, true);

    const Vector<size, Float> unit{1.0f};
    Vector<size, Float> a{1.0f};
    a[lane] = 2.0f;
    const BitVector<size> le = a <= unit, gt = a > unit, ge = a >= unit, lt = a < unit;
    CHECK("le all", lane, le.all(), false);
    CHECK("le any", lane, le.any(), size > 1);
    CHECK("le none", lane, le.none(), size == 1);
    CHECK("gt any", lane, gt.any(), true);
    CHECK("gt none", lane, gt.none(), false);
    CHECK("gt all", lane, gt.all(), size == 1);
    CHECK("ge all", lane, ge.all(), true);
    CHECK("lt none", lane, lt.none(), true);
    CHECK("lt any", lane, lt.any(), false);
    CHECK("le == but", lane, le == but, true);
    CHECK("gt == one", lane, gt == one, true);
    CHECK("le != gt", lane, le != gt, true);
    CHECK("le == gt", lane, le == gt, false);
    CHECK("eq all", lane, equal(a, unit).all(), false);
    CHECK("eq == but", lane, equal(a, unit) == but, true);
    CHECK("ne any", lane, notEqual(a, unit).any(), true);
    CHECK("ne none", lane, notEqual(a, unit).none(), false);
    CHECK("ne == one", lane, notEqual(a, unit) == one, true);
    CHECK("(le || gt) all", lane, (le || gt).all(), true);
    CHECK("(le && gt) none", lane, (le && gt).none(), true);

    Vector<size, Int> ia{3};
    ia[lane] = 5;
    const Vector<size, Int> ib{3};
    CHECK("int gt == one", lane, (ia > ib) == one, true);
    CHECK("int le == but", lane, (ia <= ib) == but, true);
    CHECK("int le all", lane, (ia <= ib).all(), false);
    CHECK("int ge all", lane, (ia >= ib).all(), true);
    CHECK("int gt any", lane, (ia > ib).any(), true);
    CHECK("int lt none", lane, (ia < ib).none(), true);
    CHECK("int lt any", lane, (ia < ib).any(), false);
    CHECK("int eq all", lane, equal(ia, ib).all(), false);
    CHECK("int ne any", lane, notEqual(ia, ib).any(), true);
    CHECK("int eq == but", lane, equal(ia, ib) == but, true);
    const Vector<size, Int> l = Math::lerp(ib, ia, one), l2 = Math::lerp(ia, ib, one);
    CHECK("vlerp", lane, l == ia, true);
    CHECK("vlerp2", lane, l2 == ib, true);
    CHECK("vlerp eq", lane, equal(l, ia).all(), true);
}

template<std::size_t size, std::size_t ...lane> void constantOperandsOneLane(Containers::Implementation::Sequence<lane...>) {
    const int dummy[]{0, (constantOperandsOneLane<size, lane>(), 0)...};
    static_cast<void>(dummy);
}

template<std::size_t size> void BitVectorUnpackedTest::constantOperandsOneLane() {
    setTestCaseTemplateName(Utility::format("{}", size));

    Test::constantOperandsOneLane<size>(typename Lanes<size, 9>::Type{});

    CORRADE_COMPARE(failures, 0);
}

/* Constants from the byte constructor, all lanes equal except `lane` */
template<std::size_t size, std::size_t lane> CORRADE_NEVER_INLINE void constantBytesOneLane() {
    const BitVector<size> but = laneBytes<size>(lane, true, false), one = laneBytes<size>(lane, false, false);
    const BitVector<size> every = laneBytes<size>(NoLane, true, false), none = laneBytes<size>(NoLane, false, false);
    const BitVector<size> butPad = laneBytes<size>(lane, true, true), everyPad = laneBytes<size>(NoLane, true, true);
    CHECK("but all", lane, but.all(), false);
    CHECK("but bool", lane, bool(but), false);
    CHECK("but any", lane, but.any(), size > 1);
    CHECK("but none", lane, but.none(), size == 1);
    CHECK("butPad all", lane, butPad.all(), false);
    CHECK("everyPad all", lane, everyPad.all(), true);
    CHECK("everyPad == every", lane, everyPad == every, true);
    CHECK("one any", lane, one.any(), true);
    CHECK("one none", lane, one.none(), false);
    CHECK("one all", lane, one.all(), size == 1);
    CHECK("every all", lane, every.all(), true);
    CHECK("every bool", lane, bool(every), true);
    CHECK("none none", lane, none.none(), true);
    CHECK("none any", lane, none.any(), false);
    CHECK("but == ~one", lane, but == ~one, true);
    CHECK("but != ~one", lane, but != ~one, false);
    CHECK("but == every", lane, but == every, false);
    CHECK("but != every", lane, but != every, true);
    CHECK("one == none", lane, one == none, false);
    CHECK("one != none", lane, one != none, true);
    CHECK("but == butPad", lane, but == butPad, true);
    CHECK("but != butPad", lane, but != butPad, false);
    CHECK("(but | one) all", lane, (but | one).all(), true);
    CHECK("(but & one) none", lane, (but & one).none(), true);
    CHECK("(but ^ one) == every", lane, (but ^ one) == every, true);
    CHECK("lerp == but", lane, Math::lerp(every, none, one) == but, true);
    CHECK("lerp all", lane, Math::lerp(but, every, one).all(), true);
    const Vector<size, Float> unit{1.0f};
    Vector<size, Float> a{1.0f};
    a[lane] = 0.0f;
    CHECK("lerp vec", lane, Math::lerp(unit, a, one) == a, true);
    CHECK("lerp vec2", lane, Math::lerp(a, unit, one) == unit, true);
    CHECK("lerp vec3", lane, Math::lerp(a, unit, but) == a, true);
    CHECK("lerp vec4", lane, Math::lerp(unit, a, but) == unit, true);
    CHECK("cmp == one", lane, (a < unit) == one, true);
    CHECK("cmp all", lane, (a >= unit).all(), false);
    CHECK("cmp eq", lane, (a >= unit) == but, true);
}

template<std::size_t size, std::size_t ...lane> void constantBytesOneLane(Containers::Implementation::Sequence<lane...>) {
    const int dummy[]{0, (constantBytesOneLane<size, lane>(), 0)...};
    static_cast<void>(dummy);
}

template<std::size_t size> void BitVectorUnpackedTest::constantBytesOneLane() {
    setTestCaseTemplateName(Utility::format("{}", size));

    Test::constantBytesOneLane<size>(typename Lanes<size, 9>::Type{});

    /* MSVC 19.53 miscompiles this at 16 only with -GL and -LTCG:INCREMENTAL. No macro tells, so a pass is
       accepted too */
    #ifdef MSVC_CONSTANT_ALL_BUG
    CORRADE_EXPECT_FAIL_IF(unpackedAt16<size>() && failures, "MSVC 19.52 and 19.53 compute all() of 16 known lanes as false.");
    #endif
    CORRADE_COMPARE(failures, 0);
}

CORRADE_NEVER_INLINE void constantOperandsMatrix() {
    constexpr std::size_t size = 16;
    const Matrix4x4<Float> a{IdentityInit}, b = a*2.0f;
    CHECK("<= all", NoLane, (a <= a).all(), true);
    CHECK(">= all", NoLane, (a >= a).all(), true);
    CHECK("<= all other", NoLane, (a <= b).all(), true);
    CHECK("< all", NoLane, (a < b).all(), false);
    CHECK("< any", NoLane, (a < b).any(), true);
    CHECK(">= all other", NoLane, (b >= a).all(), true);
    CHECK("> any", NoLane, (a > a).any(), false);
    CHECK("> none", NoLane, (a > a).none(), true);
}

void BitVectorUnpackedTest::constantOperandsMatrix() {
    Test::constantOperandsMatrix();

    /* MSVC 19.52 and 19.53 miscompile this */
    #ifdef MSVC_CONSTANT_ALL_BUG
    CORRADE_EXPECT_FAIL_IF(unpackedAt16<16>(), "MSVC 19.52 and 19.53 compute all() of 16 known lanes as false.");
    #endif
    CORRADE_COMPARE(failures, 0);
}

/* MSVC 19.52 and 19.53 with optimizations vectorize an AND over 16 bool lanes
   and compute false when the lanes are constants, with SSE2, AVX and AVX2 */
template<std::size_t size> CORRADE_NEVER_INLINE void constantLaneReductions() {
    BitVector<size> ones{ZeroInit}, one0{ZeroInit};
    for(std::size_t i = 0; i != size; ++i) {
        ones.set(i);
        one0.set(i, i != size/2);
    }
    Vector<size, Float> v;
    for(std::size_t i = 0; i != size; ++i)
        v[i] = Float(i);
    CHECK("all(1)", NoLane, ones.all(), true);
    CHECK("all(1,0)", NoLane, one0.all(), false);
    CHECK("all{true}", NoLane, BitVector<size>{true}.all(), true);
    CHECK("none{false}", NoLane, BitVector<size>{false}.none(), true);
    CHECK("all(cmp)", NoLane, (v <= v).all(), true);
    CHECK("==(cmp)", NoLane, (v <= v) == ones, true);
    CHECK("!=(cmp)", NoLane, (v <= v) != ones, false);
    CHECK("any(cmp)", NoLane, (v < v).any(), false);
    CHECK("none(cmp)", NoLane, (v < v).none(), true);
    CHECK("any(1,0)", NoLane, size == 1 || (one0.any() && !one0.none()), true);
    const BitVector<size> x = ones ^ one0;
    for(std::size_t i = 0; i != size; ++i)
        CHECK("^", i, x[i], i == size/2);
    CHECK("equal(cmp)", NoLane, equal(v, v).all(), true);
    CHECK("notEqual(cmp)", NoLane, notEqual(v, v).none(), true);
}

template<std::size_t size> void BitVectorUnpackedTest::constantLaneReductions() {
    setTestCaseTemplateName(Utility::format("{}", size));

    Test::constantLaneReductions<size>();

    #ifdef MSVC_CONSTANT_ALL_BUG
    CORRADE_EXPECT_FAIL_IF(unpackedAt16<size>(), "MSVC 19.52 and 19.53 compute all() of 16 known lanes as false.");
    #endif
    CORRADE_COMPARE(failures, 0);
}

template<std::size_t size> BitVector<size> maskExcept(std::size_t lane) {
    BitVector<size> out{ZeroInit};
    for(std::size_t i = 0; i != size; ++i)
        out.set(i, i != lane);
    return out;
}

/* Vector comparisons used this loop before they got #pragma GCC unroll, which
   hides the GCC 16.2 bug below */
template<std::size_t size, class T, class F> CORRADE_ALWAYS_INLINE BitVector<size> loopMask(const Vector<size, T>& a, const Vector<size, T>& b, F compare) {
    BitVector<size> out;
    for(std::size_t i = 0; i != size; ++i)
        out.set(i, compare(a[i], b[i]));
    return out;
}

/* GCC 16.2 at -O3, or -O2 -funroll-loops, with SSE4.1 or later replaces the vectorized != of two
   masks with 1 when one has a run-time lane and the other is a constant */
template<std::size_t size, std::size_t lane, bool loop> CORRADE_NEVER_INLINE void runtimeLaneAgainstConstants(Float x, Int y) {
    BitVector<size> but, one, every, none, alt, altK;
    for(std::size_t i = 0; i != size; ++i) {
        but.set(i, i != lane);
        one.set(i, i == lane);
        every.set(i);
        alt.set(i, i % 2);
        altK.set(i, (i % 2) != (i == lane));
    }
    Vector<size, Float> a{1.0f}, b{1.0f};
    a[lane] = x;
    Vector<size, Int> ia{3}, ib{3};
    ia[lane] = y;
    const BitVector<size> ge = loop ? loopMask(a, b, std::greater_equal<Float>{}) : a >= b;
    const BitVector<size> lt = loop ? loopMask(a, b, std::less<Float>{}) : a < b;
    const BitVector<size> ige = loop ? loopMask(ia, ib, std::greater_equal<Int>{}) : ia >= ib;
    const BitVector<size> ilt = loop ? loopMask(ia, ib, std::less<Int>{}) : ia < ib;
    const BitVector<size> ine = loop ? loopMask(ia, ib, std::not_equal_to<Int>{}) : notEqual(ia, ib);
    CHECK("xor any", lane, (ge ^ but).any(), false);
    CHECK("xor none", lane, (ge ^ but).none(), true);
    CHECK("xor alt ==", lane, (ge ^ alt) == (~one ^ alt), true);
    CHECK("xor alt !=", lane, (ge ^ alt) != (~one ^ alt), false);
    CHECK("xor alt any", lane, (ge ^ alt ^ ~altK).any(), false);
    CHECK("xor alt none", lane, (lt ^ one).none(), true);
    CHECK("int xor any", lane, (ige ^ but).any(), false);
    CHECK("int xor none", lane, (ilt ^ one).none(), true);
    CHECK("int ne == one", lane, ine == one, true);
    CHECK("int ne != one", lane, ine != one, false);
    CHECK("int ge == but", lane, ige == but, true);
    CHECK("int ge != but", lane, ige != but, false);
    CHECK("int but == ge", lane, but == ige, true);
    CHECK("int alt != ge", lane, alt != ige, !(size <= 2 && lane == 0));
    CHECK("alt == ge", lane, alt == ge, size <= 2 && lane == 0);
    CHECK("alt != ge", lane, alt != ge, !(size <= 2 && lane == 0));
    CHECK("altK == alt^ge", lane, altK == (alt ^ ~ge), true);
    BitVector<size> c = ge;
    c ^= but;
    CHECK("^= any", lane, c.any(), false);
    CHECK("^= none", lane, c.none(), true);
    BitVector<size> d = ge;
    d &= one;
    CHECK("&= any", lane, d.any(), false);
    CHECK("&= == none", lane, d == none, true);
    BitVector<size> e = lt;
    e |= but;
    CHECK("|= all", lane, e.all(), true);
    CHECK("|= == every", lane, e == every, true);
    CHECK("|= != every", lane, e != every, false);
    CHECK("~ == one", lane, ~ge == one, true);
    CHECK("~ != one", lane, ~ge != one, false);
    CHECK("! == but", lane, (!lt) == but, true);
    CHECK("&& == none", lane, (ge && one) == none, true);
    CHECK("|| == every", lane, (lt || but) == every, true);
    CHECK("lerp == alt", lane, Math::lerp(alt, one, lt) == altK, lane % 2 == 0);
    CHECK("lerp == every", lane, Math::lerp(every, one, lt) == every, true);
    CHECK("lerp none", lane, Math::lerp(none, one, ge).none(), true);
    CHECK("lerp any", lane, Math::lerp(none, one, lt).any(), true);
    const Vector<size, Float> l = Math::lerp(b, a, ge);
    CHECK("vlerp eq", lane, equal(l, b) == every, true);
    CHECK("vlerp ne", lane, notEqual(l, b).none(), true);
    CHECK("ge == but (all)", lane, (ge == but) && (but == ge) && !(ge != but) && !(but != ge), true);
    CHECK("lt == one (all)", lane, (lt == one) && (one == lt) && !(lt != one) && !(one != lt), true);
    CHECK("ige all", lane, ige.all(), false);
    CHECK("ige any", lane, ige.any(), size > 1);
    CHECK("ilt any", lane, ilt.any(), true);
    CHECK("ilt all", lane, ilt.all(), size == 1);
    CHECK("ine none", lane, ine.none(), false);
}

template<std::size_t size, std::size_t lane> CORRADE_NEVER_INLINE void runtimeLaneAgainstConstantsFloat(Float x) {
    BitVector<size> but, one, none, every;
    for(std::size_t i = 0; i != size; ++i) {
        but.set(i, i != lane);
        one.set(i, i == lane);
        every.set(i);
    }
    Vector<size, Float> a{1.0f}, b{1.0f};
    a[lane] = x;
    const BitVector<size> ge = a >= b, lt = a < b;
    CHECK("==", lane, ge == but, true);
    CHECK("!=", lane, ge != but, false);
    CHECK("== swapped", lane, but == ge, true);
    CHECK("!= swapped", lane, but != ge, false);
    CHECK("lt == one", lane, lt == one, true);
    CHECK("lt != one", lane, lt != one, false);
    CHECK("^ any", lane, (ge ^ but).any(), false);
    CHECK("^ none", lane, (ge ^ but).none(), true);
    CHECK("^ all", lane, (ge ^ ~but).all(), true);
    CHECK("~^ any", lane, (~ge ^ one).any(), false);
    CHECK("& any", lane, (ge & one).any(), false);
    CHECK("| all", lane, (ge | one).all(), true);
    CHECK("| none", lane, (lt | none).none(), false);
    CHECK("any", lane, ge.any(), size > 1);
    CHECK("lt any", lane, lt.any(), true);
    CHECK("lt none", lane, lt.none(), false);
    CHECK("all", lane, ge.all(), false);
    CHECK("bool", lane, bool(ge), false);
    CHECK("~ all", lane, (~lt).all(), false);
    CHECK("~ any", lane, (~ge).any(), true);
    CHECK("~ none", lane, (~ge).none(), false);
    CHECK("lerp ==", lane, Math::lerp(but, one, lt) == every, true);
    CHECK("lerp !=", lane, Math::lerp(one, but, ge) != every, false);
    CHECK("equal ==", lane, equal(a, b) == but, true);
    CHECK("notEqual ==", lane, notEqual(a, b) == one, true);
    CHECK("notEqual any", lane, notEqual(a, b).any(), true);
    CHECK("equal all", lane, equal(a, b).all(), false);
    CHECK("isNan any", lane, isNan(a).any(), false);
}

template<std::size_t size, std::size_t lane> CORRADE_NEVER_INLINE void runtimeLaneAgainstConstantsLowHigh(Float lo, Float hi) {
    const BitVector<size> butv = maskExcept<size>(lane), ones = maskExcept<size>(NoLane);
    Vector<size, Float> a{1.0f}, b{1.0f};
    a[lane] = lo;
    const BitVector<size> m = a >= b;
    CHECK("==", lane, m == butv, true);
    CHECK("!=", lane, m != butv, false);
    CHECK("any(^)", lane, (m ^ butv).any(), false);
    CHECK("none(^)", lane, (m ^ butv).none(), true);
    CHECK("all", lane, m.all(), false);
    CHECK("any", lane, m.any(), size > 1);
    CHECK("all(|~)", lane, (m | ~butv).all(), true);
    CHECK("==(&)", lane, (m & butv) == butv, true);
    CHECK("any(!)", lane, (!m).any(), true);
    CHECK("any(&&~)", lane, (m && ~butv).any(), false);
    CHECK("equal", lane, equal(a, b) == butv, true);
    CHECK("notEqual", lane, notEqual(a, b) == ~butv, true);
    BitVector<size> x = m;
    x ^= butv;
    CHECK("none(^=)", lane, x.none(), true);
    CHECK("any(^=)", lane, x.any(), false);
    a[lane] = hi;
    const BitVector<size> t = a >= b;
    CHECK("all(true)", lane, t.all(), true);
    CHECK("==ones", lane, t == ones, true);
    CHECK("!=ones", lane, t != ones, false);
    CHECK("any(^ones)", lane, (t ^ ones).any(), false);
}

template<std::size_t size, std::size_t ...lane> void runtimeLaneAgainstConstants(Float lo, Float hi, Int y, Containers::Implementation::Sequence<lane...>) {
    const int dummy[]{0, (runtimeLaneAgainstConstants<size, lane, false>(lo, y), runtimeLaneAgainstConstantsFloat<size, lane>(lo), runtimeLaneAgainstConstantsLowHigh<size, lane>(lo, hi), 0)...};
    static_cast<void>(dummy);
}

template<std::size_t size> void BitVectorUnpackedTest::runtimeLaneAgainstConstants() {
    setTestCaseTemplateName(Utility::format("{}", size));

    const Float lo = volatileHalf, hi = volatileTwo;
    const Int y = volatileTwoInt;
    Test::runtimeLaneAgainstConstants<size>(lo, hi, y, typename Lanes<size, 9>::Type{});

    /* MSVC 19.53 miscompiles "| all" in runtimeLaneAgainstConstantsFloat() at 16 only with -GL. No macro tells,
       so a pass is accepted too */
    #ifdef MSVC_CONSTANT_ALL_BUG
    CORRADE_EXPECT_FAIL_IF(unpackedAt16<size>() && failures, "MSVC 19.52 and 19.53 compute all() of 16 known lanes as false.");
    #endif
    CORRADE_COMPARE(failures, 0);
}

template<std::size_t size, std::size_t ...lane> void runtimeLaneLoopMasks(Float x, Int y, Containers::Implementation::Sequence<lane...>) {
    const int dummy[]{0, (runtimeLaneAgainstConstants<size, lane, true>(x, y), 0)...};
    static_cast<void>(dummy);
}

template<std::size_t size> void BitVectorUnpackedTest::runtimeLaneLoopMasks() {
    setTestCaseTemplateName(Utility::format("{}", size));

    Test::runtimeLaneLoopMasks<size>(volatileHalf, volatileTwoInt, typename Lanes<size, 9>::Type{});

    /* GCC 16.2 miscompiles this at 16 only at -O3 or -O2 -funroll-loops. No macro tells, so a pass is
       accepted too */
    #ifdef GCC_MASK_NOT_EQUAL_BUG
    CORRADE_EXPECT_FAIL_IF(unpackedAt16<size>() && failures, "GCC 16.2 computes != of a run-time and a constant mask as true.");
    #endif
    CORRADE_COMPARE(failures, 0);
}

}}}}

CORRADE_TEST_MAIN(Magnum::Math::Test::BitVectorUnpackedTest)

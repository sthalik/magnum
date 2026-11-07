#ifndef Magnum_Math_BitVector_h
#define Magnum_Math_BitVector_h
/*
    This file is part of Magnum.

    Copyright © 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019,
                2020, 2021, 2022, 2023, 2024, 2025, 2026
              Vladimír Vondruš <mosra@centrum.cz>
    Copyright © 2022 Stanislaw Halik <sthalik@misaki.pl>

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

/** @file
 * @brief Class @ref Magnum::Math::BitVector
 * @m_since_latest
 */

/* std::declval() is said to be in <utility> but libstdc++, libc++ and MSVC STL
   all have it directly in <type_traits> because it just makes sense */
#include <type_traits>
#include <Corrade/Containers/sequenceHelpers.h>
#ifndef CORRADE_SINGLES_NO_DEBUG
#include <Corrade/Utility/Debug.h>
#endif
#include <Corrade/Utility/Macros.h> /* CORRADE_CONSTEXPR14 */

#include "Magnum/Magnum.h"
#include "Magnum/Math/Math.h"
#include "Magnum/Math/Tags.h"

/* GCC 5.0 to 6.1 crash on constant evaluation of the bit operations on
   _data[] in set() and reset(), even for calls made at run time */
#if defined(CORRADE_TARGET_GCC) && !defined(CORRADE_TARGET_CLANG) && __GNUC__*100 + __GNUC_MINOR__ < 602
#define MAGNUM_BITVECTOR_SET_CONSTEXPR14
#else
#define MAGNUM_BITVECTOR_SET_CONSTEXPR14 CORRADE_CONSTEXPR14
#endif

/* Raised only by MathBitVectorUnpacked16Test. All translation units of a
   program have to agree on it, MSVC linkers check that */
#ifndef MAGNUM_MATH_BITVECTOR_MAX_UNPACKED_SIZE
#define MAGNUM_MATH_BITVECTOR_MAX_UNPACKED_SIZE 8
#endif
#ifdef CORRADE_TARGET_MSVC
#pragma detect_mismatch("MAGNUM_MATH_BITVECTOR_MAX_UNPACKED_SIZE", _CORRADE_HELPER_STR2(MAGNUM_MATH_BITVECTOR_MAX_UNPACKED_SIZE))
#endif

namespace Magnum { namespace Math {

namespace Implementation {
    template<std::size_t, class> struct BitVectorConverter;

    /* Like std::bit_and, std::bit_or and std::bit_xor, which aren't constexpr
       before C++14 */
    struct BitAnd {
        template<class T> constexpr T operator()(T a, T b) const { return T(a & b); }
    };
    struct BitOr {
        template<class T> constexpr T operator()(T a, T b) const { return T(a | b); }
    };
    struct BitXor {
        template<class T> constexpr T operator()(T a, T b) const { return T(a ^ b); }
    };

    template<class T> constexpr T repeat(T value, std::size_t) { return value; }

    #if defined(CORRADE_TARGET_MSVC) && !defined(CORRADE_TARGET_CLANG_CL)
    constexpr bool bitVectorAll(bool first) { return first; }
    template<class ...T> constexpr bool bitVectorAll(bool first, T... next) { return first & bitVectorAll(next...); }
    #endif

    enum: std::size_t { BitVectorMaxUnpackedSize = MAGNUM_MATH_BITVECTOR_MAX_UNPACKED_SIZE };

    /* Above BitVectorMaxUnpackedSize components, eight per byte */
    template<std::size_t size, bool = (size > BitVectorMaxUnpackedSize)> struct BitVectorStorage {
        typedef UnsignedByte Type;
        enum: std::size_t { Size = (size + 7)/8 };
        enum: UnsignedByte {
            FullSegmentMask = 0xFF,
            LastSegmentMask = (1 << size%8) - 1
        };

        static constexpr bool get(const Type(&data)[Size], std::size_t i) {
            return (data[i/8] >> i%8) & 0x01;
        }

        static MAGNUM_BITVECTOR_SET_CONSTEXPR14 void set(Type(&data)[Size], std::size_t i) {
            data[i/8] |= 1 << i%8;
        }

        static MAGNUM_BITVECTOR_SET_CONSTEXPR14 void reset(Type(&data)[Size], std::size_t i) {
            data[i/8] &= ~(1 << i%8);
        }

        static MAGNUM_BITVECTOR_SET_CONSTEXPR14 void set(Type(&data)[Size], std::size_t i, bool value) {
            value ? data[i/8] |=  (1 << i%8) :
                    data[i/8] &= ~(1 << i%8);
        }

        static CORRADE_CONSTEXPR14 bool equals(const Type(&a)[Size], const Type(&b)[Size]) {
            for(std::size_t i = 0; i != size/8; ++i)
                if(a[i] != b[i])
                    return false;

            /* Check last segment */
            if(size%8 && (a[Size-1] & LastSegmentMask) != (b[Size-1] & LastSegmentMask))
                return false;

            return true;
        }

        static CORRADE_CONSTEXPR14 bool notEquals(const Type(&a)[Size], const Type(&b)[Size]) { return !equals(a, b); }

        static CORRADE_CONSTEXPR14 bool all(const Type(&data)[Size]) {
            /* Check all full segments */
            for(std::size_t i = 0; i != size/8; ++i)
                if(data[i] != FullSegmentMask)
                    return false;

            /* Check last segment */
            if(size%8 && (data[Size-1] & LastSegmentMask) != LastSegmentMask)
                return false;

            return true;
        }

        static CORRADE_CONSTEXPR14 bool any(const Type(&data)[Size]) { return !none(data); }

        static CORRADE_CONSTEXPR14 bool none(const Type(&data)[Size]) {
            /* Check all full segments */
            for(std::size_t i = 0; i != size/8; ++i)
                if(data[i])
                    return false;

            /* Check last segment */
            if(size%8 && (data[Size-1] & LastSegmentMask))
                return false;

            return true;
        }

        static constexpr UnsignedByte byte(const BitVector<size>& value, std::size_t i) {
            return value.data()[i];
        }
    };

    /* Up to BitVectorMaxUnpackedSize components, one bool each. After
       inlining, the optimizer sees them as separate scalar values */
    template<std::size_t size> struct BitVectorStorage<size, false> {
        typedef bool Type;
        enum: std::size_t { Size = size };
        enum: UnsignedByte { FullSegmentMask = 1 };

        static constexpr bool get(const Type(&data)[Size], std::size_t i) {
            return data[i];
        }

        static MAGNUM_BITVECTOR_SET_CONSTEXPR14 void set(Type(&data)[Size], std::size_t i) {
            data[i] = true;
        }

        static MAGNUM_BITVECTOR_SET_CONSTEXPR14 void reset(Type(&data)[Size], std::size_t i) {
            data[i] = false;
        }

        static MAGNUM_BITVECTOR_SET_CONSTEXPR14 void set(Type(&data)[Size], std::size_t i, bool value) {
            data[i] = value;
        }

        static CORRADE_CONSTEXPR14 bool equals(const Type(&a)[Size], const Type(&b)[Size]) {
            return !notEquals(a, b);
        }

        static CORRADE_CONSTEXPR14 bool notEquals(const Type(&a)[Size], const Type(&b)[Size]) {
            UnsignedByte out = 0;
            for(std::size_t i = 0; i != size; ++i)
                out |= UnsignedByte(a[i] ^ b[i]);
            return out;
        }

        static CORRADE_CONSTEXPR14 bool all(const Type(&data)[Size]) {
            #if defined(CORRADE_TARGET_MSVC) && !defined(CORRADE_TARGET_CLANG_CL)
            /* At 3 and 4 components, a & b & c saves a movzx on MSVC */
            if(size == 3 || size == 4)
                return all(data, typename Containers::Implementation::GenerateSequence<size>::Type{});
            #endif
            bool out = true;
            for(std::size_t i = 0; i != size; ++i)
                out &= data[i];
            return out;
        }

        #if defined(CORRADE_TARGET_MSVC) && !defined(CORRADE_TARGET_CLANG_CL)
        template<std::size_t ...sequence> static constexpr bool all(const Type(&data)[Size], Containers::Implementation::Sequence<sequence...>) {
            return bitVectorAll(data[sequence]...);
        }
        #endif

        static CORRADE_CONSTEXPR14 bool none(const Type(&data)[Size]) {
            return !any(data);
        }

        static CORRADE_CONSTEXPR14 bool any(const Type(&data)[Size]) {
            /* Starting from data[0] is shorter on GCC up to 4 components, and
               longer above */
            if(size <= 4) {
                UnsignedByte out = data[0];
                for(std::size_t i = 1; i != size; ++i)
                    out |= UnsignedByte(data[i]);
                return out;
            }
            UnsignedByte out = 0;
            for(std::size_t i = 0; i != size; ++i)
                out |= UnsignedByte(data[i]);
            return out;
        }

        static CORRADE_CONSTEXPR14 UnsignedByte byte(const BitVector<size>& value, std::size_t i) {
            UnsignedByte out = 0;
            for(std::size_t j = 0; j != 8 && i*8 + j != size; ++j)
                out |= value[i*8 + j] << j;
            return out;
        }
    };
}

/**
@brief Vector of bits
@tparam size    Bit count
@m_since_latest

Result of component-wise comparison from @ref Vector. Up to 8 boolean values
are stored unpacked, as one @cpp bool @ce each. Above that they're packed as
bits in array of unsigned bytes, unused bits have undefined value which
doesn't affect comparison or @ref all() / @ref none() / @ref any() functions.
See also @ref matrix-vector for brief introduction.

@section Math-BitVector-indexing Bit indexing

Value at position 0 is the lowest bit of the first byte passed in constructor.
Value at position 8 is the lowest bit of the second byte passed in constructor.
For example:

@snippet Math-cpp14.cpp BitVector-indexing

@section Math-BitVector-boolean Boolean operations

The class implements @cpp && @ce, @cpp || @ce and @cpp ! @ce operators
component-wise, in other words equivalently to @cpp & @ce, @cpp | @ce and
@cpp ~ @ce. This is done in order to have consistent behavior with boolean
operations on scalar types --- in the following example, it causes the final
conversion to @cpp bool @ce to be done at the end (instead of it happening
already in the boolean subexpressions). Combined with @ref operator bool()
returning @cpp true @ce only if all bits are set, this means the condition will
be passed only if @cpp b @ce is around @cpp a @ce in *all dimensions*, and work
the same way as if the variables were just scalars:

@snippet Math.cpp BitVector-boolean

@see @ref Magnum::BitVector2, @ref Magnum::BitVector3, @ref Magnum::BitVector4
*/
template<std::size_t size> class BitVector {
    static_assert(size != 0, "BitVector cannot have zero elements");

    typedef Implementation::BitVectorStorage<size> Storage;

    public:
        enum: std::size_t {
            Size = size,                /**< Vector size */
            DataSize = (size + 7)/8     /**< Size of the bits packed in bytes */
        };

        /**
         * @brief Default constructor
         *
         * Equivalent to @ref BitVector(ZeroInitT).
         */
        constexpr /*implicit*/ BitVector() noexcept: _data{} {}

        /** @brief Construct a zero-filled bit vector */
        constexpr explicit BitVector(ZeroInitT) noexcept: _data{} {}

        /** @brief Construct without initializing the contents */
        CORRADE_CONSTEXPR20 explicit BitVector(Magnum::NoInitT) noexcept {}

        /**
         * @brief Construct a bit vector from segment values
         * @param first Value for first 8bit segment
         * @param next  Values for next Bbit segments
         */
        template<class ...T
            #ifndef DOXYGEN_GENERATING_OUTPUT
            /* For some reason MSVC 2015 and 2017 doesn't understand the value
               parameter here, only a type parameter. Don't need to prevent any
               ambiguity with this constructor so it's fine. */
            #ifndef CORRADE_MSVC2017_COMPATIBILITY
            , typename std::enable_if<sizeof...(T)+1 == DataSize, int>::type = 0
            #else
            , class = typename std::enable_if<sizeof...(T)+1 == DataSize>::type
            #endif
            #endif
        > constexpr /*implicit*/ BitVector(UnsignedByte first, T... next) noexcept: BitVector(Storage{}, first, UnsignedByte(next)...) {}

        /** @brief Construct a bit vector with one value for all fields */
        template<class T
            #ifndef DOXYGEN_GENERATING_OUTPUT
            , typename std::enable_if<std::is_same<bool, T>::value && size != 1, int>::type = 0
            #endif
        > constexpr explicit BitVector(T value) noexcept: BitVector(Storage{}, typename Containers::Implementation::GenerateSequence<Storage::Size>::Type{}, value ? 0xFFu : 0u) {}

        /** @brief Construct a bit vector from external representation */
        template<class U, class = decltype(Implementation::BitVectorConverter<size, U>::from(std::declval<U>()))> constexpr explicit BitVector(const U& other) noexcept: BitVector{Implementation::BitVectorConverter<size, U>::from(other)} {}

        /** @brief Convert the bit vector to external representation */
        template<class U, class = decltype(Implementation::BitVectorConverter<size, U>::to(std::declval<BitVector<size>>()))> constexpr explicit operator U() const {
            return Implementation::BitVectorConverter<size, U>::to(*this);
        }

        /**
         * @brief Raw data
         *
         * Contrary to what Doxygen shows, returns reference to a
         * one-dimensional fixed-size array of @ref DataSize elements, i.e.
         * @cpp UnsignedByte(&)[DataSize] @ce. Available only for sizes above
         * 8, which store the bits packed in bytes. Smaller sizes are
         * unpacked, with one @cpp bool @ce per bit.
         * @see @ref operator[](), @ref set()
         * @todoc Fix once there's a possibility to patch the signature in a
         *      post-processing step (https://github.com/mosra/m.css/issues/56)
         */
        #ifdef DOXYGEN_GENERATING_OUTPUT
        CORRADE_CONSTEXPR14 UnsignedByte* data();
        constexpr const UnsignedByte* data() const; /**< @overload */
        #else
        template<std::size_t size_ = size, typename std::enable_if<(size_ > Implementation::BitVectorMaxUnpackedSize), int>::type = 0> CORRADE_CONSTEXPR14 auto data() -> UnsignedByte(&)[DataSize] { return _data; }
        template<std::size_t size_ = size, typename std::enable_if<(size_ > Implementation::BitVectorMaxUnpackedSize), int>::type = 0> constexpr auto data() const -> const UnsignedByte(&)[DataSize] { return _data; }
        #endif

        /** @brief Bit at given position */
        constexpr bool operator[](std::size_t i) const {
            return Storage::get(_data, i);
        }

        /**
         * @brief Set a bit at given position
         * @m_since_latest
         */
        MAGNUM_BITVECTOR_SET_CONSTEXPR14 BitVector<size>& set(std::size_t i) {
            Storage::set(_data, i);
            return *this;
        }

        /**
         * @brief Reset a bit at given position
         * @m_since_latest
         */
        MAGNUM_BITVECTOR_SET_CONSTEXPR14 BitVector<size>& reset(std::size_t i) {
            Storage::reset(_data, i);
            return *this;
        }

        /**
         * @brief Set or reset a bit at given position
         *
         * Prefer to use @ref set(std::size_t) and @ref reset(std::size_t)
         * where possible as that's a simpler operation.
         */
        MAGNUM_BITVECTOR_SET_CONSTEXPR14 BitVector<size>& set(std::size_t i, bool value) {
            Storage::set(_data, i, value);
            return *this;
        }

        /** @brief Equality comparison */
        CORRADE_CONSTEXPR14 bool operator==(const BitVector<size>& other) const;

        /** @brief Non-equality comparison */
        CORRADE_CONSTEXPR14 bool operator!=(const BitVector<size>& other) const {
            return Storage::notEquals(_data, other._data);
        }

        /**
         * @brief Boolean conversion
         *
         * Equivalent to @ref all().
         * @see @ref any(), @ref none()
         */
        explicit CORRADE_CONSTEXPR14 operator bool() const { return all(); }

        /**
         * @brief Whether all bits are set
         *
         * @see @ref none(), @ref any(), @ref operator bool()
         */
        CORRADE_CONSTEXPR14 bool all() const;

        /**
         * @brief Whether no bits are set
         *
         * @see @ref all(), @ref any(), @ref operator bool()
         */
        CORRADE_CONSTEXPR14 bool none() const;

        /**
         * @brief Whether any bit is set
         *
         * @see @ref all(), @ref none(), @ref operator bool()
         */
        CORRADE_CONSTEXPR14 bool any() const { return Storage::any(_data); }

        /** @brief Bitwise inversion */
        CORRADE_CONSTEXPR14 BitVector<size> operator~() const;

        /**
         * @brief Component-wise boolean negation
         * @m_since{2019,10}
         *
         * Equivalent to @ref operator~(). See @ref Math-BitVector-boolean for
         * more information.
         */
        CORRADE_CONSTEXPR14 BitVector<size> operator!() const { return operator~(); }

        /**
         * @brief Bitwise AND and assign
         *
         * The computation is done in-place.
         */
        CORRADE_CONSTEXPR14 BitVector<size>& operator&=(const BitVector<size>& other) {
            for(std::size_t i = 0; i != Storage::Size; ++i)
                _data[i] &= other._data[i];

            return *this;
        }

        /**
         * @brief Bitwise AND
         *
         * @see @ref operator&=()
         */
        #ifdef DOXYGEN_GENERATING_OUTPUT
        BitVector<size> operator&(const BitVector<size>& other) const;
        #else
        template<std::size_t size_ = size, typename std::enable_if<(size_ <= Implementation::BitVectorMaxUnpackedSize), int>::type = 0> CORRADE_CONSTEXPR14 BitVector<size> operator&(const BitVector<size>& other) const {
            return BitVector<size>{Implementation::BitAnd{}, *this, other, typename Containers::Implementation::GenerateSequence<size>::Type{}};
        }
        template<std::size_t size_ = size, typename std::enable_if<(size_ > Implementation::BitVectorMaxUnpackedSize), int>::type = 0> CORRADE_CONSTEXPR14 BitVector<size> operator&(const BitVector<size>& other) const {
            return BitVector<size>(*this) &= other;
        }
        #endif

        /**
         * @brief Component-wise boolean AND
         * @m_since{2019,10}
         *
         * Equivalent to @ref operator&(). See @ref Math-BitVector-boolean for
         * more information.
         */
        #ifdef DOXYGEN_GENERATING_OUTPUT
        BitVector<size> operator&&(const BitVector<size>& other) const;
        #else
        template<std::size_t size_ = size, typename std::enable_if<(size_ <= Implementation::BitVectorMaxUnpackedSize), int>::type = 0> CORRADE_CONSTEXPR14 BitVector<size> operator&&(const BitVector<size>& other) const {
            return BitVector<size>{Implementation::BitAnd{}, *this, other, typename Containers::Implementation::GenerateSequence<size>::Type{}};
        }
        template<std::size_t size_ = size, typename std::enable_if<(size_ > Implementation::BitVectorMaxUnpackedSize), int>::type = 0> CORRADE_CONSTEXPR14 BitVector<size> operator&&(const BitVector<size>& other) const {
            return BitVector<size>(*this) &= other;
        }
        #endif

        /**
         * @brief Bitwise OR and assign
         *
         * The computation is done in-place.
         */
        CORRADE_CONSTEXPR14 BitVector<size>& operator|=(const BitVector<size>& other) {
            for(std::size_t i = 0; i != Storage::Size; ++i)
                _data[i] |= other._data[i];

            return *this;
        }

        /**
         * @brief Bitwise OR
         *
         * @see @ref operator|=()
         */
        #ifdef DOXYGEN_GENERATING_OUTPUT
        BitVector<size> operator|(const BitVector<size>& other) const;
        #else
        template<std::size_t size_ = size, typename std::enable_if<(size_ <= Implementation::BitVectorMaxUnpackedSize), int>::type = 0> CORRADE_CONSTEXPR14 BitVector<size> operator|(const BitVector<size>& other) const {
            return BitVector<size>{Implementation::BitOr{}, *this, other, typename Containers::Implementation::GenerateSequence<size>::Type{}};
        }
        template<std::size_t size_ = size, typename std::enable_if<(size_ > Implementation::BitVectorMaxUnpackedSize), int>::type = 0> CORRADE_CONSTEXPR14 BitVector<size> operator|(const BitVector<size>& other) const {
            return BitVector<size>(*this) |= other;
        }
        #endif

        /**
         * @brief Component-wise boolean OR
         * @m_since{2019,10}
         *
         * Equivalent to @ref operator|(). See @ref Math-BitVector-boolean for
         * more information.
         */
        #ifdef DOXYGEN_GENERATING_OUTPUT
        BitVector<size> operator||(const BitVector<size>& other) const;
        #else
        template<std::size_t size_ = size, typename std::enable_if<(size_ <= Implementation::BitVectorMaxUnpackedSize), int>::type = 0> CORRADE_CONSTEXPR14 BitVector<size> operator||(const BitVector<size>& other) const {
            return BitVector<size>{Implementation::BitOr{}, *this, other, typename Containers::Implementation::GenerateSequence<size>::Type{}};
        }
        template<std::size_t size_ = size, typename std::enable_if<(size_ > Implementation::BitVectorMaxUnpackedSize), int>::type = 0> CORRADE_CONSTEXPR14 BitVector<size> operator||(const BitVector<size>& other) const {
            return BitVector<size>(*this) |= other;
        }
        #endif

        /**
         * @brief Bitwise XOR and assign
         *
         * The computation is done in-place.
         */
        CORRADE_CONSTEXPR14 BitVector<size>& operator^=(const BitVector<size>& other) {
            for(std::size_t i = 0; i != Storage::Size; ++i)
                _data[i] ^= other._data[i];

            return *this;
        }

        /**
         * @brief Bitwise XOR
         *
         * @see @ref operator^=()
         */
        #ifdef DOXYGEN_GENERATING_OUTPUT
        BitVector<size> operator^(const BitVector<size>& other) const;
        #else
        template<std::size_t size_ = size, typename std::enable_if<(size_ <= Implementation::BitVectorMaxUnpackedSize), int>::type = 0> CORRADE_CONSTEXPR14 BitVector<size> operator^(const BitVector<size>& other) const {
            return BitVector<size>{Implementation::BitXor{}, *this, other, typename Containers::Implementation::GenerateSequence<size>::Type{}};
        }
        template<std::size_t size_ = size, typename std::enable_if<(size_ > Implementation::BitVectorMaxUnpackedSize), int>::type = 0> CORRADE_CONSTEXPR14 BitVector<size> operator^(const BitVector<size>& other) const {
            return BitVector<size>(*this) ^= other;
        }
        #endif

    private:
        /* Implementation for BitVector(UnsignedByte, T...) */
        template<class ...T> constexpr explicit BitVector(Implementation::BitVectorStorage<size, true>, UnsignedByte first, T... next) noexcept: _data{first, next...} {}
        constexpr explicit BitVector(Implementation::BitVectorStorage<size, false>, UnsignedByte first) noexcept: BitVector(Storage{}, typename Containers::Implementation::GenerateSequence<size>::Type{}, first) {}
        template<class ...T> constexpr explicit BitVector(Implementation::BitVectorStorage<size, false>, UnsignedByte first, UnsignedByte second, T... next) noexcept: BitVector(Storage{}, typename Containers::Implementation::GenerateSequence<size>::Type{}, {first, second, next...}) {}
        template<std::size_t ...sequence> constexpr explicit BitVector(Implementation::BitVectorStorage<size, false>, Containers::Implementation::Sequence<sequence...>, const UnsignedByte(&bytes)[DataSize]) noexcept: _data{bool((bytes[sequence/8] >> sequence%8) & 1)...} {}

        /* Implementation for BitVector(T) and the above */
        template<std::size_t ...sequence> constexpr explicit BitVector(Implementation::BitVectorStorage<size, true>, Containers::Implementation::Sequence<sequence...>, UnsignedInt value) noexcept: _data{UnsignedByte(Implementation::repeat(value, sequence))...} {}
        template<std::size_t ...sequence> constexpr explicit BitVector(Implementation::BitVectorStorage<size, false>, Containers::Implementation::Sequence<sequence...>, UnsignedInt value) noexcept: _data{bool((value >> sequence%8) & 1)...} {}

        /* Implementation for &, |, ^, && and || on unpacked storage. Clang
           keeps a copy of *this as one packed integer */
        template<class F, std::size_t ...sequence> constexpr explicit BitVector(F f, const BitVector<size>& a, const BitVector<size>& b, Containers::Implementation::Sequence<sequence...>) noexcept: _data{f(a._data[sequence], b._data[sequence])...} {}

        typename Storage::Type _data[Storage::Size];
};

#ifndef CORRADE_SINGLES_NO_DEBUG
/**
@debugoperator{BitVector}
@m_since_latest

In order to avoid potential confusion, prints the value as a comma-separated sequence of binary literals, so the output corresponds to how the value would
be constructed. For example,

@snippet Math-cpp14.cpp BitVector-debug

<b></b>

@m_class{m-noindent}

prints as

@code{.shell-session}
BitVector(0b1010) BitVector(0b00001000, 0b00000011, 0b100)
@endcode

Note that this, on the other hand, makes mapping to bit indices less obvious
--- see @ref Math-BitVector-indexing for more information.
*/
template<std::size_t size
    #ifndef DOXYGEN_GENERATING_OUTPUT
    , typename std::enable_if<(size <= Implementation::BitVectorMaxUnpackedSize), int>::type = 0
    #endif
> Debug& operator<<(Debug& debug, const BitVector<size>& value) {
    debug << "BitVector(0b" << Debug::nospace;

    for(std::size_t byte = 0; byte != BitVector<size>::DataSize; ++byte) {
        if(byte)
            debug << ", 0b" << Debug::nospace;
        const std::size_t end = byte*8 + 8 < size ? byte*8 + 8 : size;
        for(std::size_t i = end; i != byte*8; --i)
            debug << (value[i - 1] ? "1" : "0") << Debug::nospace;
    }

    return debug << ")";
}

#ifndef DOXYGEN_GENERATING_OUTPUT
template<std::size_t size, typename std::enable_if<(size > Implementation::BitVectorMaxUnpackedSize), int>::type = 0> Debug& operator<<(Debug& debug, const BitVector<size>& value) {
    debug << "BitVector(0b" << Debug::nospace;

    /* Print the full bytes comma-separated */
    for(std::size_t byte = 0; byte != BitVector<size>::DataSize - 1; ++byte) {
        for(std::size_t i = 0; i != 8; ++i)
            debug << (((value.data()[byte] >> (8 - i - 1)) & 1) ? "1" : "0")
                  << Debug::nospace;
        debug << ", 0b" << Debug::nospace;
    }

    /* Print the last (potentially) partial byte */
    constexpr std::size_t suffixSize = size%8 ? size%8 : 8;
    for(std::size_t i = 0; i != suffixSize; ++i)
        debug << (((value.data()[BitVector<size>::DataSize - 1] >> (suffixSize - i - 1)) & 1) ? "1" : "0")
              << Debug::nospace;

    return debug << ")";
}
#endif
#endif

template<std::size_t size> CORRADE_CONSTEXPR14 inline bool BitVector<size>::operator==(const BitVector<size>& other) const {
    return Storage::equals(_data, other._data);
}

template<std::size_t size> CORRADE_CONSTEXPR14 inline bool BitVector<size>::all() const {
    return Storage::all(_data);
}

template<std::size_t size> CORRADE_CONSTEXPR14 inline bool BitVector<size>::none() const {
    return Storage::none(_data);
}

template<std::size_t size> CORRADE_CONSTEXPR14 inline BitVector<size> BitVector<size>::operator~() const {
    #if CORRADE_CXX_STANDARD >= 202002L
    BitVector<size> out{Magnum::NoInit};
    #else
    BitVector<size> out{ZeroInit};
    #endif

    for(std::size_t i = 0; i != Storage::Size; ++i)
        out._data[i] = typename Storage::Type(_data[i] ^ Storage::FullSegmentMask);

    return out;
}

#ifndef MAGNUM_NO_MATH_STRICT_WEAK_ORDERING
namespace Implementation {

template<std::size_t size> struct StrictWeakOrdering<BitVector<size>> {
    bool operator()(const BitVector<size>& a, const BitVector<size>& b) const {
        for(std::size_t i = 0; i < BitVector<size>::DataSize - 1; ++i) {
            const UnsignedByte ai = BitVectorStorage<size>::byte(a, i);
            const UnsignedByte bi = BitVectorStorage<size>::byte(b, i);
            if(ai < bi)
                return true;
            if(ai > bi)
                return false;
        }

        /* Mask last element with to hide padding bits */
        constexpr UnsignedByte mask = UnsignedByte(0xFF) >> (BitVector<size>::DataSize * 8 - size);
        constexpr std::size_t i = BitVector<size>::DataSize - 1;
        return (BitVectorStorage<size>::byte(a, i) & mask) < (BitVectorStorage<size>::byte(b, i) & mask);
    }
};

}
#endif

}}

#undef MAGNUM_BITVECTOR_SET_CONSTEXPR14

#endif

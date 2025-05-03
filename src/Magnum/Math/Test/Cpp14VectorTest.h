#ifndef Magnum_Math_Test_Cpp14Vector
#define Magnum_Math_Test_Cpp14Vector

#include <Corrade/Corrade.h>

#define CE
#ifdef TESTING_CONSTEXPR
/* Same condition as CORRADE_CONSTEXPR14. MSVC 2017 doesn't define
   __cpp_constexpr. */
#if CORRADE_CXX_STANDARD >= 201402 && !defined CORRADE_MSVC2015_COMPATIBILITY
#undef CE
#define CE constexpr
#else
#define SKIP_TESTING
#endif
#endif

#endif

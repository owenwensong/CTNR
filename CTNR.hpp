/*!*****************************************************************************
 * @file    CTNR.hpp
 * @author  Owen Huang Wensong
 * @date    18 APR 2023
 * @brief   Compile Type Name Reflector to get type names automatically as a
 *          constexpr null-terminated C style string for convenience.
 *          Designed for C++11 onwards for clang, g++, MSVC.
 *
 *          To use, simply include this file and call CTNR::GetName. Examples: 
 * 
 *          clang++ and g++:
 *            GetName<void>()                   -> void
 *            GetName<classy*>()                -> classy
 *            GetName<structy<classy> const&>() -> structy<classy>
 * 
 *          MSVC: 
 *            GetName<void>()                   -> void
 *            GetName<classy*>()                -> class classy
 *            GetName<structy<classy> const&>() -> struct structy<class classy>
 *
 *          The return of GetName is a constexpr, allowing for other constexpr
 *          evaluations on the type name, eg: string hashing.
 *
 * @par     Notes for MSVC:
 *            __forceinline isn't enforced on debug mode by default
 *            struct and class typenames differ by having the prefixes of
 *            "struct " and "class " respectively, this applies to templates
 *            as shown above.
 *            
 *
 * @par     Copyright (c) 2023 Owen Huang Wensong.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
*******************************************************************************/

#if defined(__clang__) || defined(__GNUC__)
#define CTNR_ATTRIB_FORCE_INLINE __attribute__((always_inline)) inline
#elif defined(_MSC_VER)
#define CTNR_ATTRIB_FORCE_INLINE __forceinline
#else
static_assert(false, "CTNR unkown compiler error");
#endif

namespace CTNR
{
  // Internal implementation namespace, do not use these.
  namespace Impl
  {
    // Use auto to get the signature smaller in case it doesn't get optimized away
    template <typename T>
    constexpr const char* FFN()// Full Function Name
    {
#if defined(__clang__) || defined(__GNUC__)
      return __PRETTY_FUNCTION__;
#elif defined(_MSC_VER)
      return __FUNCSIG__;
#endif
    }

    // Compile Time Length (Compile Time Recursion)
    inline constexpr unsigned CTL(const char* inLiteral, unsigned tail = 0)
    {
      return *inLiteral ? CTL(inLiteral + 1, tail + 1) : tail;
    }

    // Compile Time Find Void (Compile Time Recursion)
    inline constexpr unsigned CTFV(const char* inLiteral, unsigned tail = 0, unsigned match = 0)
    {
      return 4 == match ? tail - 4 : (*inLiteral ? CTFV(inLiteral + 1, tail + 1, "void"[match] == *inLiteral ? match + 1 : 0) : -1);
    }

    // how much to advance past prefix (Compile Time Recursion)
    //inline constexpr const char* AdvPrefix(const char* inLiteral, const char* inPrefix, unsigned tail = 0)
    //{
    //  return *inPrefix ? (*inPrefix == inLiteral[tail] ? AdvPrefix(inLiteral, inPrefix + 1, tail + 1) : inLiteral) : inLiteral + tail;
    //}

    constexpr unsigned s_OffsetStart{ CTFV(FFN<void>()) };
    constexpr unsigned s_OffsetREnd{ CTL(FFN<void>() + s_OffsetStart + 4) };

    template <unsigned N> struct CSW { const char val[N]; };  // CStr Wrapper

    // Index Sequence with Short Type Name function (Designed for C++11)
    template <unsigned N, unsigned Gen = N, unsigned... Is>
    struct ISeq { static constexpr CSW<N + 1> STN(const char* inLiteral); };

    // Partial specialization for recursive floor (Designed for C++11)
    template <unsigned N, unsigned... Is>
    struct ISeq<N, 0, Is...> { static constexpr CSW<N + 1> STN(const char* inLiteral); };

    template <unsigned N, unsigned Gen, unsigned... Is>
    inline constexpr CSW<N + 1> ISeq<N, Gen, Is...>::STN(const char* inLiteral)
    {
      return ISeq<N, Gen - 1, Gen - 1, Is...>::STN(inLiteral);
    }

    template <unsigned N, unsigned... Is>
    inline constexpr CSW<N + 1> ISeq<N, 0, Is...>::STN(const char* inLiteral)
    {
      return { inLiteral[Is]..., '\0' };
    }

    template <typename T>
    struct TNH
    {
      static constexpr const unsigned s_Len{ CTL(FFN<T>() + s_OffsetStart) - s_OffsetREnd };
      static constexpr CSW<s_Len + 1> s_Str{ ISeq<s_Len>::STN(FFN<T>() + s_OffsetStart) };
    };

    // linkage for pre C++17 struct static inline constexpr
    template <typename T>
    constexpr unsigned TNH<T>::s_Len;

    // linkage for pre C++17 struct static inline constexpr
    template <typename T>
    constexpr CSW<TNH<T>::s_Len + 1> TNH<T>::s_Str;
  }

  /// @brief Get the Compile-Time known constant expression short name of a type
  /// @tparam T The type to get the name of
  /// @return string literal to the type name, null terminated for QOL
  template <typename T>
  CTNR_ATTRIB_FORCE_INLINE constexpr auto GetName() -> decltype(CTNR::Impl::TNH<T>::s_Str.val)&;

  template <typename T>
  CTNR_ATTRIB_FORCE_INLINE constexpr auto GetName() -> decltype(CTNR::Impl::TNH<T>::s_Str.val)&
  {
    return CTNR::Impl::TNH<T>::s_Str.val;
  }
}

#undef CTNR_ATTRIB_FORCE_INLINE

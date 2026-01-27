// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright © 2026      GSI Helmholtzzentrum fuer Schwerionenforschung GmbH
//                       Matthias Kretz <m.kretz@gsi.de>

/**
 * @file val.h
 * @brief Value-preserving literal types for C++26
 *
 * This header provides user-defined literal to produce untyped values that ensure their value is
 * preserved on conversion to arithmetic type, preventing precision loss and overflow.
 *
 * Requires C++26.
 */

#ifndef INCLUDE_VAL_H_
#define INCLUDE_VAL_H_

#if __cpp_impl_reflection >= 202506L && __cpp_concepts >= 202002L \
      && __cpp_deleted_function >= 202403L && __cpp_constexpr_exceptions >= 202411L

#include <concepts>
#include <exception>
#include <limits>
#include <source_location>
#include <string_view>

/**
 * @brief Feature macro for value-preserving literals.
 *
 * This is modelled after C++ feature macros.
 */
#define vir_lib_val_literal 202601L

/**
 * @namespace vir
 *
 * @note Do not import the complete namespace into your namespace. For using the user-defined
 * literal do `using vir::operator""_val;` and never `using namespace vir;`.
 */
namespace vir
{
  using std::integral;
  using std::signed_integral;
  using std::unsigned_integral;
  using std::floating_point;
  using std::source_location;
  using std::type_identity_t;
  using std::numeric_limits;
  using std::u8string_view;

  /** @internal
   * @brief Concept for arithmetic types
   *
   * Matches arithmetic types as defined by the language (no library types).
   */
  template <typename _Tp>
    concept __arithmetic = integral<_Tp> || floating_point<_Tp>;

  struct constinteger;

  struct constreal;

  /**
   * @brief Exception thrown when conversion to arithmetic type would change value.
   *
   * This exception is thrown at compile time (and can only be caught at compile time) when
   * attempting to convert a constinteger or constreal to an arithmetic type that cannot preserve
   * the exact value (e.g., overflow, precision loss, or range violation).
   *
   * The type is consteval-only to ensure it can only be used at compile time.
   */
  class bad_value_preserving_cast : public std::exception
  {
  private:
    /// Reflection poison to make the type consteval-only
    decltype(^^int) _M_poison;
    /// Source location where the error occurred
    source_location _M_where;

  public:
    /**
     * @brief Construct with source location
     *
     * @param __where Source location where the conversion failed
     */
    consteval
    bad_value_preserving_cast(source_location __where = source_location::current()) noexcept
    : _M_where{__where} {}

    /// Defaulted copy constructor
    consteval bad_value_preserving_cast(const bad_value_preserving_cast&) = default;
    /// Defaulted move constructor
    consteval bad_value_preserving_cast(bad_value_preserving_cast&&) = default;

    /// Defaulted copy assignment
    consteval bad_value_preserving_cast& operator=(const bad_value_preserving_cast&) = default;
    /// Defaulted move assignment
    consteval bad_value_preserving_cast& operator=(bad_value_preserving_cast&&) = default;

    /**
     * @brief Get error description
     *
     * @return const char* Error message
     */
    consteval const char*
    what() const noexcept override
    { return "conversion is not value-preserving"; }

    /**
     * @brief Get UTF-8 error description
     *
     * @return u8string_view UTF-8 error message
     */
    consteval u8string_view u8what() const noexcept
    { return u8"conversion is not value-preserving"; }

    /**
     * @brief Get source location of the failed conversion
     *
     * @return source_location Where the error occurred
     */
    consteval source_location where() const noexcept { return _M_where; }
  };

  /** @internal
   * @brief binary operators, compound assignment, and comparison operators for constinteger and
   * constreal
   *
   * The operators enable integration with arithmetic types.
   */
  struct _ConstBinaryOps
  {
    /** @internal
     * @brief Conversion wrapper for binary operations.
     *
     * This type helps to check conversions (consteval) before calling the binary operators, which
     * would otherwise "immediate escalate". Immediate escalation would make binary operators only
     * work with constant expressions in the other operand.
     *
     * @tparam _Tp Target arithmetic type
     */
    template <__arithmetic _Tp>
      struct _ConvertTo
      {
        const _Tp _M_value;

        /** @internal
         * @brief Convert from constinteger @p __x to arithmetic type _Tp.
         */
        consteval
        _ConvertTo(const constinteger& __x)
        : _M_value(__x)
        {}

        /** @internal
         * @brief Convert from constreal @p __x to arithmetic type _Tp.
         */
        consteval
        _ConvertTo(const constreal& __x)
        : _M_value(__x)
        {}
      };

    /** @internal
     * @brief Binary operators and compound assignment.
     */
#define _GLIBCXX_CONVERTTO_OP(constraint, op)                                                      \
    template <constraint _Tp>                                                                      \
      friend constexpr _Tp&                                                                        \
      operator op##=(_Tp& __a, _ConvertTo<type_identity_t<_Tp>> __b) noexcept                      \
      { return __a += __b._M_value; }                                                              \
                                                                                                   \
    template <constraint _Tp>                                                                      \
      friend constexpr _Tp                                                                         \
      operator op(_Tp __a, _ConvertTo<type_identity_t<_Tp>> __b) noexcept                          \
      { return __a op __b._M_value; }                                                              \
                                                                                                   \
    template <constraint _Tp>                                                                      \
      friend constexpr _Tp                                                                         \
      operator op(_ConvertTo<type_identity_t<_Tp>> __a, _Tp __b) noexcept                          \
      { return __a._M_value op __b; }

    _GLIBCXX_CONVERTTO_OP(__arithmetic, +)
    _GLIBCXX_CONVERTTO_OP(__arithmetic, -)
    _GLIBCXX_CONVERTTO_OP(__arithmetic, *)
    _GLIBCXX_CONVERTTO_OP(__arithmetic, /)
    _GLIBCXX_CONVERTTO_OP(integral, %)
    _GLIBCXX_CONVERTTO_OP(integral, &)
    _GLIBCXX_CONVERTTO_OP(integral, |)
    _GLIBCXX_CONVERTTO_OP(integral, ^)

#undef _GLIBCXX_CONVERTTO_OP

    /** @internal
     * @brief Comparison operators.
     */
#define _GLIBCXX_CONVERTTO_CMP(op)                                                                 \
    template <__arithmetic _Tp>                                                                    \
      friend constexpr bool                                                                        \
      operator op(_Tp __a, _ConvertTo<type_identity_t<_Tp>> __b) noexcept                          \
      { return __a op __b._M_value; }                                                              \
                                                                                                   \
    template <__arithmetic _Tp>                                                                    \
      friend constexpr bool                                                                        \
      operator op(_ConvertTo<type_identity_t<_Tp>> __a, _Tp __b) noexcept                          \
      { return __a._M_value op __b; }

    _GLIBCXX_CONVERTTO_CMP(==)
    _GLIBCXX_CONVERTTO_CMP(!=)
    _GLIBCXX_CONVERTTO_CMP(<=)
    _GLIBCXX_CONVERTTO_CMP(>=)
    _GLIBCXX_CONVERTTO_CMP(<)
    _GLIBCXX_CONVERTTO_CMP(>)

#undef _GLIBCXX_CONVERTTO_CMP
  };

  /**
   * @brief Untyped integer literal type.
   *
   * Represents an integer value with up to the precision of unsigned long long.
   *
   * Conversions to arithmetic types are only allowed when they are value-preserving.
   * Otherwise, bad_value_preserving_cast is thrown.
   */
  struct constinteger : _ConstBinaryOps
  {
    /// @internal The absolute value stored as unsigned long long
    unsigned long long _M_value;

    /// @internal Flag indicating if the value is negative
    bool _M_negative = false;

    /**
     * @brief Unary negation operator
     *
     * @note C++ only knows about positive literals. Subsequent unary minus produces a negative
     * value.
     */
    friend consteval constinteger
    operator-(constinteger __v) noexcept
    { return constinteger{{}, __v._M_value, !__v._M_negative}; }

    /**
     * @brief Unary plus operator (identity)
     *
     * @note Provided for completeness. (Useless.)
     */
    friend consteval constinteger
    operator+(constinteger __v) noexcept
    { return __v; }

    /**
     * @brief Bitwise complement operator (deleted)
     *
     * Cannot be applied because the width is unspecified.
     */
    friend consteval constinteger
    operator~(constinteger)
      = delete("complement cannot be applied to value of unspecified width");

    /**
     * @brief Logical NOT operator (deleted)
     *
     * Explicitly write 1 or 0 instead.
     */
    friend consteval constinteger
    operator!(constinteger) = delete("explicitly write 1 or 0 instead");

    /**
     * @brief Conversion operator to arithmetic types
     *
     * Converts to the target type only if the conversion is value-preserving.
     * Checks that the value fits within the target range and can be exactly
     * represented without precision loss.
     *
     * @tparam _Up Target arithmetic type
     * @return _Up Converted value
     * @throws bad_value_preserving_cast if conversion is not value-preserving
     */
    template <__arithmetic _Up>
      consteval
      operator _Up() const
      {
        using L = numeric_limits<_Up>;
        if constexpr (floating_point<_Up>)
          {
            if (static_cast<unsigned long long>(static_cast<_Up>(_M_value)) != _M_value)
              throw bad_value_preserving_cast();
            _Up r = static_cast<_Up>(_M_value);
            return _M_negative ? -r : r;
          }
        else
          {
            if ((_M_negative && _M_value > -static_cast<unsigned long long>(L::lowest()))
                  || (!_M_negative && _M_value > L::max()))
              throw bad_value_preserving_cast();
            return static_cast<_Up>(_M_negative ? -_M_value : _M_value);
          }
      }
  };

  /**
   * @brief User-defined literal for untyped constants.
   *
   * Creates a constinteger from a literal interpreted as unsigned long long.
   *
   * @param __x The literal value
   * @return constinteger Value-preserving integer constant
   */
  consteval constinteger
    operator""_val(unsigned long long __x) noexcept
  { return constinteger{{}, __x}; }

  /**
   * @brief Create untyped constant from typed value / constant expression.
   *
   * @param __x Unsigned integral value
   * @return constinteger Value-preserving integer constant
   */
  consteval constinteger
  val(unsigned_integral auto __x) noexcept
  { return constinteger{{}, __x}; }

  /**
   * @brief Create untyped constant from typed value / constant expression.
   *
   * @param __x Signed integral value
   * @return constinteger Value-preserving integer constant
   */
  consteval constinteger
  val(signed_integral auto __x) noexcept
  {
    if (__x >= 0)
      return constinteger{{}, static_cast<unsigned long long>(__x)};
    else
      return constinteger{{}, -static_cast<unsigned long long>(__x), true};
  }

  /**
   * @brief Untyped real literal type.
   *
   * Represents a real value with up to the precision of long double.
   *
   * Conversions to arithmetic types are only allowed when they are value-preserving.
   * Otherwise, bad_value_preserving_cast is thrown.
   */
  struct constreal : _ConstBinaryOps
  {
    /// @internal The real value stored as long double
    long double _M_value;

    /**
     * @brief Unary negation operator
     *
     * @note C++ only knows about positive literals. Subsequent unary minus produces a negative
     * value.
     */
    friend consteval constreal
    operator-(constreal __v) noexcept
    { return constreal{{}, -__v._M_value}; }

    /**
     * @brief Unary plus operator (identity)
     *
     * @note Provided for completeness. (Useless.)
     */
    friend consteval constreal
    operator+(constreal __v) noexcept
    { return __v; }

    /**
     * @brief Bitwise complement operator (deleted)
     *
     * Not applicable to floating-point types.
     */
    friend consteval constreal
    operator~(constreal) = delete;

    /**
     * @brief Logical NOT operator (deleted)
     *
     * Explicitly write 1 or 0 instead.
     */
    friend consteval constreal
    operator!(constreal) = delete("explicitly write 1 or 0 instead");

    /**
     * @copydoc constinteger::operator _Up()
     */
    template <__arithmetic _Up>
      consteval
      operator _Up() const
      {
        using L = numeric_limits<_Up>;
        if (_M_value > L::max() || _M_value < L::lowest())
          throw bad_value_preserving_cast();
        if (static_cast<long double>(static_cast<_Up>(_M_value)) != _M_value)
          throw bad_value_preserving_cast();
        return static_cast<_Up>(_M_value);
      }
  };

  /**
   * @brief User-defined literal for untyped constants
   *
   * Creates a constreal from a literal interpreted as long double.
   *
   * @param __x The literal value
   * @return constreal Value-preserving real constant
   */
  consteval constreal
    operator""_val(long double __x) noexcept
  { return constreal{{}, __x}; }

  /**
   * @brief Create untyped constant from typed value / constant expression.
   *
   * @param __x Floating-point value
   * @return constreal Value-preserving real constant
   */
  consteval constreal
  val(long double __x) noexcept
  { return constreal{{}, __x}; }
}

#endif

#endif  // INCLUDE_VAL_H_

// vim: ft=cpp

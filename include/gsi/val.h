// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright © 2026      GSI Helmholtzzentrum fuer Schwerionenforschung GmbH
//                       Matthias Kretz <m.kretz@gsi.de>

#ifndef INCLUDE_VAL_H_
#define INCLUDE_VAL_H_

#if __cpp_impl_reflection >= 202506L && __cpp_concepts >= 202002L \
      && __cpp_deleted_function >= 202403L && __cpp_constexpr_exceptions >= 202411L

#include <concepts>
#include <exception>
#include <limits>
#include <source_location>
#include <string_view>

#define gsi_lib_val_literal 202601L

namespace gsi
{
  using std::integral;
  using std::signed_integral;
  using std::unsigned_integral;
  using std::floating_point;
  using std::source_location;
  using std::type_identity_t;
  using std::numeric_limits;
  using std::u8string_view;

  template <typename _Tp>
    concept __arithmetic = integral<_Tp> || floating_point<_Tp>;

  struct constinteger;

  struct constreal;

  class bad_value_preserving_cast : public std::exception
  {
  private:
    decltype(^^int) _M_poison; // make the type consteval-only
    source_location _M_where;

  public:
    consteval
    bad_value_preserving_cast(source_location __where = source_location::current()) noexcept
    : _M_where{__where} {}

    consteval bad_value_preserving_cast(const bad_value_preserving_cast&) = default;
    consteval bad_value_preserving_cast(bad_value_preserving_cast&&) = default;

    consteval bad_value_preserving_cast& operator=(const bad_value_preserving_cast&) = default;
    consteval bad_value_preserving_cast& operator=(bad_value_preserving_cast&&) = default;

    consteval const char*
    what() const noexcept override
    { return "conversion is not value-preserving"; }

    consteval u8string_view u8what() const noexcept
    { return u8"conversion is not value-preserving"; }

    consteval source_location where() const noexcept { return _M_where; }
  };

  struct _ConstBinaryOps
  {
    template <typename _Tp>
      struct _ConvertTo
      {
        const _Tp _M_value;

        consteval
        _ConvertTo(const constinteger& x)
        : _M_value(x)
        {}

        consteval
        _ConvertTo(const constreal& x)
        : _M_value(x)
        {}
      };

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

  struct constinteger : _ConstBinaryOps
  {
    unsigned long long _M_value;

    bool _M_negative = false;

    friend consteval constinteger
    operator-(constinteger __v) noexcept
    { return constinteger{{}, __v._M_value, !__v._M_negative}; }

    friend consteval constinteger
    operator+(constinteger __v) noexcept
    { return __v; }

    friend consteval constinteger
    operator~(constinteger)
      = delete("complement cannot be applied to value of unspecified width");

    friend consteval constinteger
    operator!(constinteger) = delete("explicitly write 1 or 0 instead");

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

  consteval constinteger
    operator""_val(unsigned long long x) noexcept
  { return constinteger{{}, x}; }

  consteval constinteger
  val(unsigned_integral auto x) noexcept
  { return constinteger{{}, x}; }

  consteval constinteger
  val(signed_integral auto x) noexcept
  {
    if (x >= 0)
      return constinteger{{}, static_cast<unsigned long long>(x)};
    else
      return constinteger{{}, -static_cast<unsigned long long>(x), true};
  }

  struct constreal : _ConstBinaryOps
  {
    long double _M_value;

    friend consteval constreal
    operator-(constreal __v) noexcept
    { return constreal{{}, -__v._M_value}; }

    friend consteval constreal
    operator+(constreal __v) noexcept
    { return __v; }

    friend consteval constreal
    operator~(constreal) = delete;

    friend consteval constreal
    operator!(constreal) = delete("explicitly write 1 or 0 instead");

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

  consteval constreal
    operator""_val(long double x) noexcept
  { return constreal{{}, x}; }

  consteval constreal
  val(long double x) noexcept
  { return constreal{{}, x}; }
}

#endif

#endif  // INCLUDE_VAL_H_

// vim: ft=cpp

// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright © 2026      GSI Helmholtzzentrum fuer Schwerionenforschung GmbH
//                       Matthias Kretz <m.kretz@gsi.de>

#include <gsi/val.h>

using gsi::operator""_val;

static_assert([] {
  int a = 1_val;
  a += 0x2_val;
  a -= 02_val;
  a *= 0b11_val;
  a /= .2e1_val;
  float b = -0xf000'0000'0000'0000_val;
  b *= 2_val;
  b /= 0x100'0002_val;
  b += .5_val;
  short c = 100_val;
  c += 0x7000_val;
  c += -0x8000_val;
  c = c * gsi::val(1);
  try
    {
      b += 0x100'0001_val; // would need 25 mantissa bits
      return false;
    }
  catch(const gsi::bad_value_preserving_cast&) {}
  try
    {
      b += 0.1_val;
      return false;
    }
  catch(const gsi::bad_value_preserving_cast&) {}
  try
    {
      c += 0x8000_val; // larger than INT16_MAX
      return false;
    }
  catch(const gsi::bad_value_preserving_cast&) {}
  return true;
}());

constexpr int a = gsi::val(int(-0x8000'0000));
static_assert(a == -0x8000'0000_val);

int main()
{ return -0_val; }

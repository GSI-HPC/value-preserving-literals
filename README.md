# value-preserving literals

[![GCC](https://github.com/GSI-HPC/value-preserving-literals/actions/workflows/GCC.yml/badge.svg)](https://github.com/GSI-HPC/value-preserving-literals/actions/workflows/GCC.yml)
[![REUSE 
status](https://github.com/GSI-HPC/value-preserving-literals/actions/workflows/reuse.yml/badge.svg)](https://github.com/GSI-HPC/value-preserving-literals/actions/workflows/reuse.yml)
[![fair-software.eu](https://img.shields.io/badge/fair--software.eu-%E2%97%8F%20%20%E2%97%8F%20%20%E2%97%8B%20%20%E2%97%8F%20%20%E2%97%8B-orange)](https://fair-software.eu)

The `val.h` header defines the user-defined literal `_val` and an equivalent 
`val(x)` function. The returned object can (and must) be converted to any 
arithmetic type. If the value changes on conversion, 
`vir::bad_value_preserving_cast` is thrown. All of this happens at compile time 
(`consteval`).

## Example

```c++
#include <vir/val.h>

using vir::operator""_val;

auto displace(std::floating_point auto x)
{
  return x + 0x5EAF00D_val;
}

double a = displace(1.); // OK

float b = displace(1.f); // ill-formed
```

The `displace<float>` call fails because `int(float(0x5EAF00D)) != 0x5EAF00D`.

## Installation

```sh
cmake -B build -DCMAKE_INSTALL_PREFIX=$HOME/.local
cmake --build build --target install
```

Alternatively, a `Makefile` is provided which allows installation via

```sh
make install prefix=$HOME/.local
```

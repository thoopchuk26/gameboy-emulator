#pragma once
#include <array>
#include <iostream>
#include <string>

#include <fmt/core.h>
#include <stdint.h>
#include <stdlib.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

#define BIT(a, n) ((a & (1 << n)) ? 1 : 0)

#define BIT_SET(a, n, on) (on ? a |= (1 << n) : a &= ~(1 << n))

#define BETWEEN(a, b, c) ((a >= b) && (a <= c))

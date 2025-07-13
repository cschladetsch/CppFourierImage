#pragma once

#include <complex>
#include <type_traits>

// Single scalar type for all floating-point calculations
// Change this typedef to switch between float/double throughout the entire codebase
using Scalar = double;

// Ensure Scalar is a floating-point type
static_assert(std::is_floating_point_v<Scalar>, "Scalar must be a floating-point type");

// Complex number type based on our scalar
using Complex = std::complex<Scalar>;
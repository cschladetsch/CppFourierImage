#pragma once

// Single scalar type for all floating-point calculations
// Change this typedef to switch between float/double throughout the entire codebase
using Scalar = double;

// Complex number type based on our scalar
#include <complex>
using Complex = std::complex<Scalar>;
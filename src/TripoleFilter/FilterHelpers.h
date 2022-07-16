#pragma once

#include <cmath>

static inline double opl_rsqrt(double x)
{
  // @TODO: should probably use a faster approximation for this
  return 1.0 / std::sqrt(x);
}

static inline double opl_sigmoid(double x, double beta)
{
  auto tmp = x * x;
  auto tmp2 = tmp + beta;
  tmp = opl_rsqrt(tmp2);
  return tmp * x;
}

static inline double sech2_with_tanh(double tanh_x)
{
  return 1.0 - tanh_x * tanh_x;
}

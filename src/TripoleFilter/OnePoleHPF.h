#pragma once

#include "FilterHelpers.h"

struct OnePoleHPF
{
  OnePoleHPF() = default;

  void reset()
  {
    x1 = 0.0;
    y1 = 0.0;
  }

  inline double linOutput (double x_minus_x1_plus_z) const noexcept
  {
    return aCoef * x_minus_x1_plus_z;
  }

  inline double nonlinOutput (double x_minus_x1_plus_z, double tanh_y) const noexcept
  {
    return (-bCoef * tanh_y) + x_minus_x1_plus_z;
  }

  inline double getDerivative (double tanh_y) const noexcept
  {
    return (-bCoef * sech2_with_tanh (tanh_y)) - 1.0f;
  }

  inline double getXDerivative (double) const noexcept { return 2.0; }

  void setCoefs (double b, double a) { bCoef = b; aCoef = a; }

  inline double process (double x) const noexcept
  {
    auto x_minus_x1_plus_z = (x - x1) + y1;
    auto estimate = linOutput (x_minus_x1_plus_z);
    for (int i = 0; i < nIterStage; ++i)
    {
      auto tanh_y = opl_sigmoid (estimate, beta);
      auto residue = nonlinOutput (x_minus_x1_plus_z, tanh_y) - estimate;
      estimate = estimate - residue / getDerivative (tanh_y);
    }

    return estimate;
  }

  inline void updateState (double x, double y) noexcept
  {
    x1 = x;
    y1 = y;
  }

private:
  double aCoef;
  double bCoef;

  double y1;
  double x1;

  double beta = 1.0;
  static constexpr int nIterStage = 1;
};
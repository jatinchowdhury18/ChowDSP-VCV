#pragma once

#include "FilterHelpers.h"

struct OnePoleHPF_FB
{
  OnePoleHPF_FB() = default;

  void reset()
  {
    x_minus_x1_plus_z = 0.0;
    y1 = 0.0;
    x1 = 0.0;
  }

  inline double linOutput (double tanh_fb) const noexcept
  {
    return aCoef * ((bCoef * tanh_fb) + x_minus_x1_plus_z);
  }

  inline double nonlinOutput (double tanh_y, double tanh_fb) const noexcept
  {
    return (bCoef * (tanh_fb - tanh_y)) + x_minus_x1_plus_z;
  }

  inline double getDerivative (double tanh_y) const noexcept
  {
    return (-bCoef * sech2_with_tanh (tanh_y)) - 1.0f;
  }

  inline double getXDerivative (double tanh_fb) const noexcept
  {
    return bCoef * sech2_with_tanh (tanh_fb);
  }

  void setCoefs (double b, double a) { bCoef = b; aCoef = a; }

  inline void calcInput (double x) noexcept
  {
    x_minus_x1_plus_z = (x - x1) + y1;
  }

  inline double process (double tanh_fb, double) const noexcept
  {
    auto estimate = linOutput (tanh_fb);
    for (int i = 0; i < nIterStage; ++i)
    {
      auto tanh_y = opl_sigmoid (estimate, beta);
      auto residue = nonlinOutput (tanh_y, tanh_fb) - estimate;
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

  double x_minus_x1_plus_z;
  double y1;
  double x1;

  double beta = 1.0;
  static constexpr int nIterStage = 1;
};
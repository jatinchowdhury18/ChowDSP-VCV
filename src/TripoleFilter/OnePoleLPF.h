#pragma once

#include "FilterHelpers.h"

struct OnePoleLPF
{
  OnePoleLPF() = default;

  void reset()
  {
    y1 = 0.0;
    x1 = 0.0;
  }

  inline double linOutput (double x, double z) const noexcept
  {
    return aCoef * (bCoef * x + z);
  }

  inline double nonlinOutput (double tanh_x, double tanh_y, double z) const noexcept
  {
    return (bCoef * (tanh_x - tanh_y)) + z;
  }

  inline double getDerivative (double tanh_y) const noexcept
  {
    return (-bCoef * sech2_with_tanh (tanh_y)) - 1.0;
  }

  inline double getXDerivative (double tanh_x) const noexcept
  {
    return bCoef * sech2_with_tanh (tanh_x);
  }

  void setCoefs (double b, double a) { bCoef = b; aCoef = a; }

  inline double process (double tanh_x) const noexcept
  {
    auto estimate = linOutput(tanh_x, y1);
    for (int i = 0; i < nIterStage; ++i)
    {
      auto tanh_y = opl_sigmoid (estimate, beta);
      auto residue = nonlinOutput (tanh_x, tanh_y, y1) - estimate;
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

  double x1 = 0.0;
  double y1 = 0.0;

  double beta = 1.0;
  static constexpr int nIterStage = 1;
};

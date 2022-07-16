#pragma once

#include "FilterHelpers.h"

struct OnePoleLPF_FB
{
  OnePoleLPF_FB() = default;

  void reset()
  {
    bx = 0.0;
    y1 = 0.0;
    fb1 = 0.0;
  }

  inline double linOutput (double z_minus_fb_plus_fb1) const noexcept
  {
    return aCoef * (bx + z_minus_fb_plus_fb1);
  }

  inline double nonlinOutput (double tanh_x, double tanh_y, double z_minus_fb_plus_fb1) const noexcept
  {
    return (bCoef * (tanh_x - tanh_y)) + z_minus_fb_plus_fb1;
  }

  inline double getDerivative (double tanh_y) const noexcept
  {
    return (-bCoef * sech2_with_tanh (tanh_y)) - 1.0;
  }

  inline double getXDerivative (double) const noexcept { return 2.0; }

  inline void calcInput (double tanh_x) noexcept
  {
    bx = bCoef * tanh_x;
  }

  void setCoefs (double b, double a) { bCoef = b; aCoef = a; }

  inline double process (double tanh_x, double fb) const noexcept
  {
    auto z_minus_fb_plus_fb1 = (y1 - fb) + fb1;
    auto estimate = linOutput(z_minus_fb_plus_fb1);
    for (int i = 0; i < nIterStage; ++i)
    {
      auto tanh_y = opl_sigmoid (estimate, beta);
      auto residue = nonlinOutput (tanh_x, tanh_y, z_minus_fb_plus_fb1) - estimate;
      estimate = estimate - residue / getDerivative (tanh_y);
    }

    return estimate;
  }

  inline void updateState (double fb, double y) noexcept
  {
    fb1 = fb;
    y1 = y;
  }

private:
  double aCoef;
  double bCoef;

  double bx;
  double y1;
  double fb1;

  double beta = 1.0;
  static constexpr int nIterStage = 1;
};

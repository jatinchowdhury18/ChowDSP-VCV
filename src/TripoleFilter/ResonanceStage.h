#pragma once

#include "FilterHelpers.h"

namespace ResConstants
{
constexpr double alpha = 1.0168177;
const double log_alpha = std::log(alpha);
constexpr double beta = 9.03240196;
const double beta_exp = beta * log_alpha;
constexpr double c = 0.222161;
constexpr double bias = 8.2;

constexpr double max_val = 7.5;
constexpr double mult = 10.0;
constexpr double one = 0.99;
constexpr double oneOverMult = one / mult;
const double betaExpOverMult = beta_exp / mult;
}

struct ResonanceStage
{
  ResonanceStage() = default;

  void setRes (double newRes) { resK = newRes; }
  inline double getK() const noexcept { return resK; }

  inline double process (double x) noexcept
  {
    auto k_times_in = resK * x;
    auto resOut = (1.0 / resGain) * res_func (k_times_in * resGain);
    resDeriv = res_deriv_func (k_times_in);

    return resOut;
  }

  inline double getResDeriv() const noexcept { return resDeriv; }

  static inline double res_func (double x) noexcept
  {
    using namespace ResConstants;

    x = x * mult;

    auto x_abs = std::abs (x);
    auto y = -std::exp(beta_exp * -std::abs(x + c)) + bias;
    const auto sign_x = x >= 0.0 ? 1.0 : -1.0;
    y = sign_x * y + oneOverMult;

    return x_abs < max_val ? (x * oneOverMult) : y;
  }

  static inline double res_deriv_func (double x)
  {
    using namespace ResConstants;

    x = x * mult;

    auto x_abs = std::abs (x);
    auto y = std::exp (beta_exp * (-std::abs (x + c))) + betaExpOverMult;

    return x_abs < max_val ? 1.0 : y;
  }

private:
  double resK;
  double resDeriv;

  static constexpr double resGain = 1.5;
};

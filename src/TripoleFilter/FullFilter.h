#pragma once

#include "OnePoleLPF.h"
#include "OnePoleLPF_FB.h"
#include "OnePoleHPF.h"
#include "OnePoleHPF_FB.h"
#include "ResonanceStage.h"

class FullTripoleFilter
{
public:
  FullTripoleFilter() = default;

  void reset (double sampleRate)
  {
    fs = sampleRate;
    state = 0.0;

    filter0_lpf.reset();
    filter0_hpf.reset();
    filter1_lpf.reset();
    filter1_hpf.reset();
    filter2_lpf.reset();
    filter2_hpf.reset();
  }

  void setParameters (float freq0, float freq1, float freq2, float res)
  {
    const double T = 1.0 / fs;

    calcFilterCoefs (T, freq0, filter0_lpf, 1.0);
    calcFilterCoefs (T, freq0, filter0_hpf, 1.0);
    calcFilterCoefs (T, freq1, filter1_lpf, 0.998);
    calcFilterCoefs (T, freq1, filter1_hpf, 0.998);
    calcFilterCoefs (T, freq2, filter2_lpf, 1.0012);
    calcFilterCoefs (T, freq2, filter2_hpf, 1.0012);

    static constexpr double Iabc = 8.0;    // milliAmps
    static constexpr double Rload = 220.0; // kOhms
    const auto res_factor_db = std::log10 (Iabc * Rload);
    res = rack::clamp (0.0f, 1.0f, res);
    auto resK = -(std::pow(1.5, res_factor_db * (double) res) - 0.5);
    resStage.setRes (resK);
  }

  void setFilterTypes (int filter0Type, int filter1Type, int filter2Type)
  {
    inLPF = filter0Type == 0;
    stage1LPF = filter1Type == 0;
    stage2LPF = filter2Type == 0;
  }

  inline std::tuple<double, double, double> processSample (double x) noexcept
  {
    double tanh_x0 = 0.0;
    auto estimate = state;

    // @TODO: better approach here!
    if (inLPF)
    {
      tanh_x0 = opl_sigmoid (x, 1.0);
      filter0_lpf.calcInput (tanh_x0);
    }
    else
    {
      filter0_hpf.calcInput (x);
    }

    double estimate0, estimate1, resOut, estimate2;
    for (int i = 0; i < nIterGlobal; ++i)
    {
      double f0_deriv;
      if (inLPF)
      {
        estimate0 = filter0_lpf.process (tanh_x0, state);
        f0_deriv = filter0_lpf.getXDerivative (tanh_x0);
      }
      else
      {
        auto tanh_fb = opl_sigmoid (estimate, 1.0);
        estimate0 = filter0_hpf.process (tanh_fb, state);
        f0_deriv = filter0_hpf.getXDerivative (tanh_fb);
      }

      double f1_deriv;
      if (stage1LPF)
      {
        auto tanh_x1 = opl_sigmoid (estimate0, 1.0);
        estimate1 = filter1_lpf.process (tanh_x1);
        f1_deriv = filter1_lpf.getXDerivative (tanh_x1);
      }
      else
      {
        estimate1 = filter1_hpf.process (estimate0);
        f1_deriv = filter1_hpf.getXDerivative (estimate0);
      }

      resOut = resStage.process (estimate1);
      auto res_deriv = resStage.getResDeriv();

      double f2_deriv;
      if (stage2LPF)
      {
        auto tanh_x2 = opl_sigmoid (resOut, 1.0);
        estimate2 = filter2_lpf.process (tanh_x2);
        f2_deriv = filter2_lpf.getXDerivative (tanh_x2);
      }
      else
      {
        estimate2 = filter2_hpf.process (resOut);
        f2_deriv = filter2_hpf.getXDerivative (resOut);
      }

      auto num = estimate - estimate2;
      auto den = 1.0 - (resStage.getK() * res_deriv * f0_deriv * f1_deriv * f2_deriv);
      estimate -= (num / den);
    }

    filter0_lpf.updateState (state, estimate0);
    filter0_hpf.updateState (x, estimate0);

    filter1_lpf.updateState (estimate0, estimate1);
    filter1_hpf.updateState (estimate0, estimate1);
    filter2_lpf.updateState (resOut, estimate2);
    filter2_hpf.updateState (resOut, estimate2);

    state = estimate;

    return std::make_tuple (estimate0, estimate1, estimate);
  }

  void processBlock (float* left, float* right, const int numSamples);

private:
  template <typename FilterType>
  void calcFilterCoefs (double T, float freq, FilterType& filter, double randOffset)
  {
    static constexpr double capVal = 220.0e-12;
    freq = rack::clamp (10.0f, (float) fs * 0.24f, freq);
    const auto wc = 2.0 * M_PI * (double) freq * T;
    const auto g = (capVal / T) * (std::exp (wc) - 1.0);
    const auto b = randOffset * T * g / capVal;
    const auto a = 1.0 / (1.0 + b);
    filter.setCoefs (b, a);
  };

  double fs = 48000.0;

  OnePoleLPF_FB filter0_lpf;
  OnePoleHPF_FB filter0_hpf;

  OnePoleLPF filter1_lpf;
  OnePoleHPF filter1_hpf;

  ResonanceStage resStage;

  OnePoleLPF filter2_lpf;
  OnePoleHPF filter2_hpf;

  int outStage = 0;

  bool inLPF, stage1LPF, stage2LPF;

  double state;
  static constexpr int nIterGlobal = 4;
};

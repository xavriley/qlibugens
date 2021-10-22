// PluginBitstreamAutocorrelation.hpp
// Xavier Riley (xavriley@hotmail.com)
#pragma once

#include <q/support/literals.hpp>
#include <q/pitch/pitch_detector.hpp>
#include <q/fx/envelope.hpp>
#include <q/fx/lowpass.hpp>
#include <q/fx/biquad.hpp>
#include <q/fx/dynamic.hpp>
#include <q/fx/waveshaper.hpp>
#include "SC_PlugIn.hpp"

namespace QlibUGens {

class BitstreamAutocorrelation : public SCUnit {
public:
    BitstreamAutocorrelation();

    cycfi::q::pd_preprocessor::config cfg;
    std::shared_ptr<cycfi::q::pitch_detector> m_pd;
    std::shared_ptr<cycfi::q::pd_preprocessor> m_pp;
    float m_frequency = 0.0f;
    // Destructor
    // ~BitstreamAutocorrelation();

private:
    // Calc function
    void next(int nSamples);

    // Member variables
};

} // namespace QlibUGens

// PluginBitstreamAutocorrelation.hpp
// Xavier Riley (@xavriley)
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

        ~BitstreamAutocorrelation();

        cycfi::q::pitch_detector *m_pd;
        cycfi::q::pd_preprocessor *m_pp;
        float m_frequency = 0.0f;
        float m_lowest_detectable_frequency = 80.0f;
        float m_highest_detectable_frequency = 1280.0f;

    private:
        // Calc function
        void next(int nSamples);
    };

} // namespace QlibUGens

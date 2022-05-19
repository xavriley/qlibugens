// PluginBitstreamAutocorrelation.hpp
// Xavier Riley (@xavriley)
#pragma once

#include "ScaleMapper.cpp"
#include <q/support/literals.hpp>
#include <q/pitch/pitch_detector.hpp>
#include <q/fx/envelope.hpp>
#include <q/fx/lowpass.hpp>
#include <q/fx/biquad.hpp>
#include <q/fx/dynamic.hpp>
#include <q/fx/waveshaper.hpp>
#include "SC_PlugIn.hpp"

namespace QlibUGens {

    class BitstreamPitchCorrection : public SCUnit {
    public:
        BitstreamPitchCorrection();

        ~BitstreamPitchCorrection();

        // pitch detection
        cycfi::q::pitch_detector *m_pd;
        cycfi::q::pd_preprocessor *m_pp;
        float m_frequency = 0.0f;
        float m_read_rate = 1.0f;
        float m_lowest_detectable_frequency = 80.0f;
        float m_highest_detectable_frequency = 1280.0f;


        // ring buffer
        float maxdelay;
        int bufsize;
        int mask;
        float* buf;
        int writephase;
        float readphase;

        // scale mapping
        const static inline ScaleMapper sc;
        int chosen_key = 0;
        int chosen_scale_index = 0;
        std::vector<ScaleMapper::ToneRange> chosen_scale;
        float transpose = 0;
        float target_note = -1;

    private:
        void next(int nSamples);
        const float freqToRate(const float freq);
    };

} // namespace QlibUGens

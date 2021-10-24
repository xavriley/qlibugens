// PluginBitstreamAutocorrelation.cpp
// Xavier Riley (@xavriley)
#include "SC_PlugIn.hpp"
#include "BitstreamAutocorrelation.hpp"

static InterfaceTable* ft;

namespace QlibUGens {

using namespace cycfi::q::literals;

BitstreamAutocorrelation::BitstreamAutocorrelation() :
            m_pd(std::make_shared<cycfi::q::pitch_detector>(60_Hz, 600_Hz, sampleRate(), -45_dB)),
            m_pp(std::make_shared<cycfi::q::pd_preprocessor>(cfg, 60_Hz, 600_Hz, sampleRate()))
    {
        m_pp = std::make_shared<cycfi::q::pd_preprocessor>(cfg, 60_Hz, 600_Hz, sampleRate());
        m_pd = std::make_shared<cycfi::q::pitch_detector>(60_Hz, 600_Hz, sampleRate(), -45_dB);

        mCalcFunc = make_calc_function<BitstreamAutocorrelation, &BitstreamAutocorrelation::next>();

        m_frequency = 0.0f;
        next(1);
    }

void BitstreamAutocorrelation::next(int nSamples) {
    auto unit = this;
    const float* input = in(0);
    float* outbuf = out(0);

    for(int i = 0; i < nSamples; i++) {
        float s = m_pp->operator()(input[i]);
        bool is_ready = m_pd->operator()(s);

        if (is_ready) {
            outbuf[i] = m_frequency = m_pd->get_frequency();
        } else {
            outbuf[i] = m_frequency;
        }
    }
}

} // namespace QlibUGens

PluginLoad(BitstreamAutocorrelationUGens) {
    // Plugin magic
    ft = inTable;
    registerUnit<QlibUGens::BitstreamAutocorrelation>(ft, "BitstreamAutocorrelation", true);
}

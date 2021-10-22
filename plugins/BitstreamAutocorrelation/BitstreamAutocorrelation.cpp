// PluginBitstreamAutocorrelation.cpp
// Xavier Riley (xavriley@hotmail.com)
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
    float* outbuf = ZOUT(0);
//    float* freq = ZOUT(0);
//    float* predicted = ZOUT(1);
//    float* periodicity = ZOUT(2);

    /* Period Detection */
    auto nAudioSamples = inBufferSize(0);

    for (int i = 0; i < nAudioSamples; ++i) {
        float s = m_pp->operator()(input[i]);
	    bool is_ready = m_pd->operator()(s);

	    if (is_ready) {
		    m_frequency = m_pd->get_frequency();
            // Print("freq: %f\n", m_frequency);
            ZXP(outbuf) = m_frequency;
	    }
    }

}

} // namespace QlibUGens

PluginLoad(BitstreamAutocorrelationUGens) {
    // Plugin magic
    ft = inTable;
    registerUnit<QlibUGens::BitstreamAutocorrelation>(ft, "BitstreamAutocorrelation", true);
}

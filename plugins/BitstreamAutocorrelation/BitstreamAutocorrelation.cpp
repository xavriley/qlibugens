// PluginBitstreamAutocorrelation.cpp
// Xavier Riley (@xavriley)
#include <iostream>
#include <map>
#include "SC_PlugIn.hpp"
#include "BitstreamAutocorrelation.hpp"

static InterfaceTable *ft;

namespace QlibUGens {
    using namespace cycfi::q::literals;

    BitstreamAutocorrelation::BitstreamAutocorrelation() {
        // Notes for myself - some options on initialization
        // a) put m_pd on the stack, using an initializer member list
        //       - this is ok if the resulting object is small and stable but it would require testing to be sure
        //       - also makes it trickier to pass UGen inputs as params
        // b) use placement new to assign the memory with RTAlloc first
        //       - less of a worry about object size
        //       - allows setup to be done in stages, getting and checking the input params first
        //       - not always necessary though, and slightly harder to read
        // c) use std::make_shared with a custom allocator
        //       - in crazier scenarios (like the SC VST plugin) this makes sense but avoid otherwise
        //       e.g. https://git.iem.at/pd/vstplugin/-/blob/master/sc/src/rt_shared_ptr.hpp

        // These are specific to the pitch detection algo
        // Lower frequencies introduce longer latencies in detection, so aim to minimize this
        // Higher frequencies require marginally more computation time but it isn't as critical
        //          - if in doubt, set it high
        m_lowest_detectable_frequency = in0(1);
        m_highest_detectable_frequency = in0(2);

        void *pp_mem = RTAlloc(this->mWorld, sizeof(cycfi::q::pd_preprocessor));
        void *pd_mem = RTAlloc(this->mWorld, sizeof(cycfi::q::pitch_detector));


        // do nullptr check first https://scsynth.org/t/writing-a-plugin-which-takes-audio-and-outputs-control-rate/4733/11?u=xavriley
        if (!pp_mem || !pd_mem) {
            Print("BitstreamAutocorrelation: RT memory allocation failed, try increasing the real-time memory size in the server options\n");
            pp_mem = nullptr;
            pd_mem = nullptr;
            goto end;
        }

        this->m_pp = new(pp_mem)
                cycfi::q::pd_preprocessor(cycfi::q::pd_preprocessor::config{},
                                          cycfi::q::frequency(m_lowest_detectable_frequency),
                                          cycfi::q::frequency(m_highest_detectable_frequency),
                                          sampleRate());

        this->m_pd = new(pd_mem)
                cycfi::q::pitch_detector(cycfi::q::frequency(m_lowest_detectable_frequency),
                                         cycfi::q::frequency(m_highest_detectable_frequency),
                                         sampleRate(),
                                         -45_dB);

        mCalcFunc = make_calc_function<BitstreamAutocorrelation, &BitstreamAutocorrelation::next>();

        m_frequency = 0.0f;
        next(1);

        end:
        // Informed by https://github.com/grame-cncm/faust/issues/13
        ClearUnitOutputs(this, 1);
    }

    BitstreamAutocorrelation::~BitstreamAutocorrelation() {
        RTFree(mWorld, m_pp);
        RTFree(mWorld, m_pd);
    }

    void BitstreamAutocorrelation::next(int nSamples) {
        auto unit = this;
        const float *input = in(0);
        float *outbuf = out(0);

        for (int i = 0; i < nSamples; i++) {
            float s = (*m_pp)(input[i]);
            bool is_ready = (*m_pd)(s);

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

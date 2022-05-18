// PluginBitstreamAutocorrelation.cpp
// Xavier Riley (@xavriley)
#include <iostream>
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

    BitstreamPitchCorrection::BitstreamPitchCorrection() {
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

        // ring buffer code
        this->maxdelay = 0.04; // in seconds - determines latency of the effect
        this->bufsize = NEXTPOWEROFTWO((float) sampleRate() * this->maxdelay);
        this->mask = this->bufsize - 1;
        this->writephase = int(this->bufsize / 2);
        this->readphase = 0.0f;

        this->buf = (float *) RTAlloc(this->mWorld, this->bufsize * sizeof(float));

        if (this->buf == NULL) {
            Print("BitstreamPitchCorrection: RT memory allocation failed, try increasing the real-time memory size in the server options\n");
            this->buf = nullptr;
            goto end;
        }

        // Fill the buffer with zeros.
        memset(this->buf, 0, this->bufsize * sizeof(float));

        mCalcFunc = make_calc_function<BitstreamPitchCorrection, &BitstreamPitchCorrection::next>();

        m_frequency = 0.0f;
        m_read_rate = 1.0f;
        next(1);

        end:
        // Informed by https://github.com/grame-cncm/faust/issues/13
        ClearUnitOutputs(this, 1);

    }

    void BitstreamPitchCorrection::next(int nSamples) {
        const float *input = in(0);
        float *outbuf = out(0);

        float *buf = this->buf;
        int mask = this->mask;
        int writephase = this->writephase;
        float readphase = this->readphase;
        float delay_samples = this->bufsize;
        const float transpose = in0(3);

        for (int i = 0; i < nSamples; i++) {
            buf[writephase] = input[i];
            writephase = (writephase + 1) & mask;

            int offset = int(readphase);
            int offset_plus_one = (offset + 1) & mask;
            float frac = readphase - offset;

            // pre-processed sample into pitch tracker
            float s = (*m_pp)(input[i]);
            bool is_ready = (*m_pd)(s);

            if (is_ready) {
                m_frequency = m_pd->get_frequency();
            }

            if(m_frequency > m_lowest_detectable_frequency && m_frequency < m_highest_detectable_frequency) {
                m_read_rate = BitstreamPitchCorrection::freqToRate(m_frequency, transpose);

                int current_period = int(sampleRate() / m_frequency);
                offset = int(readphase);
                frac = readphase - offset;
                float pointer_gap = (int(writephase - offset + delay_samples) & mask) + frac;

                while(m_read_rate > 1 && current_period > pointer_gap) {
                    readphase = (int(int(readphase) - current_period + delay_samples) & mask) + frac;
                    offset = int(readphase);
                    frac = readphase - offset;
                    pointer_gap = (int(writephase - offset + delay_samples) & mask) + frac;
                }

                while(m_read_rate < 1 && (pointer_gap + current_period) > delay_samples) {
                    readphase = (int(int(readphase) + current_period + delay_samples) & mask) + frac;
                    offset = int(readphase);
                    frac = readphase - offset;
                    pointer_gap = (int(writephase - offset + delay_samples) & mask) + frac;
                }

                offset = int(readphase);
                offset_plus_one = (offset + 1) & mask;
                frac = readphase - offset;

                outbuf[i] = ((1 - frac) * buf[offset]) + (frac * buf[offset_plus_one]);
            } else {
                m_read_rate = 1.0f;

                offset = int(readphase);
                offset_plus_one = (offset + 1) & mask;
                frac = readphase - offset;

                outbuf[i] = ((1 - frac) * buf[offset]) + (frac * buf[offset_plus_one]);
            }

            readphase = readphase + m_read_rate;
            offset = int(readphase);
            frac = readphase - offset;
            readphase = (offset & mask) + frac;
        }

        this->writephase = writephase;
        this->readphase = readphase;
    }

    BitstreamPitchCorrection::~BitstreamPitchCorrection() {
        RTFree(mWorld, buf);
    }

    float BitstreamPitchCorrection::freqToRate(const float freq, const float transpose) {
        float midi = (12 * std::log2f(freq / 440.0) + 69);
        // midi = mapToMusicalScale(gChosenKey, gChosenScale, midi);
        // instead of mapping to a scale we will round to chromatic for now
        midi = round(midi);
        midi += transpose;
        float nearest_freq = powf(2, (midi - 69) / 12) * 440.0;

        return nearest_freq / freq;
    }

} // namespace QlibUGens

PluginLoad(BitstreamAutocorrelationUGens) {
    // Plugin magic
    ft = inTable;
    registerUnit<QlibUGens::BitstreamAutocorrelation>(ft, "BitstreamAutocorrelation", true);
    registerUnit<QlibUGens::BitstreamPitchCorrection>(ft, "BitstreamPitchCorrection", false);
}

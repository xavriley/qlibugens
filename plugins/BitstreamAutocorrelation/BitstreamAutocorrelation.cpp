// PluginBitstreamAutocorrelation.cpp
// Xavier Riley (xavriley@hotmail.com)

#include "SC_PlugIn.hpp"
#include "BitstreamAutocorrelation.hpp"

static InterfaceTable* ft;

namespace QlibUGens {

BitstreamAutocorrelation::BitstreamAutocorrelation() {
    mCalcFunc = make_calc_function<BitstreamAutocorrelation, &BitstreamAutocorrelation::next>();
    next(1);
}

void BitstreamAutocorrelation::next(int nSamples) {
    const float* input = in(0);
    const float* gain = in(1);
    float* outbuf = out(0);

    // simple gain function
    for (int i = 0; i < nSamples; ++i) {
        outbuf[i] = input[i] * gain[i];
    }
}

} // namespace QlibUGens

PluginLoad(BitstreamAutocorrelationUGens) {
    // Plugin magic
    ft = inTable;
    registerUnit<QlibUGens::BitstreamAutocorrelation>(ft, "BitstreamAutocorrelation", false);
}

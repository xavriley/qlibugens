// PluginBitstreamAutocorrelation.hpp
// Xavier Riley (xavriley@hotmail.com)

#pragma once

#include "SC_PlugIn.hpp"

namespace QlibUGens {

class BitstreamAutocorrelation : public SCUnit {
public:
    BitstreamAutocorrelation();

    // Destructor
    // ~BitstreamAutocorrelation();

private:
    // Calc function
    void next(int nSamples);

    // Member variables
};

} // namespace QlibUGens

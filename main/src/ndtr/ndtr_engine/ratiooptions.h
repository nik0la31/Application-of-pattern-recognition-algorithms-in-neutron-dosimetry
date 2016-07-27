#ifndef RATIOOPTIONS_H
#define RATIOOPTIONS_H

#include <string>

class RatioOptions
{
public:
    RatioOptions();

    float PixelsPerUnit;

    float XCenterOffset;

    float YCenterOffset;

    /// [um] 6  - 1 : e-06
    /// [nm] 9  - 1 : e-09
    /// [pm] 12 - 1 : e-12
    std::string Unit;
};

#endif // RATIOOPTIONS_H

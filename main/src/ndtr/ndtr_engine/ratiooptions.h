#ifndef RATIOOPTIONS_H
#define RATIOOPTIONS_H


class RatioOptions
{
public:
    RatioOptions();

    float PixelsPerUnit;

    float XCenterOffset;

    float YCenterOffset;

    /// Ratio
    /// [/]  0   -
    /// [um] 6  - 1 : e-06
    /// [nm] 9  - 1 : e-09
    /// [pm] 12 - 1 : e-12
    int BaseRatio;
};

#endif // RATIOOPTIONS_H

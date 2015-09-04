#ifndef TRACE_H
#define TRACE_H

#include <opencv2/core/core.hpp>

class Trace
{
public:
    Trace();

    Trace(int argX, int argY, double argAngle, int argD1, int argD2, int argIntensity);

    int x;
    int y;
    double angle;
    int diameter1;
    int diameter2;
    int intensity;

    cv::Scalar DebugColor;
};

#endif // TRACE_H

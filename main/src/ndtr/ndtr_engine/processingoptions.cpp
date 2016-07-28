#include "processingoptions.h"

ProcessingOptions::ProcessingOptions()
    : AutomaticOtsuThreshold(true),
      OtsuThreshold(125),
      GaussianBlur(false),
      WoB(true),
      MinTraceDiameter(3.0f),
      MaxTraceDiameter(100.0f),
      AutoDetectWob(true)
{

}

bool ProcessingOptions::Equal(ProcessingOptions& options)
{
    bool equal =
            MinTraceDiameter == options.MinTraceDiameter &&
            MaxTraceDiameter == options.MaxTraceDiameter &&
            GaussianBlur == options.GaussianBlur &&
            WoB == options.GaussianBlur &&
            AutomaticOtsuThreshold == options.AutomaticOtsuThreshold &&
            AutoDetectWob == options.AutoDetectWob;

    if (equal && !AutomaticOtsuThreshold)
    {
        equal = equal && OtsuThreshold == options.OtsuThreshold;
    }

    return equal;
}

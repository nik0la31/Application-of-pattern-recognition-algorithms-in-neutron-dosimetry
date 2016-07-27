#include "processingoptions.h"

ProcessingOptions::ProcessingOptions()
    : AutomaticOtsuThreshold(true),
      OtsuThreshold(125),
      GaussianBlur(false),
      WoB(true),
      MinTraceDiameter(3),
      MaxTraceDiameter(100),
      AutoDetectWob(true),
      KeepManualEdits(true)
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
            AutoDetectWob == options.AutoDetectWob &&
            KeepManualEdits == options.KeepManualEdits;

    if (equal && !AutomaticOtsuThreshold)
    {
        equal = equal && OtsuThreshold == options.OtsuThreshold;
    }

    return equal;
}

#ifndef IMAGEPROCESSINGOPTIONS_H
#define IMAGEPROCESSINGOPTIONS_H

class ProcessingOptions
{
public:
    ProcessingOptions();

    bool AutomaticOtsuThreshold;
    int OtsuThreshold;

    bool GaussianBlur;

    bool AutoDetectWob;
    bool WoB;

    float MinTraceDiameter;
    float MaxTraceDiameter;

    bool Equal(ProcessingOptions& options);
};

#endif // IMAGEPROCESSINGOPTIONS_H

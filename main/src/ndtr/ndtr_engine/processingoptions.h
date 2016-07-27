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

    int MinTraceDiameter;
    int MaxTraceDiameter;

    bool Equal(ProcessingOptions& options);
};

#endif // IMAGEPROCESSINGOPTIONS_H

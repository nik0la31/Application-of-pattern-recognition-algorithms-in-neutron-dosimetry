#ifndef VIEWOPTIONS_H
#define VIEWOPTIONS_H

#include <opencv2/core/core.hpp>

enum ImageType
{
    NDTR_ORIGINAL = 0,
    NDTR_GRAYSCALE,
    NDTR_BLACK_WHITE,
    NDTR_BLACK_BACKGROUND,
    NDTR_WHITE_BACKGROUND,
};

enum ShapeType
{
    NDTR_CONTOUR,
    NDTR_ELLIPSE
};

struct ViewOptions
{
public:
    ViewOptions();

    ImageType Image;
    ShapeType Shape;

    bool Fill;
    cv::Scalar FillColor;

    bool Edge;
    cv::Scalar EdgeColor;

    bool Center;
    cv::Scalar CenterColor;

    bool ApplyOffsetTransform;
};

#endif // VIEWOPTIONS_H

#ifndef DOCUMENT_H
#define DOCUMENT_H

#include <memory>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <string>
#include <vector>

typedef std::vector<cv::Point> Contuor;

// Forward declatation.
class Project;


enum ImageType
{
    NDTR_ORIGINAL = 0,
    NDTR_GRAYSCALE,
    NDTR_BLACK_WHITE,
    NDTR_BLACK_BACKGROUND,
    NDTR_WHITE_BACKGROUND
};

enum ShapeType
{
    NDTR_CONTOUR,
    NDTR_ELLIPSE
};

class Document
{
public:
    Document()
        : m_Images(3)
    {

    }

    void Init(Project*, std::wstring& name, std::wstring& path);

    Project* GetProject();

    std::wstring GetName();

    cv::Mat GetImage(
            ImageType imgType,
            ShapeType shapeType,
            bool fill,
            bool edge,
            bool center);

private:
    // Project - document parent.
    Project *m_Project;

    // Image name.
    std::wstring m_Name;

    // Image path;
    std::wstring m_Path;

    // Image dimensions.
    unsigned int m_Width;
    unsigned int m_Height;

    // Images
    std::vector<cv::Mat> m_Images;

    // Contuors.
    std::vector<Contuor> m_Contuors;

    // Ellipses.
    std::vector<cv::RotatedRect> m_Ellipses;
};

#endif // DOCUMENT_H

#include "document.h"

#include <stdlib.h>
#include <stdio.h>

using namespace cv;
using namespace std;

void Document::Init(Project* project, wstring& name, wstring& path)
{
    m_Project = project;
    m_Name = name;
    m_Path = path;

    // Load image.
    std::string p(path.begin(), path.end());
    m_Images[NDTR_ORIGINAL] = imread(p, CV_LOAD_IMAGE_COLOR);

    // Get image dimensions.
    m_Width = m_Images[NDTR_ORIGINAL].cols;
    m_Height = m_Images[NDTR_ORIGINAL].rows;

    // Convert image to grayscale.
    cvtColor(m_Images[NDTR_ORIGINAL], m_Images[NDTR_GRAYSCALE], CV_BGR2GRAY);

    // Calculate black-white image.
    threshold(m_Images[NDTR_GRAYSCALE],
              m_Images[NDTR_BLACK_WHITE],
              0,
              255,
              CV_THRESH_BINARY | CV_THRESH_OTSU);

    // Get contuors.
    findContours(m_Images[NDTR_BLACK_WHITE],
                 m_Contuors,
                 CV_RETR_LIST,
                 CV_CHAIN_APPROX_SIMPLE);

    // Calculate ellipses.
    m_Ellipses.resize(m_Contuors.size());
    for (int i = 0; i < static_cast<int>(m_Contuors.size()); i++)
    {
        if (m_Contuors[i].size() < 5)
        {
            continue;
        }
        m_Ellipses[i] = fitEllipse(Mat(m_Contuors[i]));
    }
}

wstring Document::GetName()
{
    return m_Name;
}

Mat Document::GetImage(
        ImageType imgType,
        ShapeType shapeType,
        bool fill,
        bool edge,
        bool center)
{
    // Resulrting image in RGB.
    Mat result;

    // Init resulting matrix.
    switch (imgType)
    {
    case NDTR_ORIGINAL:
        cvtColor(m_Images[NDTR_ORIGINAL], result, CV_BGR2RGB);
        break;
    case NDTR_GRAYSCALE:
        cvtColor(m_Images[NDTR_GRAYSCALE], result, CV_GRAY2RGB);
        break;
    case NDTR_BLACK_WHITE:
        cvtColor(m_Images[NDTR_BLACK_WHITE], result, CV_GRAY2BGR);
        result = result * 255;
        break;
    case NDTR_BLACK_BACKGROUND:
         result = Mat::zeros(m_Height, m_Width, CV_8UC3);
        break;
    case NDTR_WHITE_BACKGROUND:
        result = Mat::zeros(m_Height, m_Width, CV_8UC3);
        result = cv::Scalar(255, 255, 255);
        break;
    }

    if (fill)
    {
        Scalar color = Scalar(0, 255, 0);
        if (shapeType == NDTR_CONTOUR)
        {
            drawContours(result, m_Contuors, -1, color, -1);
        }
        else
        {
            for (auto e : m_Ellipses)
            {
                ellipse(result, e, color, -1);
            };
        }
    }

    if (edge)
    {
        Scalar color = Scalar(0, 0, 255);
        if (shapeType == NDTR_CONTOUR)
        {
            drawContours(result, m_Contuors, -1, color, 1);
        }
        else
        {
            for (auto e : m_Ellipses)
            {
                ellipse(result, e, color, 1);
            };
        }
    }

    if (center)
    {
        Scalar color = Scalar(204, 0, 204);
        for (auto e : m_Ellipses)
        {
            circle(result, e.center, 1, color, -1);
        };
    }

    return result;
}

Project* Document::GetProject()
{
    return m_Project;
}

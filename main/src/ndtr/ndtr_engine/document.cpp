#include "document.h"

#include <stdlib.h>
#include <stdio.h>

#include <opencv2/features2d/features2d.hpp>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include "opencv2/video/tracking.hpp"

using namespace cv;
using namespace std;

void Document::Init(Project* project, string name, string path)
{
    m_Project = project;
    m_Name = name;
    m_Path = path;

    // Load image.
    m_Images[NDTR_ORIGINAL] = imread(path, CV_LOAD_IMAGE_COLOR);

    // Get image dimensions.
    m_Width = m_Images[NDTR_ORIGINAL].cols;
    m_Height = m_Images[NDTR_ORIGINAL].rows;

    ProcessImage();
}

void Document::Process(ProcessingOptions& options)
{
    m_Options = options;

    ProcessImage();
}

Project* Document::GetProject()
{
    return m_Project;
}

string Document::GetName()
{
    return m_Name;
}

std::string Document::GetExt()
{
    int pos = m_Path.find_last_of(L'.');
    return m_Path.substr(pos);
}

void mean(vector<int>& array, float& avg, int& min, int& max)
{
    if (array.size() > 0)
    {
        avg=0;
        min = array[0];
        max = array[0];

        for(size_t i=0; i<array.size(); ++i)
        {
            avg += array[i];

            if (array[i] < min)
                min = array[i];

            if (array[i] > max)
                max = array[i];
        }
        avg = avg/array.size();
    }
}

const Stats Document::GetStats()
{
    Stats stats;

    stats.TracesCount = m_Traces.size();

    vector<int> diameters;
    diameters.reserve(2*m_Traces.size());
    vector<int> intensities;
    intensities.reserve(m_Traces.size());

    int i = -1;
    for(auto& trace : m_Traces)
    {
        i++;

        diameters.push_back(trace.diameter1);
        diameters.push_back(trace.diameter2);

        intensities.push_back(trace.intensity);
    }

    float avg;
    int min;
    int max;

    mean(diameters, avg, min, max);

    stats.MinDiameter = min;
    stats.MaxDiameter = max;
    stats.AverageDiameter = static_cast<int>(avg);

    mean(intensities, avg, min, max);

    stats.MinIntensity = min;
    stats.MaxIntensity = max;
    stats.AverageIntensity = static_cast<int>(avg);

    return stats;
}

ProcessingOptions Document::GetOptions()
{
    return m_Options;
}

std::vector<Trace>& Document::GetTraces()
{
    return m_Traces;
}

cv::Mat Document::GatTransform()
{
    return m_Transform;
}

void Document::SetTransform(cv::Mat& transform)
{
    m_Transform = transform;
}

Mat Document::GetImage(ViewOptions& view)
{
    // Resulting image in RGB.
    Mat result;

    // Init resulting matrix.
    switch (view.Image)
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

    if (view.Fill)
    {
        if (view.Shape == NDTR_CONTOUR)
        {
            drawContours(result, m_Contuors, -1, view.FillColor, -1);
        }
        else
        {
            for (auto e : m_Ellipses)
            {
                ellipse(result, e, view.FillColor, -1);
            };
        }
    }

    if (view.Edge)
    {
        if (view.Shape == NDTR_CONTOUR)
        {
            drawContours(result, m_Contuors, -1, view.EdgeColor, 1);
        }
        else
        {
            for (auto e : m_Ellipses)
            {
                ellipse(result, e, view.EdgeColor, 1);
            };
        }
    }

    if (view.Center)
    {
        for (auto e : m_Ellipses)
        {
            circle(result, e.center, 1, view.CenterColor, -1);
        };
    }

    if (!m_Transform.empty())
    {
        warpPerspective(result, result, m_Transform, result.size());
    }

    return result;
}

void Document::ProcessImage()
{
    // Convert image to grayscale.
    cvtColor(m_Images[NDTR_ORIGINAL], m_Images[NDTR_GRAYSCALE], CV_BGR2GRAY);

    if (m_Options.GaussianBlur)
    {
        Size ksize;
        ksize.width = 5;
        ksize.height= 5;
        GaussianBlur(m_Images[NDTR_GRAYSCALE], m_Images[NDTR_GRAYSCALE], ksize, 0, 0);
    }

    int thresholdType;
    if (m_Options.WoB)
    {
        thresholdType = CV_THRESH_BINARY;
    }
    else
    {
        thresholdType = CV_THRESH_BINARY_INV;
    }

    if (m_Options.AutomaticOtsuThreshold)
    {
        thresholdType |= CV_THRESH_OTSU;
    }

    // Calculate black-white image.
    m_Options.OtsuThreshold = (int)
            threshold(m_Images[NDTR_GRAYSCALE],
            m_Images[NDTR_BLACK_WHITE],
            m_Options.OtsuThreshold,
            255,
            thresholdType);

    // Get contuors.
    findContours(m_Images[NDTR_BLACK_WHITE],
                 m_Contuors,
                 CV_RETR_LIST,
                 CV_CHAIN_APPROX_SIMPLE);

    // Calculate ellipses.
    m_Ellipses.resize(m_Contuors.size());
    m_TraceFilter.resize(m_Contuors.size());
    m_Traces.resize(m_Contuors.size());

    cv::Mat labels = cv::Mat::zeros(m_Images[NDTR_GRAYSCALE].size(), CV_8UC1);

    for (int i = 0; i < static_cast<int>(m_Contuors.size()); i++)
    {
        if (m_Contuors[i].size() < 5)
        {
            continue;
        }
        m_Ellipses[i] = fitEllipse(Mat(m_Contuors[i]));

        // TODO: make this work.
        m_TraceFilter[i] = m_Ellipses[i].size.width >= m_Options.MinTraceDiameter &&
                           m_Ellipses[i].size.width <= m_Options.MaxTraceDiameter &&
                           m_Ellipses[i].size.height >= m_Options.MinTraceDiameter &&
                           m_Ellipses[i].size.height <= m_Options.MaxTraceDiameter;

        m_Traces[i].x = static_cast<int>(m_Ellipses[i].center.x);
        m_Traces[i].y = static_cast<int>(m_Ellipses[i].center.y);
        m_Traces[i].angle = m_Ellipses[i].angle;
        m_Traces[i].diameter1 = static_cast<int>(m_Ellipses[i].size.width);
        m_Traces[i].diameter2 = static_cast<int>(m_Ellipses[i].size.height);

        drawContours(labels, m_Contuors, i, cv::Scalar(i), CV_FILLED);
        cv::Rect roi = cv::boundingRect(m_Contuors[i]);
        cv::Scalar mean = cv::mean(m_Images[NDTR_GRAYSCALE](roi), labels(roi) == i);

        m_Traces[i].intensity = static_cast<int>(mean[0]);
    }
}

void UpdateDist(Mat& dist, int k, float value)
{
    if (value < dist.at<float>(k-1))
    {
        int index = k-1;
        for (int i=k-2; i>=0; i--)
        {
            if (value < dist.at<float>(i))
            {
                index = i;
                dist.at<float>(i+1) = dist.at<float>(i);
            }
            else
            {
                break;
            }
        }
        dist.at<float>(index) = value;
    }
}

Mat GetDist(vector<Trace> traces)
{
    const int k = 5;

    // For every trace distances from nearest k traces from the set.
    Mat nearest5(traces.size(), k, DataType<float>::type);
    nearest5 = std::numeric_limits<float>::infinity();

    for (size_t t=0; t<traces.size(); t++)
    {
        Trace& tt = traces[t];
        Mat &tn = nearest5.adjustROI(t, traces.size() - (traces.size() - t), 0, k-1);

        for (size_t c=t+1; c<traces.size(); c++)
        {
            Trace& ct = traces[c];
            Mat &cn = nearest5.adjustROI(c, traces.size() - (traces.size() - c), 0, k-1);

            // NOTE: We'll skip calculating sqrt, it would be just overhead.
            float dist = static_cast<float>(pow(tt.x-ct.x, 2) + pow(tt.y-ct.y, 2));

            UpdateDist(tn, k, dist);
            UpdateDist(cn, k, dist);
        }
    }

    return nearest5;
}

bool MatchComparer (DMatch* i, DMatch* j)
{
    return (i->distance < j->distance);
}

Mat Document::CalcTransform(Document* prev, Document* curr)
{
    vector<Trace>& prevTraces = prev->GetTraces();
    vector<Trace>& currTraces = curr->GetTraces();

    Mat prevN5 = GetDist(prevTraces);
    Mat currN5 = GetDist(currTraces);

    FlannBasedMatcher matcher;
    std::vector< DMatch > matches;
    matcher.match( prevN5, currN5, matches );

    std::vector< DMatch* > refMatches;

    for (DMatch& m : matches)
    {
        refMatches.push_back(&m);
    }

    // using function as comp
    std::sort (refMatches.begin(), refMatches.end(), MatchComparer);

    std::vector<Point2f> pPrev;
    std::vector<Point2f> pCurr;

    for(size_t i = 0; i < matches.size() && i < 10; i++)
    {
        Point2f pkp;
        Point2f ckp;

        DMatch* m = refMatches[i];

        pkp.x = static_cast<float>(prevTraces[m->trainIdx].x);
        pkp.y = static_cast<float>(prevTraces[m->trainIdx].y);

        ckp.x = static_cast<float>(currTraces[m->queryIdx].x);
        ckp.y = static_cast<float>(currTraces[m->queryIdx].y);

        pPrev.push_back(pkp);
        pCurr.push_back(ckp);
    }

    Mat transform;
    if (pCurr.size() >= 3 && pPrev.size() >= 3)
    {
        transform = findHomography(pCurr, pPrev, CV_RANSAC);

        // Should I check is it affine homography?

        //h_{31}=h_{32}=0, \; h_{33}=1.
        //double h31 = m_Transform.at<double>(2,0);
        //double h32 = m_Transform.at<double>(2,1);
        //double h33 = m_Transform.at<double>(2,2);
    }

    return transform;
}

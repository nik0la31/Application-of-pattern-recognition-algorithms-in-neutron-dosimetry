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

    if (m_RatioOptions.PixelsPerUnit == 0.0f)
    {
        int min = m_Width < m_Height ? m_Width : m_Height;
        m_RatioOptions.PixelsPerUnit = min / 2.0f;
    }
}

void Document::Process(ProcessingOptions& options, bool keepManuelEdits)
{
    m_Options = options;

    if (!keepManuelEdits)
    {
        m_ManualEdits.clear();
    }

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

    if (stats.TracesCount == 0)
    {
        return stats;
    }

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

void Document::SetOptions(ProcessingOptions& options)
{
    m_Options = options;
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

int MarkLocalMaximums(Mat& dist_8u, Mat& localMaxumums)
{
    int numberOfLocalMaximums = 0;

    double minDist, maxDist;
    minMaxLoc(dist_8u, &minDist, &maxDist);
    int maxDistInt = (int) maxDist;

    for (int d = maxDistInt; d >= 1; d--)
    {
        cv::Mat distBinary = ((dist_8u ==  d) /  d);

        // Get connected components.
        Mat labels;
        int labelCount = connectedComponents(distBinary, labels);

        // First label is background.
        if (labelCount > 1)
        {
            // Check every component whether it is local maximum.
            for (int label = 1; label < labelCount; ++label)
            {
                // Collect all values from neighbour of every pixel in CC.
                vector<uint8_t> neighburValues;
                for (int r=1; r<labels.rows; r++)
                {
                    for (int c=1; c<labels.cols-1; c++)
                    {
                        int val = labels.at<int>(r,c);
                        if (val == label)
                        {
                            neighburValues.push_back(dist_8u.at<uint8_t>(r-1,c-1));
                            neighburValues.push_back(dist_8u.at<uint8_t>(r-1,c));
                            neighburValues.push_back(dist_8u.at<uint8_t>(r-1,c+1));
                            neighburValues.push_back(dist_8u.at<uint8_t>(r,  c-1));
                            neighburValues.push_back(dist_8u.at<uint8_t>(r,  c));
                            neighburValues.push_back(dist_8u.at<uint8_t>(r,  c+1));
                            neighburValues.push_back(dist_8u.at<uint8_t>(r+1,c-1));
                            neighburValues.push_back(dist_8u.at<uint8_t>(r+1,c));
                            neighburValues.push_back(dist_8u.at<uint8_t>(r+1,c+1));
                        }
                    }
                }

                double nMin, nMax;
                minMaxLoc(neighburValues, &nMin, &nMax);
                double nMaxInteger = (int) nMax;

                // Label value is equal to maximim value in neighbour it is local maximum.
                if (nMaxInteger == d)
                {
                    numberOfLocalMaximums++;

                    for (int r=1; r<labels.rows; r++)
                    {
                        for (int c=1; c<labels.cols-1; c++)
                        {
                            int val = labels.at<int>(r,c);
                            if (val == label)
                            {
                                localMaxumums.at<int>(r,c) = numberOfLocalMaximums;
                            }
                        }
                    }
                }
            }
        }
    }

    return numberOfLocalMaximums;
}

bool IsTouchingEdge(Mat& bw, Contour& contour)
{
    Mat m = bw.clone();
    m.setTo(0);
    vector<Contour> vec;
    vec.push_back(contour);
    drawContours(m, vec, 0, Scalar(255));

    Rect bb = boundingRect(contour);
    if (bb.x == 1)
    {
        for (int i=bb.y; i<bb.y+bb.height; i++)
        {
            if (bw.at<char>(i, 0) != 0)
            {
                Point2i pt(0, i);
                double dist = pointPolygonTest(contour, pt, true);

                if (dist >= -1)
                {
                    return true;
                }
            }
        }
    }

    if (bb.x + bb.width == bw.cols - 1)
    {
        for (int i=bb.y; i<bb.y+bb.height; i++)
        {
            if (bw.at<char>(i, bw.cols - 1) != 0)
            {
                Point2i pt(bw.cols - 1, i);
                double dist = pointPolygonTest(contour, pt, true);

                if (dist >= -1)
                {
                    return true;
                }
            }
        }
    }

    if (bb.y == 1)
    {
        for (int i=bb.x; i<bb.x+bb.width; i++)
        {
            if (bw.at<char>(0, i) != 0)
            {
                Point2i pt(i, 0);
                double dist = pointPolygonTest(contour, pt, true);

                if (dist >= -1)
                {
                    return true;
                }
            }
        }
    }

    if (bb.y + bb.height == bw.rows - 1)
    {
        for (int i=bb.x; i<bb.x+bb.width; i++)
        {
            char asd = bw.at<char>(bw.rows - 1, i);
            if (bw.at<char>(bw.rows - 1, i) != 0)
            {
                Point2i pt(i, bw.rows - 1);
                double dist = pointPolygonTest(contour, pt, true);

                if (dist >= -1)
                {
                    return true;
                }
            }
        }
    }

    return false;
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
    if (m_Options.WoB || m_Options.AutoDetectWob)
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
    Mat origBW = m_Images[NDTR_BLACK_WHITE].clone();

    if (m_Options.AutoDetectWob)
    {
        int TotalNumberOfPixels = origBW.rows * origBW.cols;
        int countWhite = TotalNumberOfPixels - countNonZero(origBW);
        int countBlack = TotalNumberOfPixels - countWhite;

          if (countBlack > countWhite)
          {
            m_Options.WoB = !m_Options.WoB;

            origBW = 255 - origBW;

            m_Images[NDTR_BLACK_WHITE] = origBW;
          }
    }

    m_InitialContuors.clear();
    findContours(m_Images[NDTR_BLACK_WHITE],
                 m_InitialContuors,
                 CV_RETR_EXTERNAL,
                 CV_CHAIN_APPROX_NONE);

    //cv::Mat asd = Mat::zeros(m_Height, m_Width, CV_8UC1);
    //drawContours(asd, initialContours, -1, Scalar(255));

    // Reseve memory.
    size_t reserveCount = static_cast<size_t>(m_InitialContuors.size() * 1.2);
    m_Contuors.clear();
    m_Contuors.reserve(reserveCount);
    m_InitialContourIndex.clear();
    m_InitialContourIndex.reserve(reserveCount);
    m_NoiseContuors.clear();
    m_NoiseContuors.reserve(reserveCount);
    m_InitialNoiseContourIndex.clear();
    m_InitialNoiseContourIndex.reserve(reserveCount);
    m_Ellipses.clear();
    m_Ellipses.reserve(reserveCount);
    m_Traces.clear();
    m_Traces.reserve(reserveCount);

    // Detect traces.
    cv::Mat temp = Mat::zeros(m_Height, m_Width, CV_8UC1);
    for (size_t i = 0; i < m_InitialContuors.size(); i++)
    {
        Contour& contour = m_InitialContuors[i];

        if (m_ManualEdits.size() > 0)
        {
            Rect bb = boundingRect(contour);

            bool manual = false;
            for (int eIndex=0; eIndex<m_ManualEdits.size(); eIndex++)
            {
                EditInfo& ei = m_ManualEdits[eIndex];

                Rect eBB = boundingRect(ei.EditContour);

                Rect iBB = bb & eBB;

                float area = iBB.area();
                if (area / bb.area() < 0.9 || area / eBB.area() < 0.9)
                {
                    continue;
                }

                if (true)
                {
                    ei.Index = i;
                    ApplyEdit(ei, false);
                    manual = true;
                    break;
                }
            }

            if (manual)
            {
                continue;
            }
        }

        // Check if there are enough point to analyze contour.
        const size_t minPointCount = 5;
        if (contour.size() < minPointCount)
        {
            m_NoiseContuors.push_back(contour);
            m_InitialNoiseContourIndex.push_back(i);

            continue;
        }

        // Skip contour if it is touching image edge.
        if (IsTouchingEdge(origBW, contour))
        {
            m_NoiseContuors.push_back(contour);
            m_InitialNoiseContourIndex.push_back(i);

            continue;
        }

        // Calculate ellipse.
        Ellipse ellipse = fitEllipse(Mat(contour));

        // ///////////////////////////////////////////////////////
        // Here goes the magic.

        // Calculate how far is cotuor from being convex.
        Contour hull;
        convexHull(contour, hull);

        double cArea = contourArea(contour);
        double hArea = contourArea(hull);
        double perc = cArea / (double) hArea;

        // Draw current contour.
        drawContours(temp, m_InitialContuors, i, Scalar(1), -1);

        // Get ROI - Region Of Interest.
        Rect roi = boundingRect(contour);

        // Get mask image.
        Mat mask = temp(roi);

        cv::Mat markers = cv::Mat::zeros(mask.size(), CV_32SC1);
        int numberOfMarkers = 0;

        // Check if current contour should be divided.
        if (perc < 0.95)
        {
            // Invalidate mask outer pixels/grame.
            // Trace center are certanly not there.
            // It keeps CCs cleaner.
            Mat framedMask = mask.clone();
            for (int r = 0; r < mask.rows; ++r) {
                framedMask.at<char>(r, 0) = 0;
                framedMask.at<char>(r, mask.cols-1) = 0;
            }
            for (int c = 0; c < mask.cols; ++c) {
                framedMask.at<char>(0, c) = 0;
                framedMask.at<char>(mask.rows-1, c) = 0;
            }

            // Calculate distance matrix.
            cv::Mat dist;
            cv::distanceTransform(framedMask, dist, CV_DIST_L2, 5);

            // Convers distance to integers.
            cv::Mat dist_8u;
            dist.convertTo(dist_8u, CV_8U);

            // Mark local maximums and use it as hint for traces.
            numberOfMarkers = MarkLocalMaximums(dist_8u, markers);
        }

        if (numberOfMarkers <= 1)
        {
            // Check if ellipse dimensions are in specified range.
            if (ellipse.size.width < m_Options.MinTraceDiameter ||
                ellipse.size.width > m_Options.MaxTraceDiameter ||
                ellipse.size.height < m_Options.MinTraceDiameter ||
                ellipse.size.height > m_Options.MaxTraceDiameter)
            {
                m_NoiseContuors.push_back(contour);
                m_InitialNoiseContourIndex.push_back(i);

                continue;
            }

            m_Contuors.push_back(contour);
            m_InitialContourIndex.push_back(i);

            // Create trace from original contour.
            m_Ellipses.emplace_back(ellipse);

            //drawContours(labels, m_Contuors, i, cv::Scalar(i), CV_FILLED);
            //cv::Rect roi = cv::boundingRect(m_Contuors[i]);
            cv::Scalar mean = cv::mean(m_Images[NDTR_GRAYSCALE](roi), mask == 1);

            m_Traces.emplace_back(
                static_cast<int>(ellipse.center.x),
                static_cast<int>(ellipse.center.y),
                ellipse.angle,
                static_cast<int>(ellipse.size.width),
                static_cast<int>(ellipse.size.height),
                static_cast<int>(mean[0]));

            // static_cast<int>(mean[0]));

            m_Traces[m_Traces.size() - 1].DebugColor = Scalar(0, 255, 0);
        }
        else
        {
            // Add new contours.

            // Markers are previously obtained local maximas.
            // Mark background.
            for (int r = 0; r < mask.rows; r++)
            {
                for (int c = 0; c < mask.cols; c++)
                {
                    int index = (int) mask.at<char>(r,c);
                    if (index == 0)
                    {
                        markers.at<int>(r,c) = 255;
                    }
                }
            }

            Mat result;
            cvtColor(mask, result, CV_GRAY2BGR);
            cv::watershed(result, markers);

//            // Generate random colors
//            std::vector<cv::Vec3b> colors;
//            for (int i = 0; i < numberOfMarkers; i++)
//            {
//                int b = cv::theRNG().uniform(0, 255);
//                int g = cv::theRNG().uniform(0, 255);
//                int r = cv::theRNG().uniform(0, 255);

//                colors.push_back(cv::Vec3b((uchar)b, (uchar)g, (uchar)r));
//            }

//            // Create the result image
//            cv::Mat dst = cv::Mat::zeros(markers.size(), CV_8UC3);

//            // Fill labeled objects with random colors
//            for (int i = 0; i < markers.rows; i++)
//            {
//                for (int j = 0; j < markers.cols; j++)
//                {
//                    int index = markers.at<int>(i,j);
//                    if (index > 0 && index <= numberOfMarkers)
//                        dst.at<cv::Vec3b>(i,j) = colors[index-1];
//                    else
//                        dst.at<cv::Vec3b>(i,j) = cv::Vec3b(0,0,0);
//                }
//            }

            for (size_t nc = 1; nc <= numberOfMarkers; nc++)
            {
                cv::Mat newMask = markers == nc;
                std::vector<Contour> newContours;
                findContours(newMask,
                             newContours,
                             CV_RETR_LIST,
                             CV_CHAIN_APPROX_SIMPLE);

                // There should be only one contour in this vector.
                for (size_t k = 0; k < newContours.size(); ++k)
                {
                    Contour& newContour = newContours[k];

                    // Check if there are enough point to analyze contour.
                    if (newContour.size() < minPointCount)
                    {
                        m_NoiseContuors.push_back(newContour);
                        m_InitialNoiseContourIndex.push_back(i);

                        continue;
                    }

                    // Check if ellipse dimensions are in specified range.
                    Ellipse newEllipse = fitEllipse(Mat(newContour));
                    if (newEllipse.size.width < m_Options.MinTraceDiameter ||
                        newEllipse.size.width > m_Options.MaxTraceDiameter ||
                        newEllipse.size.height < m_Options.MinTraceDiameter ||
                        newEllipse.size.height > m_Options.MaxTraceDiameter)
                    {
                        m_NoiseContuors.push_back(newContour);
                        m_InitialNoiseContourIndex.push_back(i);

                        continue;
                    }

                    for (Point2i& pt : newContour)
                    {
                        pt.x += roi.x;
                        pt.y += roi.y;
                    }

                    m_Contuors.push_back(newContour);
                    m_InitialContourIndex.push_back(i);

                    // Offset ellipse data.
                    newEllipse.center.x += roi.x;
                    newEllipse.center.y += roi.y;

                    m_Ellipses.emplace_back(newEllipse);

                    cv::Scalar mean = cv::mean(m_Images[NDTR_GRAYSCALE](roi), newMask == 1);

                    m_Traces.emplace_back(
                        static_cast<int>(newEllipse.center.x),
                        static_cast<int>(newEllipse.center.y),
                        newEllipse.angle,
                        static_cast<int>(newEllipse.size.width),
                        static_cast<int>(newEllipse.size.height),
                        static_cast<int>(mean[0]));

                    m_Traces[m_Traces.size() - 1].DebugColor = Scalar(0, 0, 255);
                }
            }
        }

        mask.setTo(0);

        // ///////////////////////////////////////////////////////
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
        Mat tn = nearest5(Rect(0, t, k, 1));

        for (size_t c=t+1; c<traces.size(); c++)
        {
            Trace& ct = traces[c];
            Mat cn = nearest5(Rect(0, c, k, 1));

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
    Mat transform;

    vector<Trace>& prevTraces = prev->GetTraces();
    vector<Trace>& currTraces = curr->GetTraces();

    if (prevTraces.size() < 3 || currTraces.size() < 3)
    {
        return transform;
    }

    Mat prevN5 = GetDist(prevTraces);
    Mat currN5 = GetDist(currTraces);

    BFMatcher matcher;
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

        pkp.x = static_cast<float>(prevTraces[m->queryIdx].x);
        pkp.y = static_cast<float>(prevTraces[m->queryIdx].y);

        ckp.x = static_cast<float>(currTraces[m->trainIdx].x);
        ckp.y = static_cast<float>(currTraces[m->trainIdx].y);

        pPrev.push_back(pkp);
        pCurr.push_back(ckp);
    }

    if (pCurr.size() >= 3 && pPrev.size() >= 3)
    {
        Mat temp = findHomography(pCurr, pPrev, CV_RANSAC);

        // Check is it affine homography.
        // h_{31}=h_{32}=0, \; h_{33}=1.
        double h31 = temp.at<double>(2,0);
        double h32 = temp.at<double>(2,1);
        double h33 = temp.at<double>(2,2);
        if (abs(h31) < 0.001 && abs(h32) < 0.001 && abs(h33 - 1) < 0.001)
        {
            transform = temp;
        }
    }

    return transform;
}

void Document::Clear()
{
    m_Contuors.clear();
    m_Ellipses.clear();
    m_Traces.clear();
}

TraceInfo Document::PosTest(int x, int y)
{
    TraceInfo ti;

    for (size_t i=0; i<m_Contuors.size(); i++)
    {
        Contour& contour = m_Contuors[i];

        if (pointPolygonTest(contour, Point2f(x,y), false) > 0)
        {
            ti.Type = 1;
            ti.Index = i;
            ti.InitIndex = m_InitialContourIndex[i];

            return ti;
        }
    }

    for (size_t i=0; i<m_NoiseContuors.size(); i++)
    {
        Contour& contour = m_NoiseContuors[i];

        if (pointPolygonTest(contour, Point2f(x,y), false) > 0)
        {
            ti.Type = 2;
            ti.Index = i;
            ti.InitIndex = m_InitialNoiseContourIndex[i];

            return ti;
        }
    }

    return ti;
}

void Document::MarkTrace(int noiseContourIndex, bool addNew)
{
    if (noiseContourIndex < m_NoiseContuors.size())
    {
        int index = m_InitialNoiseContourIndex[noiseContourIndex];
        Contour c = m_NoiseContuors[noiseContourIndex];

        m_InitialNoiseContourIndex.erase(
                    m_InitialNoiseContourIndex.begin() + noiseContourIndex);
        m_NoiseContuors.erase(
                    m_NoiseContuors.begin() + noiseContourIndex);

        m_InitialContourIndex.push_back(index);
        m_Contuors.push_back(c);

        // Calculate ellipse.
        Ellipse ellipse = fitEllipse(Mat(c));
        m_Ellipses.push_back(ellipse);

        // Get ROI - Region Of Interest.
        Rect roi = boundingRect(c);

        // Get mask image.
        cv::Mat temp = Mat::zeros(m_Height, m_Width, CV_8UC1);
        drawContours(temp, m_InitialContuors, index, Scalar(1), -1);
        Mat mask = temp(roi);

        cv::Scalar mean = cv::mean(m_Images[NDTR_GRAYSCALE](roi), mask == 1);

        m_Traces.emplace_back(
            static_cast<int>(ellipse.center.x),
            static_cast<int>(ellipse.center.y),
            ellipse.angle,
            static_cast<int>(ellipse.size.width),
            static_cast<int>(ellipse.size.height),
            static_cast<int>(mean[0]));

        if (addNew)
        {
            Rect roi2 = boundingRect(m_InitialContuors[index]);

            // Get ROI - Region Of Interest.
            Contour oc = c;
            for (Point2i& pt : oc)
            {
                pt.x -= roi2.x;
                pt.y -= roi2.y;
            }

            Point2f pt(ellipse.center.x - roi2.x, ellipse.center.y - roi2.y);

            bool foundEdit = false;
            int editsCount = m_ManualEdits.size();
            for (int i=0; i<editsCount;i++)
            {
                EditInfo& cmp = m_ManualEdits[i];
                if (cmp.Index == index)
                {
                    for (size_t n=0; n<cmp.NoiseContours.size(); n++)
                    {
                        Contour& contour = cmp.NoiseContours[n];

                        if (pointPolygonTest(contour, pt, false) >= 0)
                        {
                            Contour fc = cmp.NoiseContours[n];
                            cmp.NoiseContours.erase(cmp.NoiseContours.begin() + n);
                            cmp.TraceContours.push_back(fc);
                            break;
                        }
                    }

                    foundEdit = true;
                    break;
                }
            }

            if (!foundEdit)
            {
                EditInfo ei;
                ei.Index = index;
                ei.EditContour = m_InitialContuors[index];
                ei.TraceContours.push_back(oc);

                for (int qwe=0; qwe<m_InitialContourIndex.size()-1; qwe++)
                {
                    if (m_InitialContourIndex[qwe] == index)
                    {
                        Contour zxc = m_Contuors[qwe];

                        for (Point2i& pt : zxc)
                        {
                            pt.x -= roi2.x;
                            pt.y -= roi2.y;
                        }

                        ei.TraceContours.push_back(zxc);
                    }
                }

                for (int qwe=0; qwe<m_InitialNoiseContourIndex.size(); qwe++)
                {
                    if (m_InitialNoiseContourIndex[qwe] == index)
                    {
                        Contour zxc = m_Contuors[qwe];

                        for (Point2i& pt : zxc)
                        {
                            pt.x -= roi2.x;
                            pt.y -= roi2.y;
                        }

                        ei.NoiseContours.push_back(zxc);
                    }
                }

                m_ManualEdits.push_back(ei);
            }
        }
    }
}

void Document::MarkNoise(int traceContourIndex, bool addNew)
{
    if (traceContourIndex < m_Contuors.size())
    {
        int index = m_InitialContourIndex[traceContourIndex];
        Contour c = m_Contuors[traceContourIndex];

        m_InitialContourIndex.erase(m_InitialContourIndex.begin() + traceContourIndex);
        m_Contuors.erase(m_Contuors.begin() + traceContourIndex);
        m_Ellipses.erase(m_Ellipses.begin() + traceContourIndex);
        m_Traces.erase(m_Traces.begin() + traceContourIndex);

        m_InitialNoiseContourIndex.push_back(index);
        m_NoiseContuors.push_back(c);

        if (addNew)
        {
            // Calculate ellipse.
            Ellipse ellipse = fitEllipse(Mat(c));

            Rect roi2 = boundingRect(m_InitialContuors[index]);

            // Get ROI - Region Of Interest.
            Contour oc = c;
            for (Point2i& pt : oc)
            {
                pt.x -= roi2.x;
                pt.y -= roi2.y;
            }

            Point2f pt(ellipse.center.x - roi2.x, ellipse.center.y - roi2.y);

            bool foundEdit = false;
            int editsCount = m_ManualEdits.size();
            for (int i=0; i<editsCount;i++)
            {
                EditInfo& cmp = m_ManualEdits[i];
                if (cmp.Index == index)
                {
                    for (size_t n=0; n<cmp.TraceContours.size(); n++)
                    {
                        Contour& contour = cmp.TraceContours[n];

                        if (pointPolygonTest(contour, pt, false) >= 0)
                        {
                            Contour fc = cmp.TraceContours[n];
                            cmp.TraceContours.erase(cmp.TraceContours.begin() + n);
                            cmp.NoiseContours.push_back(fc);
                            break;
                        }
                    }

                    foundEdit = true;
                    break;
                }
            }

            if (!foundEdit)
            {
                EditInfo ei;
                ei.Index = index;
                ei.EditContour = m_InitialContuors[index];
                ei.NoiseContours.push_back(oc);

                for (int qwe=0; qwe<m_InitialContourIndex.size(); qwe++)
                {
                    if (m_InitialContourIndex[qwe] == index)
                    {
                        Contour zxc = m_Contuors[qwe];

                        for (Point2i& pt : zxc)
                        {
                            pt.x -= roi2.x;
                            pt.y -= roi2.y;
                        }

                        ei.TraceContours.push_back(zxc);
                    }
                }

                for (int qwe=0; qwe<m_InitialNoiseContourIndex.size()-1; qwe++)
                {
                    if (m_InitialNoiseContourIndex[qwe] == index)
                    {
                        Contour zxc = m_Contuors[qwe];

                        for (Point2i& pt : zxc)
                        {
                            pt.x -= roi2.x;
                            pt.y -= roi2.y;
                        }

                        ei.NoiseContours.push_back(zxc);
                    }
                }

                m_ManualEdits.push_back(ei);
            }
        }
    }
}

EditInfo Document::GetEditInfo(int contourIndex)
{
    EditInfo ei;

    ei.Index = contourIndex;
    ei.EditContour = m_InitialContuors[contourIndex];

    // Get ROI - Region Of Interest.
    Rect roi = boundingRect(ei.EditContour);

    // Get mask image.
    cv::Mat mask = Mat::zeros(m_Height, m_Width, CV_8UC1);
    drawContours(mask, m_InitialContuors, contourIndex, Scalar(1), -1);
    Mat temp;
    m_Images[NDTR_GRAYSCALE].copyTo(temp, mask);

    Mat imgOut = temp(roi).clone();

    ei.Grayscale = imgOut;

    return ei;
}

void Document::ApplyEdit(EditInfo& ei, bool addNew)
{
    int numberOfMarkers = ei.TraceContours.size() + ei.NoiseContours.size();

    if (numberOfMarkers == 0)
    {
        return;
    }

    if (addNew)
    {
        int editsCount = m_ManualEdits.size();
        for (int i=0; i<editsCount;i++)
        {
            EditInfo cmp = m_ManualEdits[i];
            if (cmp.Index == ei.Index)
            {
                m_ManualEdits.erase(m_ManualEdits.begin() + i);

                i--;
                editsCount--;
            }
        }

        m_ManualEdits.push_back(ei);

        int contourCount = m_InitialContourIndex.size();
        for (size_t i=0; i<contourCount; i++)
        {
            if (m_InitialContourIndex[i] == ei.Index)
            {
                m_Contuors.erase(m_Contuors.begin() + i);
                m_InitialContourIndex.erase(m_InitialContourIndex.begin() + i);
                m_Traces.erase(m_Traces.begin() + i);
                m_Ellipses.erase(m_Ellipses.begin() + i);

                contourCount--;
                i--;
            }
        }

        contourCount = m_InitialNoiseContourIndex.size();
        for (size_t i=0; i<contourCount; i++)
        {
            if (m_InitialNoiseContourIndex[i] == ei.Index)
            {
                m_NoiseContuors.erase(m_NoiseContuors.begin() + i);
                m_InitialNoiseContourIndex.erase(m_InitialNoiseContourIndex.begin() + i);

                contourCount--;
                i--;
            }
        }
    }

    cv::Mat temp = Mat::zeros(m_Height, m_Width, CV_8UC1);
    drawContours(temp, m_InitialContuors, ei.Index, Scalar(1), -1);

    // Get ROI - Region Of Interest.
    Rect roi = boundingRect(m_InitialContuors[ei.Index]);

    // Get mask image.
    Mat mask = temp(roi);

    cv::Mat markers = cv::Mat::zeros(mask.size(), CV_32SC1);

    // Mark traces
    for (size_t i=0; i<ei.NoiseContours.size(); i++)
    {
        drawContours(markers, ei.NoiseContours, i, Scalar(i+1), -1);
    }

    for (size_t i=0; i<ei.TraceContours.size(); i++)
    {
        drawContours(markers, ei.TraceContours, i, Scalar(ei.NoiseContours.size() + i + 1), -1);
    }

    // Mark background.
    for (int r = 0; r < mask.rows; r++)
    {
        for (int c = 0; c < mask.cols; c++)
        {
            int index = (int) mask.at<char>(r,c);
            if (index == 0)
            {
                markers.at<int>(r,c) = 255;
            }
        }
    }

    Mat result;
    cvtColor(mask, result, CV_GRAY2BGR);
    cv::watershed(result, markers);

    for (size_t nc = 1; nc <= numberOfMarkers; nc++)
    {
        cv::Mat newMask = markers == nc;

        std::vector<Contour> newContours;
        findContours(newMask,
                     newContours,
                     CV_RETR_LIST,
                     CV_CHAIN_APPROX_SIMPLE);

        // There should be only one contour in this vector.
        for (size_t k = 0; k < newContours.size(); ++k)
        {
            Contour& newContour = newContours[k];

            // test if it is noise or trace

            Ellipse newEllipse = fitEllipse(Mat(newContour));

            for (Point2i& pt : newContour)
            {
                pt.x += roi.x;
                pt.y += roi.y;
            }

            bool noise = false;
            Point2f centerPt(newEllipse.center.x, newEllipse.center.y);
            for (int i=0; i<ei.NoiseContours.size(); i++)
            {
                Contour& c = ei.NoiseContours[i];

                if (pointPolygonTest(c, centerPt, false) > 0)
                {
                    m_NoiseContuors.push_back(newContour);
                    m_InitialNoiseContourIndex.push_back(ei.Index);
                    noise = true;
                    break;
                }
            }

            if (noise)
            {
                continue;
            }

            // Offset ellipse data.
            newEllipse.center.x += roi.x;
            newEllipse.center.y += roi.y;

            m_Contuors.push_back(newContour);
            m_InitialContourIndex.push_back(ei.Index);


            m_Ellipses.emplace_back(newEllipse);

            cv::Scalar mean = cv::mean(m_Images[NDTR_GRAYSCALE](roi), newMask == 1);

            m_Traces.emplace_back(
                static_cast<int>(newEllipse.center.x),
                static_cast<int>(newEllipse.center.y),
                newEllipse.angle,
                static_cast<int>(newEllipse.size.width),
                static_cast<int>(newEllipse.size.height),
                static_cast<int>(mean[0]));

            m_Traces[m_Traces.size() - 1].DebugColor = Scalar(0, 0, 255);
        }
    }
}

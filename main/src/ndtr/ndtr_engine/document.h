#ifndef DOCUMENT_H
#define DOCUMENT_H

#include <memory>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <string>
#include <vector>

#include "viewoptions.h"
#include "processingoptions.h"
#include "stats.h"
#include "trace.h"
#include "ratiooptions.h"

typedef std::vector<cv::Point> Contour;
typedef cv::RotatedRect Ellipse;

// Forward declatation.
class Project;

struct TraceInfo
{
    // 0 - None
    // 1 - Trace
    // 2 - Noise
    int Type;
    int Index;
    int InitIndex;

    TraceInfo() : Type(0), Index(-1)
    {}
};

struct EditInfo
{
    int Index;
    cv::Mat Grayscale;
    Contour EditContour;

    std::vector<Contour> TraceContours;
    std::vector<Contour> NoiseContours;
};

class Document
{
public:
    Document()
        : m_Images(3)
    {

    }

    void Init(Project*, std::string name, std::string path);

    void Process(ProcessingOptions& options);

    Project* GetProject();

    std::string GetName();

    std::string GetExt();

    const Stats GetStats();

    ProcessingOptions GetOptions();

    void SetOptions(ProcessingOptions& options);

    std::vector<Trace>& GetTraces();

    cv::Mat GatTransform();

    void SetTransform(cv::Mat& transform);

    cv::Mat GetImage(ViewOptions& view);

    static cv::Mat CalcTransform(Document* prev, Document* curr);

    void Clear();

    RatioOptions GetRatioOptions()
    {
        return m_RatioOptions;
    }

    void SetRatioOptions(float pixelsPerUnit, float xCenterOffset, float yCenterOffset)
    {
        int min = m_Width < m_Height ? m_Width : m_Height;
        int max = m_Width < m_Height ? m_Width : m_Height;

        if (pixelsPerUnit > min / 20.0f && pixelsPerUnit < 2 * max)
        {
            m_RatioOptions.PixelsPerUnit = pixelsPerUnit;
        }

        m_RatioOptions.XCenterOffset = xCenterOffset;
        m_RatioOptions.YCenterOffset = yCenterOffset;
    }

    void SetBaseUnitRatio(int baseUnitRatio)
    {
        m_RatioOptions.BaseRatio = baseUnitRatio;
    }

    int GetWidth() { return m_Width; }
    int GetHeigth() { return m_Height; }

    TraceInfo PosTest(int x, int y);

    void MarkTrace(int noiseContourIndex, bool addNew = true);

    void MarkNoise(int traceContourIndex, bool addNew = true);

    EditInfo GetEditInfo(int contourIndex);

    void ApplyEdit(EditInfo& ei, bool addNew = true);

private:
    // Project - document parent.
    Project *m_Project;

    // Image name.
    std::string m_Name;

    // Processing options.
    ProcessingOptions m_Options;

    // Image path;
    std::string m_Path;

    // Image dimensions.
    unsigned int m_Width;
    unsigned int m_Height;

    // Images
    std::vector<cv::Mat> m_Images;

    // Traces.
    std::vector<Contour> m_InitialContuors;
    std::vector<Contour> m_Contuors;
    std::vector<Contour> m_NoiseContuors;
    std::vector<int> m_InitialContourIndex;
    std::vector<int> m_InitialNoiseContourIndex;
    std::vector<Ellipse> m_Ellipses;
    std::vector<Trace> m_Traces;

    // Transform matrix
    cv::Mat m_Transform;

    RatioOptions m_RatioOptions;

    std::vector<EditInfo> m_ManualEdits;

    void ProcessImage();
};

#endif // DOCUMENT_H

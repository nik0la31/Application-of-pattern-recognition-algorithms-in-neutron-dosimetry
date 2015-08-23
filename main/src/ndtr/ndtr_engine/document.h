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

typedef std::vector<cv::Point> Contuor;

// Forward declatation.
class Project;

class Document
{
public:
    Document()
        : m_Images(3)
    {

    }

    void Init(Project*, std::string& name, std::string& path);

    void Process(ProcessingOptions& options);

    Project* GetProject();

    std::string GetName();

    std::string GetExt();

    const Stats GetStats();

    ProcessingOptions GetOptions();

    std::vector<Trace>& GetTraces();

    cv::Mat GatTransform();

    void SetTransform(cv::Mat& transform);

    cv::Mat GetImage(ViewOptions& view);

    static cv::Mat CalcTransform(Document* prev, Document* curr);

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
    std::vector<Contuor> m_Contuors;
    std::vector<cv::RotatedRect> m_Ellipses;
    std::vector<bool> m_TraceFilter;
    std::vector<Trace> m_Traces;

    // Transform matrix
    cv::Mat m_Transform;

    void ProcessImage();
};

#endif // DOCUMENT_H

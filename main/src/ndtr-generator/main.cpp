#include <QCoreApplication>
#include "ndtrimagegenerator.h"

#include <memory>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>

#include "dirent.h"

using namespace cv;
using namespace std;

enum TraceColor
{
    White = 0,
    Black,
    GrayWhite
};

struct TraceDef
{
    Scalar PrimaryColor;
    Scalar SecondaryColor;
};

TraceDef GetTraceDef(TraceColor definition)
{
    TraceDef td;
    switch (definition)
    {
    case White:
        td.PrimaryColor = Scalar(250, 250, 250);
        td.SecondaryColor = Scalar(250, 250, 250);
        break;
    case Black:
        td.PrimaryColor = Scalar(10, 10, 10);
        td.SecondaryColor = Scalar(10, 10, 10);
        break;
    case GrayWhite:
        td.PrimaryColor = Scalar(100, 100, 100);
        td.SecondaryColor = Scalar(200, 200, 200);
        break;
    default:
        printf("Unkown trace def.");
        exit(0);
    }

    return td;
}

struct Parameters
{
    int width    = -1; // Required. Image width.
    int height   = -1; // Required. Image height.
    int min      = -1; // Required. Min trace radix.
    int max      = -1; // Required. Max trace radix.
    int count    = -1; // Required. Trace count (+/- 20%).
    int repeat   =  1; // Number samples to generate.
    int angle    =  0; // Max rotation angle.
    int sampling =  1; // Number of rotated images per sample.
    string bg;
    string out;
    TraceColor colors;
};

Parameters ParseArgs(int argc, char *argv[])
{
    Parameters p;

    // Skip program name.
    for (int i=1; i<argc; i++)
    {
        string s(argv[i]);

        if (s[0] != '/')
        {
            printf("Nepravilno zadat parametar: %s.", s.c_str());
            exit(0);
        }

        size_t pos = s.find(':');
        if (pos == string::npos)
        {
            printf("Nepravilno zadat parametar: %s.", s.c_str());
            exit(0);
        }

        string name = s.substr(1, pos - 1);
        string value = s.substr(pos + 1);

        if (name == "bg")
        {
            p.bg = value;
            continue;
        }
        else if (name == "out")
        {
            p.out = value;
            continue;
        }

        int numValue = atoi(value.c_str());

        if (name == "width")
        {
            p.width = numValue;
        }
        else if (name == "height")
        {
            p.height = numValue;
        }
        else if (name == "min")
        {
            p.min = numValue;
        }
        else if (name == "max")
        {
            p.max = numValue;
        }
        else if (name == "count")
        {
            p.count = numValue;
        }
        else if (name == "repeat")
        {
            p.repeat = numValue;
        }
        else if (name == "sampling")
        {
            p.sampling = numValue;
        }
        else if (name == "angle")
        {
            p.angle = numValue;
        }
        else if (name == "colors")
        {
            p.colors = static_cast<TraceColor>(numValue);
        }
    }

    if (p.width == -1 || p.height == -1 ||
            p.min ==-1 || p.max ==-1 ||
            p.count == -1 ||
            p.bg.empty() || p.out.empty())
    {
        printf("Nije zadat obavezan parametar.");
        exit(0);
    }

    return p;
}

vector<string> GetFiles(string& path)
{
    vector<string> files;
    DIR *dir;
    struct dirent *ent;
    if ((dir = opendir (path.c_str())) != NULL) {
      while ((ent = readdir (dir)) != NULL) {
        string name(ent->d_name);
        if (name.find(".png") != string::npos)
        {
            files.push_back(path + "\\" + name);
        }
      }
      closedir (dir);
    }

    return files;
}

vector<Mat> LoadImages(string& path, bool bgImages)
{
    vector<string> files = GetFiles(path);
    vector<Mat> images;
    images.reserve(files.size());

    int height = -1;
    for (auto& file : files)
    {
        Mat img = imread(file.c_str(), CV_LOAD_IMAGE_COLOR);
        if (height != -1)
        {
            if (bgImages && height != img.rows)
            {
                printf("Sve sabloni pozadine moraju imati istu h dimenziju\n");
                exit(0);
            }
        }
        else
        {
            height = img.rows;
        }

        images.push_back(img);
    }

    return images;
}

void FillBackground(Mat& image, vector<Mat>& bgPatterns)
{
    int width = image.cols;
    int height = image.rows;
    int bgHeight = bgPatterns[0].rows;
    int row = 0;
    while (row < height)
    {
        int column = 0;

        int bgW;
        int bgH;

        if (row + bgHeight > height)
            bgH = height - row;
        else
            bgH = bgHeight;

        while (column < width)
        {
            Mat bg = bgPatterns[rand() % bgPatterns.size()];

            if (column + bg.cols > width)
                bgW = width - column;
            else
                bgW = bg.cols;

            Mat &target = image(Rect(column, row, bgW, bgH));
            bg(Rect(0, 0, bgW, bgH)).copyTo(target);

            column += bgW;
        }

        row += bgH;
    }
}

string GetFileName(string base, int r, int s, string ext)
{
    char numR[10];
    itoa(r, numR, 10);
    char numS[10];
    itoa(s, numS, 10);
    string filename =
            base +
            string("\\sample_") +
            string(numR) +
            string("_") +
            string(numS) +
            ext;

    return filename;
}

Mat Generate(Mat& bg, vector<cv::RotatedRect>& traces, vector<cv::RotatedRect>& traces2, TraceDef td)
{
    Mat dst = bg;
    for (size_t t=0; t<traces.size(); t++)
    {
        ellipse(dst, traces[t], td.PrimaryColor, -1);
    };

    for (size_t t=0; t<traces2.size(); t++)
    {
        ellipse(dst, traces2[t], td.SecondaryColor, -1);
    };

    return dst;
}

void Grow(vector<cv::RotatedRect>& traces)
{
    for (size_t t=0; t<traces.size(); t++)
    {
        traces[t].size.width += static_cast<float>(rand() % 3);
        traces[t].size.height += static_cast<float>(rand() % 3);
    }
}

void AddNoise(Mat& image)
{
    GaussianBlur(image, image, Size(5, 15), 0, 0 );

    Mat noise(image.size(), image.type());
    randn(noise, 128, 10);

    Mat noiseToAdd = noise - Scalar::all(128);
    Mat noiseToSub = Scalar::all(128) - noise;

    // Apply noise.
    Mat noisyImage = image.clone();
    noisyImage += noiseToAdd;
    noisyImage -= noiseToSub;

    image = noisyImage;
}

int main(int argc, char *argv[])
{
    Parameters p = ParseArgs(argc, argv);

    // Repeat.
    for (int r=0; r<p.repeat; r++)
    {
        // Create image.
        Mat image = Mat::zeros(p.height, p.width, CV_8UC3);

        // Fill background.
        vector<Mat> bgPatterns = LoadImages(p.bg, true);
        FillBackground(image, bgPatterns);

        // Generate traces.
        vector<cv::RotatedRect> traces;
        vector<cv::RotatedRect> traces2;
        int traceCount = p.count * ((60 + (rand() % 80 )) / 100.0);
        for (int t=0; t<traceCount; t++)
        {
            Point2f center;
            center.x = static_cast<float>(rand() % p.width);
            center.y = static_cast<float>(rand() % p.height);

            Size2f size;
            size.width = static_cast<float>(p.min + rand() % (p.max - p.min));
            size.height = static_cast<float>(p.min + rand() % (p.max - p.min));

            float angle;
            angle = static_cast<float>(rand() % 360);

            traces.emplace_back(center, size, angle);

            Size2f size2 = size;
            if (rand() % 4 == 0)
            {
                size2.width *= 0.7;
                size2.height *= 0.7;
            }
            else
            {
                size2.width *= 0.5;
                size2.height *= 0.5;
            }

            traces2.emplace_back(center, size2, angle);
        }

        TraceDef td = GetTraceDef(p.colors);

        Mat dst = Generate(image, traces, traces2, td);
        AddNoise(dst);
        imwrite(GetFileName(p.out, r+1000, 100, ".png"), dst);

        ofstream out;
        out.open(GetFileName(p.out, r+1000, 100, ".csv"));

        out << "Broj tragova,\n";
        out << traceCount << ",\n";

        out << "x,y,ugao,dijametar1,dijametar2,\n";
        for (cv::RotatedRect& trace : traces)
        {
            out << static_cast<int>(trace.center.x) << ","
                << static_cast<int>(trace.center.y) << ","
                << trace.angle << ","
                << trace.size.width << ","
                << trace.size.height << ",\n";
        }

        out.close();

        // Angle.
        if (p.sampling > 1)
        {
            for (int s=1; s<p.sampling; s++)
            {
                Grow(traces);
                dst = Generate(image, traces, traces2, td);

                Point2f center(image.cols / 2.0f, image.rows/ 2.0f);
                double angle = (rand() % (2 * p.angle)) - p.angle;
                Mat transform = getRotationMatrix2D(center, angle, 1.0);

                Mat rotated;
                Vec3b bgColor = bgPatterns[0].at<Vec3b>(0,0);
                warpAffine(image, rotated, transform, image.size(), INTER_CUBIC, BORDER_CONSTANT, bgColor);

                AddNoise(rotated);
                imwrite(GetFileName(p.out, r+1000, s+100, ".png"), rotated);

                ofstream out;
                out.open(GetFileName(p.out, r+1000, s+100, ".angle.txt"));

                out << angle << ",\n";

                out.close();
            }
        }
    }

    printf("Completed.");

    return 0;
}

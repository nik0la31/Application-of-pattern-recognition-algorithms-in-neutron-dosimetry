#include "arrangetracedialog.h"
#include "ui_arrangetracedialog.h"

ArrangeTraceDialog::ArrangeTraceDialog(EditInfo ei, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ArrangeTraceDialog)
{
    ui->setupUi(this);

    m_EditInfo = ei;
    m_EditInfo.TraceContours.clear();
    m_EditInfo.NoiseContours.clear();

    // Get image.
    cv::Mat img;
    cvtColor(ei.Grayscale, img, CV_GRAY2RGB);

    // Prepare image.
    QImage qim = QImage(img.data, img.cols, img.rows, img.step, QImage::Format_RGB888);
    QPixmap pm(QPixmap::fromImage(qim));

    int larger = std::max(img.rows, img.cols);
    slaceFactor = 250.0f / larger;

    imageLabel = new MyLabel();

    imageLabel->setBackgroundRole(QPalette::Dark);
    imageLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    imageLabel->setScaledContents(true);
    imageLabel->setPixmap(pm);

    ui->scrollArea->setWidget(imageLabel);

    QRect rect;
    rect.setRect(
        10,
        10,
        img.cols * slaceFactor,
        img.rows * slaceFactor);

    imageLabel->setGeometry(rect);
    ui->scrollArea->setGeometry(rect);

    ui->buttonBox->setGeometry(
        ui->buttonBox->x(),
        imageLabel->y() + imageLabel->height() + 10,
        std::max(200, ui->buttonBox->width() - (250 - imageLabel->width())),
        ui->buttonBox->height());

    this->setGeometry(
        100,
        100,
        ui->buttonBox->width() + 30,
        ui->buttonBox->y() + 30);

    connect(imageLabel, SIGNAL(StartDraw(bool,int, int)), this, SLOT(on_imageStartDraw_triggered(bool, int, int)));
    connect(imageLabel, SIGNAL(Draw(int, int)), this, SLOT(on_imageDraw_triggered(int, int)));
    connect(imageLabel, SIGNAL(EndDraw(int, int)), this, SLOT(on_imageEndDraw_triggered(int, int)));
    connect(imageLabel, SIGNAL(Erase(int, int)), this, SLOT(on_imageErase_triggered(int, int)));
}

ArrangeTraceDialog::~ArrangeTraceDialog()
{
    delete ui;
}

void ArrangeTraceDialog::on_imageStartDraw_triggered(bool trace, int x, int y)
{
    m_IsCurrentTrace = trace;
    m_CurrentCountour.clear();

    on_imageDraw_triggered(x, y);
}

void ArrangeTraceDialog::on_imageDraw_triggered(int x, int y)
{
    cv::Point pt(x / slaceFactor, y / slaceFactor);
    m_CurrentCountour.push_back(pt);

    UpdateDrawing();
}

void ArrangeTraceDialog::on_imageEndDraw_triggered(int x, int y)
{
    on_imageDraw_triggered(x, y);

    if (m_IsCurrentTrace)
    {
        m_EditInfo.TraceContours.push_back(m_CurrentCountour);
    }
    else
    {
        m_EditInfo.NoiseContours.push_back(m_CurrentCountour);
    }

    m_CurrentCountour.clear();

    // Draw
    UpdateDrawing();
}

void ArrangeTraceDialog::on_imageErase_triggered(int x, int y)
{
    cv::Point pt(x / slaceFactor, y / slaceFactor);

    for (size_t i=0; i<m_EditInfo.TraceContours.size(); i++)
    {
        Contour& contour = m_EditInfo.TraceContours[i];

        if (pointPolygonTest(contour, pt, false) >= 0)
        {
            m_EditInfo.TraceContours.erase(m_EditInfo.TraceContours.begin() + i);
            UpdateDrawing();
            return;
        }
    }

    for (size_t i=0; i<m_EditInfo.NoiseContours.size(); i++)
    {
        Contour& contour = m_EditInfo.NoiseContours[i];

        if (pointPolygonTest(contour, pt, false) >= 0)
        {
            m_EditInfo.NoiseContours.erase(m_EditInfo.NoiseContours.begin() + i);
            UpdateDrawing();
            return;
        }
    }
}

void ArrangeTraceDialog::UpdateDrawing()
{
    // Draw
    cv::Mat temp = m_EditInfo.Grayscale;

    cv::Mat img;
    cvtColor(temp, img, CV_GRAY2RGB);

    cv::drawContours(img, m_EditInfo.TraceContours, -1, cv::Scalar(0,0,255), 1);
    cv::drawContours(img, m_EditInfo.NoiseContours, -1, cv::Scalar(255,0,0), 1);

    std::vector<Contour> current;
    current.push_back(m_CurrentCountour);
    if (m_IsCurrentTrace)
    {
        cv::drawContours(img, current, -1, cv::Scalar(0,255,0), 1);
    }
    else
    {
        cv::drawContours(img, current, -1, cv::Scalar(255,255,0), 1);
    }

    QImage qim = QImage(img.data, img.cols, img.rows, img.step, QImage::Format_RGB888);
    QPixmap pm(QPixmap::fromImage(qim));
    imageLabel->setPixmap(pm);
}

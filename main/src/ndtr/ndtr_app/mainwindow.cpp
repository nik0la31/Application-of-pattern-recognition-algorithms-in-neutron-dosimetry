#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <math.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <stdio.h>
#include <string>
#include <QCheckBox>
#include <QColorDialog>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QImage>
#include <QPixmap>
#include <QString>
#include <Qt>
#include "utils.h"
#include "workspace.h"

#include <QMessageBox>
#include <QBuffer>
#include <QMimeData>
#include <QClipboard>

#include <QPainter>

#include <QToolTip>

#include <arrangetracedialog.h>

#include <stdio.h>  /* defines FILENAME_MAX */

#ifdef WINDOWS
    #include <direct.h>
    #define GetCurrentDir _getcwd
#else
    #include <unistd.h>
    #define GetCurrentDir getcwd
#endif

using namespace std;
using namespace cv;
using namespace Qt;

QSize MainWindow::s_ZoomOffset = QSize(2, 2);

float x_offset = 0.0;
float y_offset = 0.0;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_RatioColor(0,0,0)
{
    ui->setupUi(this);

    ui->treeView->setModel(Workspace::Instance.GetProjectsModel());
    ui->treeView->setContextMenuPolicy(Qt::CustomContextMenu);

    ui->treeView->setSelectionMode(QAbstractItemView::NoSelection);

    // Add image action.
    addImageAct = new QAction(tr("Dodaj sliku"), this);
    connect(addImageAct, SIGNAL(triggered()), this, SLOT(on_actionAdd_Image_triggered()));

    saveProjctAct = new QAction(tr("Sačuvaj projekat"), this);
    connect(saveProjctAct, SIGNAL(triggered()), this, SLOT(on_actionSave_Project_triggered()));

    closeProjectAct = new QAction(tr("Zatvori projekat"), this);
    connect(closeProjectAct, SIGNAL(triggered()), this, SLOT(on_actionClose_Project_triggered()));

    removeImageAct = new QAction(tr("Ukloni sliku"), this);
    connect(removeImageAct, SIGNAL(triggered()), this, SLOT(on_actionRemove_Image_triggered()));

    newProjectAct = new QAction(tr("Novi projekat"), this);
    connect(newProjectAct, SIGNAL(triggered()), this, SLOT(on_actionNew_Project_triggered()));

    openProjectAct = new QAction(tr("Otvori projekat"), this);
    connect(openProjectAct, SIGNAL(triggered()), this, SLOT(on_actionOpen_Project_triggered()));

    saveAllAct = new QAction(tr("Sačuvaj sve"), this);
    connect(saveAllAct, SIGNAL(triggered()), this, SLOT(on_actionSave_All_triggered()));

    closeAllAct = new QAction(tr("Zatvori sve"), this);
    connect(closeAllAct, SIGNAL(triggered()), this, SLOT(on_actionClose_All_triggered()));

    //switchToAct = new QAction(tr("Switch to ..."), this);
    //connect(switchToAct, SIGNAL(triggered()), this, SLOT(on_actionSwitch_To_triggered()));

    markTraceAct = new QAction(tr("Označi trag"), this);
    connect(markTraceAct, SIGNAL(triggered()), this, SLOT(on_actionMarkTrace_triggered()));

    markNoiseAct = new QAction(tr("Označi šum"), this);
    connect(markNoiseAct, SIGNAL(triggered()), this, SLOT(on_actionMarkNoise_triggered()));

    arrangeAct = new QAction(tr("Uredi"), this);
    connect(arrangeAct, SIGNAL(triggered()), this, SLOT(on_actionArrange_triggered()));

    infoAct = new QAction(tr("Info"), this);
    connect(infoAct, SIGNAL(triggered()), this, SLOT(on_actionInfo_triggered()));

    // Image viewer
    imageLabel = new QLabel();
    imageLabel->setBackgroundRole(QPalette::Dark);
    imageLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    imageLabel->setScaledContents(true);

    scrollArea = new MyQScrollArea();
    scrollArea->setBackgroundRole(QPalette::Dark);
    scrollArea->setWidget(imageLabel);
    setCentralWidget(scrollArea);

    connect(scrollArea, SIGNAL(ZoomedIn()), this, SLOT(on_imageScroll_In_triggered()));
    connect(scrollArea, SIGNAL(ZoomedOut()), this, SLOT(on_imageScroll_Out_triggered()));
    connect(scrollArea, SIGNAL(Pan(int, int)), this, SLOT(on_imagePan_triggered(int, int)));
    connect(scrollArea, SIGNAL(MouseDown(int,int)), this, SLOT(on_imageMouseDown_triggered(int, int)));

    m_ImageTypeGroup = new QActionGroup(this);
    m_View.Image = NDTR_ORIGINAL;
    ui->actionOriginal->setChecked(true);
    ui->actionOriginal->setActionGroup(m_ImageTypeGroup);
    ui->actionGrayscale->setActionGroup(m_ImageTypeGroup);
    ui->actionBlackWhite->setActionGroup(m_ImageTypeGroup);
    ui->actionBlack_Background->setActionGroup(m_ImageTypeGroup);
    ui->actionWhite_Background->setActionGroup(m_ImageTypeGroup);

    m_ShapeTypeGroup = new QActionGroup(this);
    m_View.Shape = NDTR_ELLIPSE;
    ui->actionEllipse->setChecked(true);
    ui->actionEllipse->setActionGroup(m_ShapeTypeGroup);
    ui->actionContour->setActionGroup(m_ShapeTypeGroup);

    ui->actionFill->setChecked(true);
    ui->actionBorder->setChecked(true);
    ui->actionCenter->setChecked(true);

    // Display default image processing options.
    DisplayImageProcesingOptions();

    UpdateActionAvailability();

    ui->treeView->setEditTriggers(QAbstractItemView::NoEditTriggers);

    m_scaleRatioMode = false;
    m_autoProcess = false;

    m_BaseRatio = 0;

    ui->cmbUnit->addItem("- Izbrati -");
    ui->cmbUnit->addItem("µm (mikrometar)");
    ui->cmbUnit->addItem("nm (nanometar)");
    ui->cmbUnit->addItem("pm (pikometar)");
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionNew_Project_triggered()
{
    QString fullFilePath = QFileDialog::getSaveFileName(this, tr("New Project"), "", tr("Files (*.ndtr)"));

    if(fullFilePath.length() > 0)
    {
        try
        {
            if (!fullFilePath.endsWith(".ndtr"))
            {
                fullFilePath.append(QString(".ndtr"));
            }

            Workspace::Instance.AddProject(fullFilePath);

            RefreshImage();
            DisplayImageProcesingOptions();
            UpdateActionAvailability();
        }
        catch (const std::exception& ex)
        {
            QMessageBox msg;
            QString msgText(ex.what());
            msg.setText(msgText);
            msg.exec();
        }
    }
}

void MainWindow::on_actionOpen_Project_triggered()
{
    QString fullFilePath = QFileDialog::getOpenFileName(this, tr("Open Project"), "", tr("Files (*.ndtr)"));

    if(fullFilePath.length() > 0)
    {
        try
        {
            Workspace::Instance.AddProject(fullFilePath, true);

            if (Workspace::Instance.IsImageAvalilable())
            {
                Workspace::Instance.Update(m_Options, m_autoProcess);
                m_Options = Workspace::Instance.GetCurrentDocument()->GetOptions();
                m_BaseRatio = Workspace::Instance.GetCurrentDocument()->GetRatioOptions().BaseRatio;
            }

            RefreshImage();
            DisplayImageProcesingOptions();
            UpdateActionAvailability();

            on_actionFit_to_Window_triggered();
        }
        catch (const std::exception& ex)
        {
            QMessageBox msg;
            QString msgText(ex.what());
            msg.setText(msgText);
            msg.exec();
        }
    }
}

void MainWindow::on_actionAdd_Image_triggered()
{
    QString fullFilePath = QFileDialog::getOpenFileName(this, tr("Add Image"), "", tr("Files (*.bmp *.png *.jpg *.jpeg)"));

    if(fullFilePath.length() > 0)
    {
        try
        {
            auto index = ui->treeView->currentIndex();
            auto item = (ProjectItem*) Workspace::Instance.GetProjectsModel()->itemFromIndex(index);

            Workspace::Instance.AddDocument(item->GetName(), fullFilePath, m_Options);

            ui->treeView->expand(index);

            Workspace::Instance.Update(m_Options, m_autoProcess);

            RefreshImage();
            DisplayImageProcesingOptions();
            UpdateActionAvailability();

            on_actionFit_to_Window_triggered();
        }
        catch (const std::exception& ex)
        {
            QMessageBox msg;
            QString msgText(ex.what());
            msg.setText(msgText);
            msg.exec();
        }
    }
}

void MainWindow::on_treeView_customContextMenuRequested(const QPoint &pt)
{
    QPoint globalPt = ui->treeView->mapToGlobal(pt);
    QModelIndex index = ui->treeView->indexAt(pt);

    QMenu menu;

    if (index.isValid())
    {
        on_treeView_clicked(index);

        ProjectItem* item = (ProjectItem*) Workspace::Instance.GetProjectsModel()->itemFromIndex(index);

        //switchToAct->setEnabled(!item->IsSelected());
        //menu.addAction(switchToAct);

        if(!item->IsDocument())
        {
            menu.addAction(addImageAct);
            menu.addSeparator();
            saveProjctAct->setEnabled(!Workspace::Instance.IsCurrentProjectPersistent());
            menu.addAction(saveProjctAct);
            menu.addAction(closeProjectAct);
            menu.addSeparator();
        }
        else
        {
            menu.addAction(removeImageAct);
            menu.addSeparator();
        }
    }

    menu.addAction(newProjectAct);
    menu.addAction(openProjectAct);
    menu.addSeparator();
    saveAllAct->setEnabled(!Workspace::Instance.AreAllProjectsPersistent());
    menu.addAction(saveAllAct);
    closeAllAct->setEnabled(Workspace::Instance.GetCurrentProject());
    menu.addAction(closeAllAct);

    menu.exec(globalPt);
}

void MainWindow::on_actionSwitch_To_triggered()
{
    auto index = ui->treeView->currentIndex();
    on_treeView_clicked(index);
}

void MainWindow::on_treeView_clicked(const QModelIndex &index)
{
    auto item = (ProjectItem*) Workspace::Instance.GetProjectsModel()->itemFromIndex(index);

    std::string name = item->GetName();

    if (item->IsDocument())
    {
        ProjectItem* parent = (ProjectItem*) item->parent();

        if (Workspace::Instance.GetCurrentProject()->GetName() == parent->GetName() &&
                Workspace::Instance.GetCurrentDocument()->GetName() == item->GetName())
            return;

        Workspace::Instance.SetCurrent(parent->GetName(), item->GetName());
    }
    else
    {
        if (Workspace::Instance.GetCurrentProject()->GetName() == name)
            return;

        Workspace::Instance.SetCurrent(item->GetName());
    }

    if (Workspace::Instance.IsImageAvalilable())
    {
        m_Options = Workspace::Instance.GetCurrentDocument()->GetOptions();
        m_BaseRatio = Workspace::Instance.GetCurrentDocument()->GetRatioOptions().BaseRatio;

        Workspace::Instance.Update(m_Options, m_autoProcess);
    }

    RefreshImage();
    DisplayImageProcesingOptions();
    UpdateActionAvailability();

    on_actionFit_to_Window_triggered();
}

void MainWindow::on_imageScroll_In_triggered()
{
    if (!Workspace::Instance.IsImageAvalilable())
    {
        return;
    }

    if (QApplication::keyboardModifiers() == Qt::ControlModifier)
    {
        Document* doc = Workspace::Instance.GetCurrentDocument();
        RatioOptions options = doc->GetRatioOptions();
        doc->SetRatioOptions(options.PixelsPerUnit * 1.1, options.XCenterOffset, options.YCenterOffset);
        RefreshImage();
    }
    else
    {
        // Zoom in 10%.
       on_actionZoom_In_triggered();
    }
}

void MainWindow::on_imageScroll_Out_triggered()
{
    if (!Workspace::Instance.IsImageAvalilable())
    {
        return;
    }

    if (QApplication::keyboardModifiers() == Qt::ControlModifier)
    {
        Document* doc = Workspace::Instance.GetCurrentDocument();
        RatioOptions options = doc->GetRatioOptions();
        doc->SetRatioOptions(options.PixelsPerUnit * 0.91, options.XCenterOffset, options.YCenterOffset);
        RefreshImage();
    }
    else
    {
        on_actionZoom_Out_triggered();
    }
}

void MainWindow::on_imagePan_triggered(int x, int y)
{
    if (!Workspace::Instance.IsImageAvalilable())
    {
        return;
    }

    if (QApplication::keyboardModifiers() == Qt::ControlModifier)
    {
        Document* doc = Workspace::Instance.GetCurrentDocument();
        RatioOptions options = doc->GetRatioOptions();

        doc->SetRatioOptions(options.PixelsPerUnit, options.XCenterOffset + x / scaleFactor, options.YCenterOffset + y / scaleFactor);
        RefreshImage();
    }
    else
    {
        scrollArea->horizontalScrollBar()->setValue(scrollArea->horizontalScrollBar()->value() - x);
        scrollArea->verticalScrollBar()->setValue(scrollArea->verticalScrollBar()->value() - y);
    }
}

void MainWindow::on_imageMouseDown_triggered(int x, int y)
{
    if (!Workspace::Instance.IsImageAvalilable())
    {
        return;
    }

    Document* doc = Workspace::Instance.GetCurrentDocument();

    int xAbs = scrollArea->horizontalScrollBar()->value() + x;
    int yAbs = scrollArea->verticalScrollBar()->value() + y;

    int xScaled = xAbs / scaleFactor;
    int yScaled = yAbs / scaleFactor;

    int xAreaScaled = scrollArea->width() / scaleFactor;
    int yAreaScaled = scrollArea->height() / scaleFactor;

    xScaled = xAreaScaled - doc->GetWidth() > 0 ? xScaled - (xAreaScaled - doc->GetWidth()) / 2 : xScaled;
    yScaled = yAreaScaled - doc->GetHeigth() > 0 ? yScaled - (yAreaScaled - doc->GetHeigth()) / 2 : yScaled;

    TraceInfo ti = doc->PosTest(xScaled, yScaled);
    m_TraceInfo = ti;

    if (ti.Type > 0)
    {
        QMenu menu;

        if (ti.Type == 1)
        {
            menu.addAction(infoAct);
            menu.addAction(markNoiseAct);
        }
        else
        {
            menu.addAction(markTraceAct);
        }

        menu.addAction(arrangeAct);

        QPoint pt(x, y);
        QPoint globalPt = scrollArea->mapToGlobal(pt);
        menu.exec(globalPt);
    }
}

void MainWindow::on_actionInfo_triggered()
{
    Document* doc = Workspace::Instance.GetCurrentDocument();
    Trace trace = doc->GetTraces()[m_TraceInfo.Index];

    QString traceInfo;
    traceInfo.append("Info:\n");

    traceInfo.append("Pozicija     ");
    traceInfo.append(QString::number(trace.x));
    traceInfo.append(", ");
    traceInfo.append(QString::number(trace.y));
    traceInfo.append("\n");

    traceInfo.append("Dijametar ");
    traceInfo.append(QString::number(trace.diameter1));
    traceInfo.append(", ");
    traceInfo.append(QString::number(trace.diameter2));
    traceInfo.append("\n");

    traceInfo.append("Ugao         ");
    traceInfo.append(QString::number(int(trace.angle)));
    traceInfo.append("\n");

    traceInfo.append("Intenzitet  ");
    traceInfo.append(QString::number(trace.intensity));

    QToolTip::showText(QCursor::pos(), traceInfo);
}

void MainWindow::on_actionMarkTrace_triggered()
{
    Document* doc = Workspace::Instance.GetCurrentDocument();

    doc->MarkTrace(m_TraceInfo.Index);

    RefreshImage();
}

void MainWindow::on_actionMarkNoise_triggered()
{
    Document* doc = Workspace::Instance.GetCurrentDocument();

    doc->MarkNoise(m_TraceInfo.Index);

    RefreshImage();
}

void MainWindow::on_actionArrange_triggered()
{
    Document* doc = Workspace::Instance.GetCurrentDocument();
    EditInfo ei = doc->GetEditInfo(m_TraceInfo.InitIndex);

    ArrangeTraceDialog dialog(ei, this);

    if(dialog.exec())
    {
        doc->ApplyEdit(dialog.m_EditInfo);
        RefreshImage();
    }
}

void MainWindow::on_actionZoom_In_triggered()
{
    if (!Workspace::Instance.IsImageAvalilable())
    {
        return;
    }

    // Zoom in 10%.
    ScaleImage(1.1);
}

void MainWindow::on_actionZoom_Out_triggered()
{
    if (!Workspace::Instance.IsImageAvalilable())
    {
        return;
    }

    // Zoom out 9%.
    ScaleImage(0.91);
}

void MainWindow::on_actionFit_to_Window_triggered()
{
    if (!Workspace::Instance.IsImageAvalilable())
    {
        return;
    }

    // Get best fitting scale factor and apply it.
    QSize windowsSize = this->centralWidget()->size() - s_ZoomOffset;
    QSize pixmapSize = imageLabel->pixmap()->size();

    double wRatio =  (windowsSize.width()) / (double) pixmapSize.width();
    double hRatio = (windowsSize.height()) / (double) pixmapSize.height();

    double requiredFactor = min(wRatio, hRatio);

    ScaleImage(requiredFactor / scaleFactor);
}

void MainWindow::on_actionNormal_Size_triggered()
{
    if (!Workspace::Instance.IsImageAvalilable())
    {
        return;
    }

    // Get scale factor back to 1.0.
    ScaleImage(1.0 / scaleFactor);
}

void MainWindow::ScaleImage(double factor)
{
    // Calculate new scale factor.
    scaleFactor *= factor;

    QSize windowsSize = this->centralWidget()->size() - s_ZoomOffset;
    QSize pixmapSize = imageLabel->pixmap()->size();

    int horizontalMargin = static_cast<int>(
                max(windowsSize.width() - pixmapSize.width() * scaleFactor, 0.0) / 2);

    int verticalMargin = static_cast<int>(
                max(windowsSize.height() - pixmapSize.height() * scaleFactor, 0.0) / 2);

    imageLabel->setContentsMargins(
                horizontalMargin,
                verticalMargin,
                horizontalMargin,
                verticalMargin);

    QSize pixmapScaledSize = scaleFactor * imageLabel->pixmap()->size();

    QSize imageLabelSize(
                pixmapScaledSize.width() + 2 * horizontalMargin,
                pixmapScaledSize.height() + 2 * verticalMargin);

    imageLabel->resize(imageLabelSize);

    AdjustScrollBar(scrollArea->horizontalScrollBar(), factor);
    AdjustScrollBar(scrollArea->verticalScrollBar(), factor);
}

void MainWindow::AdjustScrollBar(QScrollBar *scrollBar, double factor)
{
    scrollBar->setValue(static_cast<int>(
        factor * scrollBar->value() + ((factor - 1) * scrollBar->pageStep()/2)));
}

void MainWindow::RefreshImage()
{
    if (Workspace::Instance.IsImageAvalilable())
    {
        // Get image.
        cv::Mat img = Workspace::Instance.GetImage(m_View);

        // Prepare image.
        QImage qim = QImage(img.data, img.cols, img.rows, img.step, QImage::Format_RGB888);
        QPixmap pm(QPixmap::fromImage(qim));

        if (m_scaleRatioMode)
        {
            Document* doc = Workspace::Instance.GetCurrentDocument();
            RatioOptions options = doc->GetRatioOptions();

            float one = options.PixelsPerUnit;
            float x_center = img.cols / 2.0f + options.XCenterOffset;
            float y_center = img.rows / 2.0f + options.YCenterOffset;

            QPainter painter(&pm);
            QBrush brush1(m_RatioColor);
            QBrush brush2(m_RatioColor);

            QPen pen1(brush1, 1, Qt::SolidLine);
            QPen pen2(brush2, 1, Qt::DotLine);

            // left
            float i = 0.0f;
            while (true)
            {
                float x = x_center - i * one;

                if (x < 0.0f)
                {
                    break;
                }

                if ( int(10*i+5) % 10 != 0 )
                {
                    painter.setPen(pen2);
                }
                else
                {
                    painter.setPen(pen1);
                }


                QLineF line(x, 0, x, img.rows);
                painter.drawLine(line);

                i += 0.1f;
            }

            // right
            i = -0.1f;
            while (true)
            {
                i += 0.1f;

                float x = x_center + i * one;

                if (x > img.cols)
                {
                    break;
                }

                if ( int(10*i+5) % 10 != 0 )
                {
                    painter.setPen(pen2);
                }
                else
                {
                    painter.setPen(pen1);
                }

                QLine line(x, 0, x, img.rows);
                painter.drawLine(line);
            }

            // up
            i = 0.0f;
            while (true)
            {
                float y = y_center - i * one;

                if (y < 0.0f)
                {
                    break;
                }

                if ( int(10*i+5) % 10 != 0 )
                {
                    painter.setPen(pen2);
                }
                else
                {
                    painter.setPen(pen1);
                }

                QLine line(0, y, img.cols, y);
                painter.drawLine(line);

                i += 0.1f;
            }

            // down
            i = 0.0f;
            while (true)
            {
                float y = y_center + i * one;

                if (y > img.rows)
                {
                    break;
                }

                if ( int(10*i+5) % 10 != 0 )
                {
                    painter.setPen(pen2);
                }
                else
                {
                    painter.setPen(pen1);
                }

                QLine line(0, y, img.cols, y);
                painter.drawLine(line);

                i += 0.1f;
            }
        }

        // Display image.
        imageLabel->setPixmap(pm);

        // Get options.
        RatioOptions ratioOptions = Workspace::Instance.GetCurrentDocument()->GetRatioOptions();
        m_Options = Workspace::Instance.GetCurrentDocument()->GetOptions();
        m_BaseRatio = ratioOptions.BaseRatio;
        DisplayImageProcesingOptions();

        Stats stats = Workspace::Instance.GetStats();

        char val[100];

        string statsStr;
        statsStr.append("Broj tragova:<br/>");
        sprintf(val, "%d", stats.TracesCount);
        statsStr.append(val);

        if (m_BaseRatio > 0)
        {
            float surface = (img.cols * img.rows) / (ratioOptions.PixelsPerUnit * ratioOptions.PixelsPerUnit);
            float countPerSurface = stats.TracesCount / surface;

            statsStr.append("<br/><br/>Broj tragova po ");

            if (m_BaseRatio == 6)
            {
                statsStr.append("µm");
            }
            else if (m_BaseRatio == 9)
            {
                statsStr.append("nm");
            }
            else if (m_BaseRatio == 12)
            {
                statsStr.append("pm");
            }

            statsStr.append("<sup>2</sup>:<br/>");
            sprintf(val, "%.2f", countPerSurface);
            statsStr.append(val);
        }

        if (stats.TracesCount > 0)
        {
            statsStr.append("<br/><br/>Minimalni dijametar:<br/>");
            sprintf(val, "%d", stats.MinDiameter);
            statsStr.append(val);
            statsStr.append("<br/><br/>Maksimalni dijametar:<br/>");
            sprintf(val, "%d", stats.MaxDiameter);
            statsStr.append(val);
            statsStr.append("<br/><br/>Srednji dijametar:<br/>");
            sprintf(val, "%d", stats.AverageDiameter);
            statsStr.append(val);
            statsStr.append("<br/><br/>Minimalni intenzitet:<br/>");
            sprintf(val, "%d", stats.MinIntensity);
            statsStr.append(val);
            statsStr.append("<br/><br/>Maksimalni intenzitet:<br/>");
            sprintf(val, "%d", stats.MaxIntensity);
            statsStr.append(val);
            statsStr.append("<br/><br/>Srednji intenzitet:<br/>");
            sprintf(val, "%d", stats.AverageIntensity);
            statsStr.append(val);
        }

        ui->labelStats->setText(statsStr.c_str());
    }
    else
    {
        imageLabel->clear();
        ui->labelStats->clear();
    }
}

void MainWindow::on_actionOriginal_triggered(bool checked)
{
    if (checked)
    {
        m_View.Image = NDTR_ORIGINAL;
        RefreshImage();
    }
}

void MainWindow::on_actionGrayscale_triggered(bool checked)
{
    if (checked)
    {
        m_View.Image = NDTR_GRAYSCALE;
        RefreshImage();
    }
}

void MainWindow::on_actionBlackWhite_triggered(bool checked)
{
    if (checked)
    {
        m_View.Image = NDTR_BLACK_WHITE;
        RefreshImage();
    }
}

void MainWindow::on_actionBlack_Background_triggered(bool checked)
{
    if (checked)
    {
        m_View.Image = NDTR_BLACK_BACKGROUND;
        RefreshImage();
    }
}

void MainWindow::on_actionWhite_Background_triggered(bool checked)
{
    if (checked)
    {
        m_View.Image = NDTR_WHITE_BACKGROUND;
        RefreshImage();
    }
}

void MainWindow::on_actionFill_triggered(bool checked)
{
    m_View.Fill = checked;
    RefreshImage();
}

void MainWindow::on_actionBorder_triggered(bool checked)
{
    m_View.Edge = checked;
    RefreshImage();
}

void MainWindow::on_actionCenter_triggered(bool checked)
{
    m_View.Center = checked;
    RefreshImage();
}

void MainWindow::on_actionContour_triggered(bool checked)
{
    if (checked)
    {
        m_View.Shape = NDTR_CONTOUR;
        RefreshImage();
    }
}

void MainWindow::on_actionEllipse_triggered(bool checked)
{
    if (checked)
    {
        m_View.Shape = NDTR_ELLIPSE;
        RefreshImage();
    }
}

void MainWindow::on_actionFillColor_clicked()
{
    QColor color(m_View.FillColor[0], m_View.FillColor[1], m_View.FillColor[2]);
    color = QColorDialog::getColor(color, this);

    if (color != QColor::Invalid)
    {
        m_View.FillColor[0] = color.red();
        m_View.FillColor[1] = color.green();
        m_View.FillColor[2] = color.blue();

        RefreshImage();
    }
}

void MainWindow::on_actionEdgeColor_clicked()
{
    QColor color(m_View.EdgeColor[0], m_View.EdgeColor[1], m_View.EdgeColor[2]);
    color = QColorDialog::getColor(color, this);

    if (color != QColor::Invalid)
    {
        m_View.EdgeColor[0] = color.red();
        m_View.EdgeColor[1] = color.green();
        m_View.EdgeColor[2] = color.blue();

        RefreshImage();
    }
}

void MainWindow::on_actionCenterColor_clicked()
{
    QColor color(m_View.CenterColor[0], m_View.CenterColor[1], m_View.CenterColor[2]);
    color = QColorDialog::getColor(color, this);

    if (color != QColor::Invalid)
    {
        m_View.CenterColor[0] = color.red();
        m_View.CenterColor[1] = color.green();
        m_View.CenterColor[2] = color.blue();

        RefreshImage();
    }
}

void MainWindow::DisplayImageProcesingOptions()
{
    ui->checkBoxAutoThreshold->setChecked(m_Options.AutomaticOtsuThreshold);
    ui->sliderThreshold->setValue(m_Options.OtsuThreshold);
    ui->sliderThreshold->setToolTip(QString::number(ui->sliderThreshold->value()));
    ui->sliderThreshold->setStatusTip(QString::number(ui->sliderThreshold->value()));
    //ui->sliderThreshold->setEnabled(!m_Options.AutomaticOtsuThreshold);

    ui->buttonBlur->setChecked(m_Options.GaussianBlur);

    ui->btnAutoWob->setChecked(m_Options.AutoDetectWob);
    ui->buttonWoB->setChecked(m_Options.WoB);
    //ui->buttonWoB->setEnabled(!m_Options.AutoDetectWob);

    ui->spinMin->setValue((double) m_Options.MinTraceDiameter);
    ui->spinMax->setValue((double) m_Options.MaxTraceDiameter);

    if (m_Options.AutomaticOtsuThreshold)
    {
        ui->groupThreshold->setTitle("Prag binarizacije: AUTO");
    }
    else
    {
        ui->groupThreshold->setTitle("Prag binarizacije: " + QString::number(ui->sliderThreshold->value()));
    }

    int index = m_BaseRatio / 3;
    index = index > 0 ? index - 1 : 0;
    ui->cmbUnit->setCurrentIndex(index);

    // -----------------------------------
    // Document

    bool isDocumetAvailable = Workspace::Instance.IsImageAvalilable();

    ui->checkBoxAutoThreshold->setEnabled(isDocumetAvailable);
    ui->sliderThreshold->setEnabled(isDocumetAvailable && !m_Options.AutomaticOtsuThreshold);
    ui->buttonBlur->setEnabled(isDocumetAvailable);;
    ui->btnAutoWob->setEnabled(isDocumetAvailable);
    ui->buttonWoB->setEnabled(isDocumetAvailable && !m_Options.AutoDetectWob);
    ui->spinMin->setEnabled(isDocumetAvailable);
    ui->spinMax->setEnabled(isDocumetAvailable);
    ui->cmbUnit->setEnabled(isDocumetAvailable);

    ui->btnProcessImage->setEnabled(isDocumetAvailable && !ui->btnAutoProcess->isChecked());
}

void MainWindow::UpdateActionAvailability()
{
    // -----------------------------------
    // Project

    //ui->actionNew_Project->setEnabled(true);
    //ui->actionOpen_Project->setEnabled(true);
    //ui->actionOpen_File->setEnabled(true);
    //ui->actionExit->setEnabled(true);

    if (Workspace::Instance.GetCurrentProject())
    {
        ui->actionClose_Project->setEnabled(true);
        ui->actionClose_All->setEnabled(true);
        ui->actionSave_Project->setEnabled(Workspace::Instance.IsCurrentProjectPersistent());
        ui->actionSave_All->setEnabled(Workspace::Instance.AreAllProjectsPersistent());
    }
    else
    {
        ui->actionClose_Project->setEnabled(false);
        ui->actionClose_All->setEnabled(false);
        ui->actionSave_Project->setEnabled(false);
        ui->actionSave_All->setEnabled(false);
    }

    // -----------------------------------
    // Edit

    ui->actionUndo->setEnabled(false);
    ui->actionRedo->setEnabled(false);

    ui->actionCopy_Image->setEnabled(Workspace::Instance.IsImageAvalilable());
    ui->actionSave_Image->setEnabled(Workspace::Instance.IsImageAvalilable());
    ui->actionExport->setEnabled(Workspace::Instance.IsImageAvalilable());

    // -----------------------------------
    // View

    ui->actionZoom_In->setEnabled(Workspace::Instance.IsImageAvalilable());
    ui->actionZoom_Out->setEnabled(Workspace::Instance.IsImageAvalilable());
    ui->actionNormal_Size->setEnabled(Workspace::Instance.IsImageAvalilable());
    ui->actionFit_to_Window->setEnabled(Workspace::Instance.IsImageAvalilable());

    // -----------------------------------
    // About

    //ui->actionUser_Guide->setEnabled(true);
    //ui->actionAbout->setEnabled(true);
}

void MainWindow::on_checkBoxAutoThreshold_clicked(bool checked)
{
    m_Options.AutomaticOtsuThreshold = checked;

    if (Workspace::Instance.IsImageAvalilable())
    {
        Workspace::Instance.Update(m_Options, m_autoProcess);
        RefreshImage();
    }

    DisplayImageProcesingOptions();
}

void MainWindow::on_sliderThreshold_sliderReleased()
{
    m_Options.OtsuThreshold = ui->sliderThreshold->value();

    if (Workspace::Instance.IsImageAvalilable())
    {
        Workspace::Instance.Update(m_Options, m_autoProcess);
        RefreshImage();
        DisplayImageProcesingOptions();
    }
}

void MainWindow::on_buttonBlur_clicked(bool checked)
{
    m_Options.GaussianBlur = checked;

    if (Workspace::Instance.IsImageAvalilable())
    {
        Workspace::Instance.Update(m_Options, m_autoProcess);
        RefreshImage();
        DisplayImageProcesingOptions();
    }
}

void MainWindow::on_buttonWoB_clicked(bool checked)
{
    m_Options.WoB = checked;

    if (Workspace::Instance.IsImageAvalilable())
    {
        Workspace::Instance.Update(m_Options, m_autoProcess);
        RefreshImage();
        DisplayImageProcesingOptions();
    }
}

void MainWindow::on_spinMin_editingFinished()
{
    m_Options.MinTraceDiameter = (int) ui->spinMin->value();

    if (Workspace::Instance.IsImageAvalilable())
    {
        Workspace::Instance.Update(m_Options, m_autoProcess);
        RefreshImage();
        DisplayImageProcesingOptions();
    }
}

void MainWindow::on_spinMax_editingFinished()
{
    m_Options.MaxTraceDiameter = (int) ui->spinMax->value();

    if (Workspace::Instance.IsImageAvalilable())
    {
        Workspace::Instance.Update(m_Options, m_autoProcess);
        RefreshImage();
        DisplayImageProcesingOptions();
    }
}

void MainWindow::on_btnAutoWob_clicked(bool checked)
{
    m_Options.AutoDetectWob = checked;

    if (Workspace::Instance.IsImageAvalilable())
    {
        Workspace::Instance.Update(m_Options, m_autoProcess);
    }

    RefreshImage();
    DisplayImageProcesingOptions();
}

void MainWindow::on_sliderThreshold_sliderMoved(int position)
{
    ui->groupThreshold->setTitle("Threshold: " + QString::number(position));
}

void MainWindow::on_actionAbout_triggered()
{
    QMessageBox aboutBox;
    aboutBox.setWindowTitle(QString("O programu..."));
    aboutBox.setText(QString("Softver za detekciju tragova neutronske dozimetrije"));
    aboutBox.setInformativeText(
                QString("Softver je izrađen u okviru master rada sa temom "
                        "'Primena algoritama prepoznavanja oblika u neutranskoj dozimetriji' "
                        "na Institutu za Matematiku i Informatiku, Prirodno-matematickog fakulteta, Univerziteta u Kragujevcu.\n\n"
                        "Student:\tNikola Nikolić\n"
                        "Mentor:\tMilos Ivanović"));
    aboutBox.setIcon(QMessageBox::Information);
    aboutBox.setStandardButtons(QMessageBox::Close);
    aboutBox.setDefaultButton(QMessageBox::Close);
    aboutBox.exec();
}

void MainWindow::on_actionUser_Guide_triggered()
{
    if (system(NULL))
    {
        char cCurrentPath[FILENAME_MAX];

         if (GetCurrentDir(cCurrentPath, sizeof(cCurrentPath)))
         {
            string command;
            command.append("start");
            command.append(" ");
            command.append(cCurrentPath);
            command.append("\\");
            command.append("master-rad.pdf");

            system(command.c_str());
         }
    }
}

void MainWindow::on_actionProjects_triggered(bool checked)
{
    if (checked)
    {
        ui->dockProjects->show();
    }
    else
    {
        ui->dockProjects->close();
    }
}

void MainWindow::on_actionStatistics_triggered(bool checked)
{
    if (checked)
    {
        ui->dockStatistics->show();
    }
    else
    {
        ui->dockStatistics->close();
    }
}

void MainWindow::on_actionOptions_triggered(bool checked)
{
    if (checked)
    {
        ui->dockSettings->show();
    }
    else
    {
        ui->dockSettings->close();
    }
}

void MainWindow::on_dockProjects_visibilityChanged(bool visible)
{
    ui->actionProjects->setChecked(visible);
}

void MainWindow::on_dockSettings_visibilityChanged(bool visible)
{
    ui->actionOptions->setChecked(visible);
}

void MainWindow::on_dockStatistics_visibilityChanged(bool visible)
{
    ui->actionStatistics->setChecked(visible);
}

void MainWindow::on_actionCopy_Image_triggered()
{
    if (Workspace::Instance.IsImageAvalilable())
    {
        QClipboard *clipboard = QApplication::clipboard();
        QPixmap img = imageLabel->pixmap()->copy();
        clipboard->setPixmap(img);
    }
}

void MainWindow::on_actionSave_Image_triggered()
{
    if (Workspace::Instance.IsImageAvalilable())
    {
        QString fullFilePath = QFileDialog::getSaveFileName(this, tr("New Project"), "image.bmp", tr("Files (*.bmp *.png .jpg)"));

        if (fullFilePath.length() > 0)
        {
            imageLabel->pixmap()->save(fullFilePath);
        }
    }
}

void MainWindow::on_actionExit_triggered()
{
    bool cancel = false;

    if (!Workspace::Instance.AreAllProjectsPersistent())
    {
        QMessageBox msgBox;
        msgBox.setWindowTitle(QString("Pitanje"));
        msgBox.setText(QString("Želite li da sačuvate promene?"));

        msgBox.setIcon(QMessageBox::Question);
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Yes);
        int ret = msgBox.exec();

        if (ret == QMessageBox::Yes)
        {
            Workspace::Instance.SaveAllProjects();
        }
        else if (ret == QMessageBox::Cancel)
        {
            cancel = true;
        }
    }

    if(!cancel)
    {
        exit(0);
    }
}

void MainWindow::on_actionSave_All_triggered()
{
    Workspace::Instance.SaveAllProjects();
    UpdateActionAvailability();
}

void MainWindow::on_actionSave_Project_triggered()
{
    if (Workspace::Instance.GetCurrentProject() == nullptr)
        return;

    Workspace::Instance.SaveCurrentProject();

    UpdateActionAvailability();
}

void MainWindow::closeEvent(QCloseEvent * /*event*/)
{
    on_actionExit_triggered();
}

void MainWindow::on_actionClose_Project_triggered()
{
    if (Workspace::Instance.GetCurrentProject() == nullptr)
        return;

    bool cancel = false;

    if (!Workspace::Instance.IsCurrentProjectPersistent())
    {
        QMessageBox msgBox;
        msgBox.setWindowTitle(QString("Pitanje"));
        msgBox.setText(QString("Želite li da sačuvate promene?"));

        msgBox.setIcon(QMessageBox::Question);
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Yes);
        int ret = msgBox.exec();

        if (ret == QMessageBox::Yes)
        {
            Workspace::Instance.SaveCurrentProject();
        }
        else if (ret == QMessageBox::Cancel)
        {
            cancel = true;
        }
    }

    if (!cancel)
    {
        Workspace::Instance.CloseCurrentProject();

        QModelIndex index = ui->treeView->currentIndex();

        if (index.isValid())
        {
            ProjectItem* item = (ProjectItem*) Workspace::Instance.GetProjectsModel()->itemFromIndex(index);

            if (item->IsDocument())
            {
                ProjectItem* parent = (ProjectItem*) item->parent();
                Workspace::Instance.SetCurrent(parent->GetName(), item->GetName());
            }
            else
            {
                Workspace::Instance.SetCurrent(item->GetName());
            }
        }

        RefreshImage();
        DisplayImageProcesingOptions();
        UpdateActionAvailability();
    }
}

void MainWindow::on_actionClose_All_triggered()
{
    bool cancel = false;

    if (!Workspace::Instance.AreAllProjectsPersistent())
    {
        QMessageBox msgBox;
        msgBox.setWindowTitle(QString("Pitanje"));
        msgBox.setText(QString("Želite li da sačuvate promene?"));

        msgBox.setIcon(QMessageBox::Question);
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Yes);
        int ret = msgBox.exec();

        if (ret == QMessageBox::Yes)
        {
            Workspace::Instance.SaveAllProjects();
        }
        else if (ret == QMessageBox::Cancel)
        {
            cancel = true;
        }
    }

    if (!cancel)
    {
        Workspace::Instance.CloseAllProjects();
        RefreshImage();
        DisplayImageProcesingOptions();
        UpdateActionAvailability();
    }
}

void MainWindow::on_actionOpen_File_triggered()
{
    if (Workspace::Instance.GetCurrentProject() == nullptr)
    {
        on_actionNew_Project_triggered();
    }

    if (Workspace::Instance.GetCurrentProject() != nullptr)
    {
        on_actionAdd_Image_triggered();
    }
}

void MainWindow::on_actionRemove_Image_triggered()
{
    Workspace::Instance.RemoveCurrentDocument();

    QModelIndex index = ui->treeView->currentIndex();

    if (index.isValid())
    {
        ProjectItem* item = (ProjectItem*) Workspace::Instance.GetProjectsModel()->itemFromIndex(index);

        if (item->IsDocument())
        {
            ProjectItem* parent = (ProjectItem*) item->parent();
            Workspace::Instance.SetCurrent(parent->GetName(), item->GetName());
        }
        else
        {
            Workspace::Instance.SetCurrent(item->GetName());
        }
    }

    RefreshImage();
    DisplayImageProcesingOptions();
    UpdateActionAvailability();
}

void MainWindow::on_actionExport_triggered()
{
    if (Workspace::Instance.IsImageAvalilable())
    {
        QString fullFilePath = QFileDialog::getSaveFileName(this, tr("Izvoz tragova"), "izvoz.csv", tr("Files (*.csv)"));

        if(fullFilePath.length() > 0)
        {
            try
            {
                Workspace::Instance.ExportTraces(fullFilePath.toStdString());
            }
            catch (const std::exception& ex)
            {
                QMessageBox msg;
                QString msgText(ex.what());
                msg.setText(msgText);
                msg.exec();
            }
        }
    }
}

void MainWindow::on_btnProcessImage_clicked()
{
    if (Workspace::Instance.IsImageAvalilable())
    {
        Workspace::Instance.GetCurrentDocument()->Process(m_Options);
        RefreshImage();
        DisplayImageProcesingOptions();
    }
}

void MainWindow::on_btnAutoProcess_clicked(bool checked)
{
    m_autoProcess = checked;

    if (Workspace::Instance.IsImageAvalilable())
    {
         if (Workspace::Instance.GetCurrentDocument()->GetTraces().size() == 0)
         {
             Workspace::Instance.GetCurrentDocument()->Process(m_Options);
             RefreshImage();
             DisplayImageProcesingOptions();
         }
    }
}

void MainWindow::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Control)
    {
        m_scaleRatioMode = true;
        RefreshImage();
    }
}

void MainWindow::keyReleaseEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Control)
    {
        m_scaleRatioMode = false;
        RefreshImage();
    }
}

void MainWindow::on_cmbUnit_currentIndexChanged(int index)
{
    if (Workspace::Instance.IsImageAvalilable())
    {
        // 0 -          0
        // 1 - um 6     3
        // 2 - nm 9     6
        // 3 - pm 12    9

        int val = index * 3;
        val = val >= 3 ? val + 3 : 0;
        m_BaseRatio = val;
        Workspace::Instance.GetCurrentDocument()->SetBaseUnitRatio(val);
        RefreshImage();
    }
}

void MainWindow::on_actionRatioColor_clicked()
{
    QColor color = QColorDialog::getColor(m_RatioColor, this);

    if (color != QColor::Invalid)
    {
        m_RatioColor = color;
    }
}

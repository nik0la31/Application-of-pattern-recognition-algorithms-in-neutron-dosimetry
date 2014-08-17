#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <math.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <stdio.h>
#include <string>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QImage>
#include <QPixmap>
#include <QString>
#include "utils.h"
#include "workspace.h"

#include <QMessageBox>

using namespace std;

QSize MainWindow::s_ZoomOffset = QSize(2, 2);

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->treeView->setModel(Workspace::Instance.GetProjectsModel());
    ui->treeView->setContextMenuPolicy(Qt::CustomContextMenu);

    ui->treeView->setSelectionMode(QAbstractItemView::NoSelection);

    // Add image action.
    addImageAct = new QAction(tr("Add Image"), this);
    connect(addImageAct, SIGNAL(triggered()), this, SLOT(on_actionAdd_Image_triggered()));

    saveProjctAct = new QAction(tr("Save Project"), this);
    // TODO: connect

    closeProjectAct = new QAction(tr("Close Project"), this);
    // TODO: connect

    removeImageAct = new QAction(tr("Remove Image"), this);
    // TODO: connect

    newProjectAct = new QAction(tr("New Project"), this);
    connect(newProjectAct, SIGNAL(triggered()), this, SLOT(on_actionNew_Project_triggered()));

    openProjectAct = new QAction(tr("Open Project"), this);
    connect(openProjectAct, SIGNAL(triggered()), this, SLOT(on_actionOpen_Project_triggered()));

    saveAllAct = new QAction(tr("Save All"), this);
    // TODO: connect

    closeAllAct = new QAction(tr("Close All"), this);
    // TODO: connect

    switchToAct = new QAction(tr("Switch to ..."), this);
    connect(switchToAct, SIGNAL(triggered()), this, SLOT(on_actionSwitch_To_triggered()));

    // Image viewer
    imageLabel = new QLabel();
    imageLabel->setBackgroundRole(QPalette::Dark);
    imageLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    imageLabel->setScaledContents(true);

    scrollArea = new QScrollArea();
    scrollArea->setBackgroundRole(QPalette::Dark);
    scrollArea->setWidget(imageLabel);
    setCentralWidget(scrollArea);

    m_ImageTypeGroup = new QActionGroup(this);
    m_ImageType = NDTR_ORIGINAL;
    ui->actionOriginal->setChecked(true);
    ui->actionOriginal->setActionGroup(m_ImageTypeGroup);
    ui->actionGrayscale->setActionGroup(m_ImageTypeGroup);
    ui->actionBlackWhite->setActionGroup(m_ImageTypeGroup);
    ui->actionBlack_Background->setActionGroup(m_ImageTypeGroup);
    ui->actionWhite_Background->setActionGroup(m_ImageTypeGroup);

    m_ShapeTypeGroup = new QActionGroup(this);
    m_ShapeType = NDTR_ELLIPSE;
    ui->actionEllipse->setChecked(true);
    ui->actionEllipse->setActionGroup(m_ShapeTypeGroup);
    ui->actionContour->setActionGroup(m_ShapeTypeGroup);

    ui->actionFill->setChecked(true);
    ui->actionBorder->setChecked(true);
    ui->actionCenter->setChecked(true);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionExit_triggered()
{   
    exit(0);
}

void MainWindow::on_actionNew_Project_triggered()
{
    QString fullFilePath = QFileDialog::getSaveFileName(this, tr("New Project"), "", tr("Files (*.ndtr)"));

    if(fullFilePath.length() > 0)
    {
        try
        {
            Workspace::Instance.AddProject(fullFilePath);

            RefreshImage();
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
            Workspace::Instance.AddProject(fullFilePath);

            RefreshImage();

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

            Workspace::Instance.AddDocument(item->GetName(), fullFilePath);

            ui->treeView->expand(index);

            RefreshImage();
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
        ProjectItem* item = (ProjectItem*) Workspace::Instance.GetProjectsModel()->itemFromIndex(index);

        switchToAct->setEnabled(!item->IsSelected());
        menu.addAction(switchToAct);

        if(!item->IsDocument())
        {
            menu.addAction(addImageAct);
            menu.addSeparator();
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
    menu.addAction(saveAllAct);
    menu.addAction(closeAllAct);

    menu.exec(globalPt);
}

void MainWindow::on_actionSwitch_To_triggered()
{
    auto index = ui->treeView->currentIndex();
    on_treeView_doubleClicked(index);
}

void MainWindow::on_treeView_doubleClicked(const QModelIndex &index)
{
    auto item = (ProjectItem*) Workspace::Instance.GetProjectsModel()->itemFromIndex(index);

    if (item->IsDocument())
    {
        ProjectItem* parent = (ProjectItem*) item->parent();
        Workspace::Instance.SetCurrent(parent->GetName(), item->GetName());
    }
    else
    {
        Workspace::Instance.SetCurrent(item->GetName());
    }

    RefreshImage();
    on_actionFit_to_Window_triggered();
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
        cv::Mat img = Workspace::Instance.GetImage(m_ImageType, m_ShapeType, m_Fill, m_Border, m_Center);

        // Prepare image.
        QImage qim = QImage(img.data, img.cols, img.rows, QImage::Format_RGB888);
        QPixmap pm(QPixmap::fromImage(qim));

        // Display image.
        imageLabel->setPixmap(pm);
    }
    else
    {
        imageLabel->clear();
    }
}

void MainWindow::on_actionOriginal_triggered(bool checked)
{
    if (checked)
    {
        m_ImageType = NDTR_ORIGINAL;
        RefreshImage();
    }
}

void MainWindow::on_actionGrayscale_triggered(bool checked)
{
    if (checked)
    {
        m_ImageType = NDTR_GRAYSCALE;
        RefreshImage();
    }
}

void MainWindow::on_actionBlackWhite_triggered(bool checked)
{
    if (checked)
    {
        m_ImageType = NDTR_BLACK_WHITE;
        RefreshImage();
    }
}

void MainWindow::on_actionBlack_Background_triggered(bool checked)
{
    if (checked)
    {
        m_ImageType = NDTR_BLACK_BACKGROUND;
        RefreshImage();
    }
}

void MainWindow::on_actionWhite_Background_triggered(bool checked)
{
    if (checked)
    {
        m_ImageType = NDTR_WHITE_BACKGROUND;
        RefreshImage();
    }
}

void MainWindow::on_actionFill_triggered(bool checked)
{
    m_Fill = checked;
    RefreshImage();
}

void MainWindow::on_actionBorder_triggered(bool checked)
{
    m_Border = checked;
    RefreshImage();
}

void MainWindow::on_actionCenter_triggered(bool checked)
{
    m_Center = checked;
    RefreshImage();
}

void MainWindow::on_actionContour_triggered(bool checked)
{
    if (checked)
    {
        m_ShapeType = NDTR_CONTOUR;
        RefreshImage();
    }
}

void MainWindow::on_actionEllipse_triggered(bool checked)
{
    if (checked)
    {
        m_ShapeType = NDTR_ELLIPSE;
        RefreshImage();
    }
}

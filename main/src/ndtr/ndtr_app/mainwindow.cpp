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
    connect(saveProjctAct, SIGNAL(triggered()), this, SLOT(on_actionSave_Project_triggered()));

    closeProjectAct = new QAction(tr("Close Project"), this);
    connect(closeProjectAct, SIGNAL(triggered()), this, SLOT(on_actionClose_Project_triggered()));

    removeImageAct = new QAction(tr("Remove Image"), this);
    connect(removeImageAct, SIGNAL(triggered()), this, SLOT(on_actionRemove_Image_triggered()));

    newProjectAct = new QAction(tr("New Project"), this);
    connect(newProjectAct, SIGNAL(triggered()), this, SLOT(on_actionNew_Project_triggered()));

    openProjectAct = new QAction(tr("Open Project"), this);
    connect(openProjectAct, SIGNAL(triggered()), this, SLOT(on_actionOpen_Project_triggered()));

    saveAllAct = new QAction(tr("Save All"), this);
    connect(saveAllAct, SIGNAL(triggered()), this, SLOT(on_actionSave_All_triggered()));

    closeAllAct = new QAction(tr("Close All"), this);
    connect(closeAllAct, SIGNAL(triggered()), this, SLOT(on_actionClose_All_triggered()));

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

    ui->treeView->setEditTriggers(QAbstractItemView::DoubleClicked);
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
            Workspace::Instance.AddProject(fullFilePath, true);

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

            Workspace::Instance.AddDocument(item->GetName(), fullFilePath, m_Options);

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
        on_treeView_clicked(index);

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
        cv::Mat img = Workspace::Instance.GetImage(m_View);

        // Prepare image.
        QImage qim = QImage(img.data, img.cols, img.rows, QImage::Format_RGB888);
        QPixmap pm(QPixmap::fromImage(qim));

        // Display image.
        imageLabel->setPixmap(pm);

        // Get options.
        m_Options = Workspace::Instance.GetCurrentDocument()->GetOptions();
        DisplayImageProcesingOptions();

        Stats stats = Workspace::Instance.GetStats();

        char val[100];

        string statsStr;
        statsStr.append("Broj tragova:\n");
        statsStr.append(itoa(stats.TracesCount, val, 10));
        statsStr.append("\n\nMinimalni dijametar:\n");
        statsStr.append(itoa(stats.MinDiameter, val, 10));
        statsStr.append("\n\nMaksimalni dijametar:\n");
        statsStr.append(itoa(stats.MaxDiameter, val, 10));
        statsStr.append("\n\nSrednji dijametar:\n");
        statsStr.append(itoa(stats.AverageDiameter, val, 10));
        statsStr.append("\n\nMinimalni intenzitet:\n");
        statsStr.append(itoa(stats.MinIntensity, val, 10));
        statsStr.append("\n\nMaksimalni intenzitet:\n");
        statsStr.append(itoa(stats.MaxIntensity, val, 10));
        statsStr.append("\n\nSrednji intenzitet:\n");
        statsStr.append(itoa(stats.AverageIntensity, val, 10));

        ui->labelStats->setText(QString(statsStr.c_str()));
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
    ui->sliderThreshold->setEnabled(!m_Options.AutomaticOtsuThreshold);
    ui->buttonBlur->setChecked(m_Options.GaussianBlur);
    ui->buttonWoB->setChecked(m_Options.WoB);
    ui->spinMin->setValue((double) m_Options.MinTraceDiameter);
    ui->spinMax->setValue((double) m_Options.MaxTraceDiameter);

    if (m_Options.AutomaticOtsuThreshold)
    {
        ui->groupThreshold->setTitle("Threshold: AUTO");
    }
    else
    {
        ui->groupThreshold->setTitle("Threshold: " + QString::number(ui->sliderThreshold->value()));
    }

}

void MainWindow::on_checkBoxAutoThreshold_clicked(bool checked)
{
    m_Options.AutomaticOtsuThreshold = checked;

    if (Workspace::Instance.IsImageAvalilable())
    {
        Workspace::Instance.Update(m_Options);
        RefreshImage();
    }

    DisplayImageProcesingOptions();
}

void MainWindow::on_sliderThreshold_sliderReleased()
{
    m_Options.OtsuThreshold = ui->sliderThreshold->value();

    if (Workspace::Instance.IsImageAvalilable())
    {
        Workspace::Instance.Update(m_Options);
        RefreshImage();
        DisplayImageProcesingOptions();
    }
}

void MainWindow::on_buttonBlur_clicked(bool checked)
{
    m_Options.GaussianBlur = checked;

    if (Workspace::Instance.IsImageAvalilable())
    {
        Workspace::Instance.Update(m_Options);
        RefreshImage();
        DisplayImageProcesingOptions();
    }
}

void MainWindow::on_buttonWoB_clicked(bool checked)
{
    m_Options.WoB = checked;

    if (Workspace::Instance.IsImageAvalilable())
    {
        Workspace::Instance.Update(m_Options);
        RefreshImage();
        DisplayImageProcesingOptions();
    }
}

void MainWindow::on_spinMin_editingFinished()
{
    m_Options.MinTraceDiameter = (int) ui->spinMin->value();

    if (Workspace::Instance.IsImageAvalilable())
    {
        Workspace::Instance.Update(m_Options);
        RefreshImage();
        DisplayImageProcesingOptions();
    }
}

void MainWindow::on_spinMax_editingFinished()
{
    m_Options.MaxTraceDiameter = (int) ui->spinMax->value();

    if (Workspace::Instance.IsImageAvalilable())
    {
        Workspace::Instance.Update(m_Options);
        RefreshImage();
        DisplayImageProcesingOptions();
    }
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
}

void MainWindow::on_actionSave_Project_triggered()
{
    if (Workspace::Instance.GetCurrentProject() == nullptr)
        return;

    Workspace::Instance.SaveCurrentProject();
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

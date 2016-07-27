#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QActionGroup>
#include <QLabel>
#include <QMainWindow>
#include <QMenu>
#include <QScrollArea>
#include <QScrollBar>
#include <QtPrintSupport/QPrinter>
#include "workspace.h"

#include "viewoptions.h"
#include "processingoptions.h"

#include <QWheelEvent>

namespace Ui {
class MainWindow;
}

class MyQScrollArea : public QScrollArea
{
    Q_OBJECT

private:
    QPoint m_mousePos;

protected:
    void wheelEvent(QWheelEvent* event) override
    {
        if (event->delta() > 0)
        {
            emit ZoomedIn();
        }
        else
        {
            emit ZoomedOut();
        }

        event->accept();
    }

    void mousePressEvent(QMouseEvent *e) override
    {
        m_mousePos = e->pos();

        if (e->buttons() == Qt::RightButton)
        {
            emit MouseDown(m_mousePos.x(), m_mousePos.y());
        }
    }

    void mouseMoveEvent(QMouseEvent *e) override
    {
        if (e->buttons() == Qt::LeftButton)
        {
            QPoint diff = e->pos() - m_mousePos;
            m_mousePos = e->pos();

            emit Pan(diff.x(), diff.y());
            //emit MouseMove(m_mousePos.x(), m_mousePos.y());

            e->accept();
        }
    }

signals:

    void ZoomedIn();

    void ZoomedOut();

    void Pan(int x, int y);

    void MouseDown(int x, int y);
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_actionExit_triggered();

    void on_actionNew_Project_triggered();

    void on_treeView_customContextMenuRequested(const QPoint &pos);

    void on_actionOpen_Project_triggered();

    void on_actionAdd_Image_triggered();

    void on_actionSwitch_To_triggered();

    void on_imageScroll_In_triggered();
    void on_imageScroll_Out_triggered();
    void on_imagePan_triggered(int x, int y);
    void on_imageMouseDown_triggered(int x, int y);

    void on_actionZoom_In_triggered();

    void on_actionZoom_Out_triggered();

    void on_actionFit_to_Window_triggered();

    void on_actionNormal_Size_triggered();

    void on_actionOriginal_triggered(bool checked);

    void on_actionGrayscale_triggered(bool checked);

    void on_actionBlackWhite_triggered(bool checked);

    void on_actionBlack_Background_triggered(bool checked);

    void on_actionWhite_Background_triggered(bool checked);

    void on_actionFill_triggered(bool checked);

    void on_actionBorder_triggered(bool checked);

    void on_actionCenter_triggered(bool checked);

    void on_actionContour_triggered(bool checked);

    void on_actionEllipse_triggered(bool checked);

    void on_actionFillColor_clicked();

    void on_actionEdgeColor_clicked();

    void on_actionCenterColor_clicked();

    void on_checkBoxAutoThreshold_clicked(bool checked);

    void on_sliderThreshold_sliderReleased();

    void on_buttonBlur_clicked(bool checked);

    void on_buttonWoB_clicked(bool checked);

    void on_spinMin_editingFinished();

    void on_spinMax_editingFinished();

    void on_sliderThreshold_sliderMoved(int position);

    void on_actionAbout_triggered();

    void on_actionUser_Guide_triggered();

    void on_actionProjects_triggered(bool checked);

    void on_actionStatistics_triggered(bool checked);

    void on_actionOptions_triggered(bool checked);

    void on_dockProjects_visibilityChanged(bool visible);

    void on_dockSettings_visibilityChanged(bool visible);

    void on_dockStatistics_visibilityChanged(bool visible);

    void on_actionSave_Project_triggered();

    void on_actionCopy_Image_triggered();

    void on_actionSave_Image_triggered();

    void on_actionOpen_File_triggered();

    void on_actionClose_Project_triggered();

    void on_actionClose_All_triggered();

    void on_actionSave_All_triggered();

    void on_actionRemove_Image_triggered();

    void on_treeView_clicked(const QModelIndex &index);

    void on_actionExport_triggered();

    void on_btnProcessImage_clicked();

    void on_btnAutoProcess_clicked(bool checked);

    void on_btnAutoWob_clicked(bool checked);

    void on_cmbUnit_currentIndexChanged(int index);

    void on_actionRatioColor_clicked();

    void on_actionMarkTrace_triggered();

    void on_actionMarkNoise_triggered();

    void on_actionArrange_triggered();

    void on_actionInfo_triggered();

    void on_btnKeepManuals_clicked(bool checked);

private:
    Ui::MainWindow *ui;

    QLabel* imageLabel;
    MyQScrollArea* scrollArea;
    double scaleFactor = 1.0;

    QAction* addImageAct;
    QAction* saveProjctAct;
    QAction* closeProjectAct;
    QAction* removeImageAct;
    QAction* newProjectAct;
    QAction* openProjectAct;
    QAction* saveAllAct;
    QAction* closeAllAct;
    QAction* switchToAct;

    QAction* markTraceAct;
    QAction* markNoiseAct;
    QAction* arrangeAct;
    QAction* infoAct;

    Workspace m_Workspace;

    static QSize s_ZoomOffset;

    void ScaleImage(double factor);

    void AdjustScrollBar(QScrollBar *scrollBar, double factor);

    void RefreshImage();

    QActionGroup* m_ImageTypeGroup;
    QActionGroup* m_ShapeTypeGroup;

    ViewOptions m_View;

    // Curently displayed settings.
    ProcessingOptions m_Options;

    void DisplayImageProcesingOptions();

    void UpdateActionAvailability();

    // If true, re-process image on any image action (add, change params, etc).
    // Othervize, clear results.
    bool m_autoProcess;

    bool m_keepManualEdits;

    bool m_scaleRatioMode;

    std::string m_Unit;

    QColor m_RatioColor;

    TraceInfo m_TraceInfo;

protected:
    void closeEvent(QCloseEvent * /*event*/);

    void keyPressEvent(QKeyEvent* event);

    void keyReleaseEvent(QKeyEvent* event);
};

#endif // MAINWINDOW_H

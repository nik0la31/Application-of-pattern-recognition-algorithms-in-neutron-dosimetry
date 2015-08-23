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

namespace Ui {
class MainWindow;
}

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

private:
    Ui::MainWindow *ui;

    QLabel* imageLabel;
    QScrollArea* scrollArea;
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

protected:
    void closeEvent(QCloseEvent * /*event*/);
};

#endif // MAINWINDOW_H

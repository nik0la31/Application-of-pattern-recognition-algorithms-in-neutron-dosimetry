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

    void on_treeView_doubleClicked(const QModelIndex &index);

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

private:
    Ui::MainWindow *ui;

    QLabel* imageLabel;
    QScrollArea* scrollArea;
    double scaleFactor;

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
    ImageType m_ImageType;

    QActionGroup* m_ShapeTypeGroup;
    ShapeType m_ShapeType;

    bool m_Fill   = true;
    bool m_Border = true;
    bool m_Center = true;

};

#endif // MAINWINDOW_H

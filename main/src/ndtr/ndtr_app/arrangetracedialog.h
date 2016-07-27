#ifndef ARRANGETRACEDIALOG_H
#define ARRANGETRACEDIALOG_H

#include <QDialog>

#include <document.h>
#include <QLabel>
#include <QMouseEvent>

namespace Ui {
class ArrangeTraceDialog;
}

class MyLabel : public QLabel
{
    Q_OBJECT

private:
    bool m_IsDrawing = false;

protected:
    void mouseMoveEvent(QMouseEvent *e) override
    {
        if (m_IsDrawing)
        {
            QPoint pt = e->pos();

            emit Draw(pt.x(), pt.y());

            e->accept();
        }
    }

    void mousePressEvent(QMouseEvent *e) override
    {
        if (m_IsDrawing)
        {
            return;
        }

        if (e->buttons() == Qt::LeftButton || e->buttons() == Qt::RightButton)
        {
            QPoint pt = e->pos();
            m_IsDrawing = true;

            emit StartDraw(e->buttons() == Qt::LeftButton, pt.x(), pt.y());

            e->accept();
        }
        else if (e->buttons() == Qt::MiddleButton)
        {
            QPoint pt = e->pos();

            emit Erase(pt.x(), pt.y());
        }
    }

    void mouseReleaseEvent(QMouseEvent *e) override
    {
        if (m_IsDrawing)
        {
            m_IsDrawing = false;

            QPoint pt = e->pos();

            emit EndDraw(pt.x(), pt.y());

            e->accept();
        }
    }

signals:
    void StartDraw(bool trace, int x, int y);
    void Draw(int x, int y);
    void EndDraw(int x, int y);
    void Erase(int x, int y);
};

class ArrangeTraceDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ArrangeTraceDialog(EditInfo ei, QWidget *parent = 0);
    ~ArrangeTraceDialog();

    EditInfo m_EditInfo;

private:
    Ui::ArrangeTraceDialog *ui;

    float slaceFactor;

    MyLabel* imageLabel;

    Contour m_CurrentCountour;
    bool m_IsCurrentTrace;

    void UpdateDrawing();

private slots:
    void on_imageStartDraw_triggered(bool trace, int x, int y);
    void on_imageDraw_triggered(int x, int y);
    void on_imageEndDraw_triggered(int x, int y);
    void on_imageErase_triggered(int x, int y);
};

#endif // ARRANGETRACEDIALOG_H

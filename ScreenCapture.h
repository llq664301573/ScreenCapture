#pragma once

#include "AssignDialog.h"
#include <QtGui>
#include <QtWidgets>
#include <vector>
#include <queue>
using namespace std;

struct MyWindow
{
    HWND hwnd;
    QRect rect;
    QString name;
};

class ScreenCapture : public QWidget
{
    Q_OBJECT
public:
    explicit ScreenCapture(QWidget *parent = 0);
    ~ScreenCapture();

public slots:
    void capture();

protected:
    virtual void mousePressEvent(QMouseEvent *);
    virtual void mouseReleaseEvent(QMouseEvent *);
    virtual void mouseDoubleClickEvent(QMouseEvent *);
    virtual void mouseMoveEvent(QMouseEvent *);

    virtual void paintEvent(QPaintEvent *event);

    virtual void enumChildWindows(HWND parent, vector<MyWindow> &orderWindows) {}

protected:
    QPixmap desktopPixmap;

    vector<MyWindow> topWindows;
    vector<vector<MyWindow>*> childWindows;
    QRect captureRect;

    bool capturing;
    bool seleted;

    enum NonDiagonallyResizingPosition
    {
        Left, Top, Right, Bottm
    };
    bool nonDiagonallyResizing;
    NonDiagonallyResizingPosition resizingPosition;
    int resizingFixedEdge;

    bool diagonallyResizing;
    QPoint resizingFixedPoint;

    bool leftButtonPress;
    QPoint leftButtonPressPoint;

    bool captureRectPress;
    QPoint captureRectPressPoint;

private:
    void createTrayIcon();
};

#include "ScreenCapture.h"

ScreenCapture::ScreenCapture(QWidget *parent) : QWidget(parent)
{
    setMouseTracking(true);

    QShortcut *shortcut = new QShortcut(QKeySequence(Qt::Key_Escape), this);
    connect(shortcut, SIGNAL(activated()), this, SLOT(hide()));

    createTrayIcon();
}

ScreenCapture::~ScreenCapture()
{
    for (int i = 0; i < childWindows.size(); i++)
        delete childWindows[i];
}

void ScreenCapture::createTrayIcon()
{
    QSystemTrayIcon *trayIcon = new QSystemTrayIcon(this);
    trayIcon->setIcon(style()->standardIcon(QStyle::SP_TitleBarMenuButton));
    trayIcon->setToolTip(windowTitle());
    trayIcon->setVisible(true);

    QMenu *trayIconMenu = new QMenu(this);

    QAction *assignAction = new QAction(QString::fromLocal8Bit("设置"), this);
    connect(assignAction, SIGNAL(triggered()), new AssignDialog(this), SLOT(show()));
    trayIconMenu->addAction(assignAction);

    QAction *quitAction = new QAction(QString::fromLocal8Bit("退出"), this);
    connect(quitAction, SIGNAL(triggered(bool)), this, SLOT(setEnabled(bool)));
    trayIconMenu->addAction(quitAction);

    trayIcon->setContextMenu(trayIconMenu);
}

void ScreenCapture::capture()
{
    if(!isHidden())//说明上一次截屏操作未完成
        return ;

    seleted = capturing = false;
    leftButtonPress = captureRectPress = false;
    captureRect = QRect();
    setCursor(QCursor(Qt::ArrowCursor));

    const QDesktopWidget *desktopWidget = QApplication::desktop();
    desktopPixmap = QPixmap::grabWindow(desktopWidget->winId(), 0, 0, desktopWidget->width(), desktopWidget->height());

    topWindows.clear();
    enumChildWindows(NULL, topWindows);

    for (int i = 0; i < childWindows.size(); i++)
        delete childWindows[i];
    childWindows.clear();

    for (int i = 0; i < topWindows.size(); i++)
    {
        vector<MyWindow> *windows = new vector<MyWindow>();
        enumChildWindows(topWindows[i].hwnd, *windows);
        childWindows.push_back(windows);
    }

    showFullScreen();
}

void ScreenCapture::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {
        leftButtonPressPoint = event->pos();
        leftButtonPress = true;
        if(captureRect.contains(event->pos()))
        {
            captureRectPressPoint = event->pos();
            captureRectPress = true;
        }

        diagonallyResizing = cursor().shape() == Qt::SizeBDiagCursor || cursor().shape() == Qt::SizeFDiagCursor;
        if(diagonallyResizing)
        {
            if(event->pos().x() == captureRect.left() && event->pos().y() == captureRect.top())
                resizingFixedPoint = captureRect.bottomRight();
            else if(event->pos().x() == captureRect.right() && event->pos().y() == captureRect.bottom())
                resizingFixedPoint = captureRect.topLeft();
            else if(event->pos().x() == captureRect.right() && event->pos().y() == captureRect.top())
                resizingFixedPoint = captureRect.bottomLeft();
            else if(event->pos().x() == captureRect.left() && event->pos().y() == captureRect.bottom())
                resizingFixedPoint = captureRect.topRight();
        }


        nonDiagonallyResizing = cursor().shape() == Qt::SizeVerCursor || cursor().shape() == Qt::SizeHorCursor;
        if(nonDiagonallyResizing)
        {
            if(event->pos().x() == captureRect.left())
            {
                resizingPosition = Left;
                resizingFixedEdge = captureRect.right();
            }
            else if(event->pos().y() == captureRect.top())
            {
                resizingPosition = Top;
                resizingFixedEdge = captureRect.bottom();
            }
            else if(event->pos().x() == captureRect.right())
            {
                resizingPosition = Right;
                resizingFixedEdge = captureRect.left();
            }
            else if(event->pos().y() == captureRect.bottom())
            {
                resizingPosition = Bottm;
                resizingFixedEdge = captureRect.top();
            }
        }
    }

    if(event->button() == Qt::RightButton && (!seleted || !captureRect.contains(event->pos())))
    {
        hide();

        if(seleted)
            capture();
    }
}

void ScreenCapture::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {
        if(captureRectPressPoint == event->pos() || capturing)
        {
            seleted = true;
            setCursor(QCursor(Qt::SizeAllCursor));
        }

        leftButtonPress = captureRectPress = false;
    }
}

void ScreenCapture::mouseDoubleClickEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton && captureRect.contains(event->pos()))
    {
        QApplication::clipboard()->setPixmap(desktopPixmap.copy(captureRect));
        hide();
    }
}

void ScreenCapture::mouseMoveEvent(QMouseEvent *event)
{
    if(leftButtonPress && (!seleted || diagonallyResizing))
    {
        if(!diagonallyResizing)
            resizingFixedPoint = leftButtonPressPoint;

        QPoint topLeft;
        topLeft.setX(min(resizingFixedPoint.x(), event->pos().x()));
        topLeft.setY(min(resizingFixedPoint.y(), event->pos().y()));
        QPoint bottomRight;
        bottomRight.setX(max(resizingFixedPoint.x(), event->pos().x()));
        bottomRight.setY(max(resizingFixedPoint.y(), event->pos().y()));

        captureRect.setTopLeft(topLeft);
        captureRect.setBottomRight(bottomRight);

        capturing = true;
        update();
    }

    if(leftButtonPress && nonDiagonallyResizing)
    {
        switch (resizingPosition)
        {
        case ScreenCapture::Left:
            if(event->pos().x() > resizingFixedEdge)
            {
                captureRect.setLeft(resizingFixedEdge);
                captureRect.setRight(event->pos().x());
            }
            else
            {
                captureRect.setLeft(event->pos().x());
                captureRect.setRight(resizingFixedEdge);
            }
            break;
        case ScreenCapture::Top:
            if(event->pos().y() > resizingFixedEdge)
            {
                captureRect.setTop(resizingFixedEdge);
                captureRect.setBottom(event->pos().y());
            }
            else
            {
                captureRect.setTop(event->pos().y());
                captureRect.setBottom(resizingFixedEdge);
            }
            break;
        case ScreenCapture::Right:
            if(event->pos().x() < resizingFixedEdge)
            {
                captureRect.setRight(resizingFixedEdge);
                captureRect.setLeft(event->pos().x());
            }
            else
            {
                captureRect.setRight(event->pos().x());
                captureRect.setLeft(resizingFixedEdge);
            }
            break;
        case ScreenCapture::Bottm:
            if(event->pos().y() < resizingFixedEdge)
            {
                captureRect.setBottom(resizingFixedEdge);
                captureRect.setTop(event->pos().y());
            }
            else
            {
                captureRect.setBottom(event->pos().y());
                captureRect.setTop(resizingFixedEdge);
            }
            break;
        default:
            break;
        }

        update();
    }

    if(!seleted && !leftButtonPress)
    {
        for(int i = 0 ; i < topWindows.size() ; i++)
        {
            if(topWindows[i].rect.contains(event->pos()))
            {
                captureRect = topWindows[i].rect;
                for (int k = childWindows[i]->size() - 1; k >= 0; k--)
                {
                    if(childWindows[i]->at(k).rect.contains(event->pos()))
                    {
                        captureRect = childWindows[i]->at(k).rect;
                        break;
                    }
                }

                break;
            }
        }

        capturing = false;
        update();
    }

    if(seleted)
    {
        if(captureRect.contains(event->pos()))
        {
            setCursor(QCursor(Qt::SizeAllCursor));

            if((event->pos().x() == captureRect.left() && event->pos().y() == captureRect.top() || event->pos().x() == captureRect.right() && event->pos().y() == captureRect.bottom()))
                setCursor(QCursor(Qt::SizeFDiagCursor));
            else if((event->pos().x() == captureRect.right() && event->pos().y() == captureRect.top() || event->pos().x() == captureRect.left() && event->pos().y() == captureRect.bottom()))
                setCursor(QCursor(Qt::SizeBDiagCursor));
            else if(event->pos().x() == captureRect.left() || event->pos().x() == captureRect.right())
                setCursor(QCursor(Qt::SizeHorCursor));
            else if(event->pos().y() == captureRect.top() || event->pos().y() == captureRect.bottom())
                setCursor(QCursor(Qt::SizeVerCursor));
        }
        else
            setCursor(QCursor(Qt::ArrowCursor));

        if(captureRectPress && !nonDiagonallyResizing && !diagonallyResizing)
        {
            QPoint delta = event->pos() - captureRectPressPoint;
            captureRectPressPoint = event->pos();

            if(captureRect.x() + delta.x() < 0)
                delta.setX(-captureRect.x());
            if(captureRect.y() + delta.y() < 0)
                delta.setY(-captureRect.y());
            if(captureRect.right() + delta.x() > desktopPixmap.width() - 1)
                delta.setX(desktopPixmap.width() - 1 - captureRect.right());
            if(captureRect.bottom() + delta.y() > desktopPixmap.height() - 1)
                delta.setY(desktopPixmap.height() - 1 - captureRect.bottom());

            captureRect.translate(delta);

            update();
        }
    }
}

void ScreenCapture::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.drawPixmap(0, 0, desktopPixmap);
    painter.fillRect(0, 0, desktopPixmap.width(), desktopPixmap.height(), QColor(0, 0, 0, 100));
    painter.drawPixmap(captureRect, desktopPixmap.copy(captureRect));

    QPen pen;
    pen.setWidth(3);
    pen.setColor(QColor(87, 175, 215));

    painter.setPen(pen);
    painter.drawRect(captureRect);

    if(captureRect != QRect())
    {
        QRect sizeRect(0, 0, 72, 20);

        sizeRect.translate(captureRect.topLeft());

        if(sizeRect.right() > desktopPixmap.width() - 1)
            sizeRect.translate(-sizeRect.width(), 0);
        else
        {
            sizeRect.translate(0, -sizeRect.height());
            if(sizeRect.y() < 0)
                sizeRect.translate(0, sizeRect.height());
        }

        painter.fillRect(sizeRect, QColor(76, 76, 76, 192));

        QPen pen;
        pen.setColor(0xffffff);
        painter.setPen(pen);
        painter.drawText(sizeRect, Qt::AlignCenter, QString::number(captureRect.width()) + " x " + QString::number(captureRect.height()));
    }
}


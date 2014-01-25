#pragma once

#include "ScreenCapture.h"

#ifdef Q_OS_WIN32
class WIN32ScreenCapture : public ScreenCapture
{
protected:
    void enumChildWindows(HWND parent, vector<MyWindow> &orderWindows)
    {

        queue<HWND> windowQueue;
        windowQueue.push(parent);
        while(!windowQueue.empty())
        {
            HWND hWnd = windowQueue.front();
            windowQueue.pop();

            hWnd = ::GetTopWindow(hWnd);
            while(hWnd != NULL)
            {
                if( IsWindow(hWnd) && IsWindowVisible(hWnd) && !IsIconic(hWnd))
                {
                    RECT windowRect;
                    ::GetWindowRect(hWnd, &windowRect);
                    if(windowRect.left < 0)
                        windowRect.left = 0;
                    if(windowRect.top < 0)
                        windowRect.top = 0;
                    if(windowRect.right > desktopPixmap.width() - 1)
                        windowRect.right = desktopPixmap.width() - 1;
                    if(windowRect.bottom > desktopPixmap.height() - 1)
                        windowRect.bottom = desktopPixmap.height() - 1;

                    QRect rect(QPoint(windowRect.left, windowRect.top), QPoint(windowRect.right, windowRect.bottom));

                    MyWindow myWindow;
                    myWindow.hwnd = hWnd;
                    myWindow.rect = rect;
                    wchar_t str[128];
                    ::GetWindowText(hWnd, str, 128);
                    myWindow.name = QString::fromStdWString(str);

                    if(parent == NULL)//顶级窗口需判断是否被顶级窗口完全遮挡
                    {
                        int i;
                        for (i = 0; i < orderWindows.size(); i++)
                        {
                            if(orderWindows[i].rect.contains(rect))
                                break;
                        }

                        if(i == orderWindows.size())
                            orderWindows.push_back(myWindow);
                    }
                    else
                    {
                        orderWindows.push_back(myWindow);
                        windowQueue.push(hWnd);
                    }
                }

                hWnd = ::GetNextWindow(hWnd, GW_HWNDNEXT);
            }
        }
    }
};

#endif
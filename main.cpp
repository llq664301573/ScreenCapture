#include "WIN32ScreenCapture.h"

//#pragma comment( linker, "/subsystem:console /entry:WinMainCRTStartup" )

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    ScreenCapture *screenCapture = new WIN32ScreenCapture();

    QApplication::processEvents();

    MSG msg;
    while(screenCapture->isEnabled() && GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);

        if (msg.message == WM_HOTKEY)
            screenCapture->capture();
    }

    delete screenCapture;

    return msg.wParam;
}

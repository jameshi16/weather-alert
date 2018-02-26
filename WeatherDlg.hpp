#ifndef WEATHERDIALOG_H
#define WEATHERDIALOG_H

#include "windows.h"
#include "tchar.h"

// The procedure function for this window
LRESULT CALLBACK WeatherDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) { //LRESULT is a 64bit int.
    return 0;
}

// Class functions will handle all pointers passed to it, so don't delete anything.
class WeatherDialog {
    public:
    WeatherDialog(HINSTANCE hInstance, LPCTSTR windowTitle, int nCmdShow);
    virtual ~WeatherDialog();

    //Returns false if it can't appear
    bool appear(HWND parentWindow = NULL);

    private:
    WNDCLASSEX wcex;
    HWND hwnd;
    TCHAR* _windowTitle;
    int nCmdShow;
};

#endif
#ifndef WEATHERDLG_H
#define WEATHERDLG_H

#include "windows.h"
#include "tchar.h"

// The procedure function for this window
LRESULT CALLBACK WeatherDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) { //LRESULT is a 64bit int.
    return 0;
}

// Class functions will handle all pointers passed to it, so don't delete anything.
class WeatherDialog {
    public:
    WeatherDialog(HINSTANCE hInstance, LPCTSTR windowTitle);
    virtual ~WeatherDialog();

    bool appear();

    private:
    WNDCLASSEX wcex;
    HWND hwnd;
    LPCTSTR _windowTitle;
};

#endif
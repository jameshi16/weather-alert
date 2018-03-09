#ifndef WEATHERDIALOG_H
#define WEATHERDIALOG_H

#include "windows.h"
#include "windowsx.h"
#include "tchar.h"

#include "weatherInfo/weatherStation.hpp"

//Member Windows
enum : short{
    APIKEY_TEXTBOX,
    LOCATION_TEXTBOX,
    BUTTON
};

// The procedure function for this window
extern LRESULT CALLBACK WeatherDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

// Class functions will handle all pointers passed to it, so don't delete anything.
class WeatherDialog {
    public:
    WeatherDialog(HINSTANCE hInstance, LPCTSTR windowTitle, int nCmdShow);
    virtual ~WeatherDialog();

    //Returns false if it can't appear
    bool appear(HWND parentWindow = NULL);

    private:
    WNDCLASSEX wcex; //the class structure
    HWND hwnd; //the main window

    TCHAR* _windowTitle; //the title of window
    int nCmdShow; //?
};

#endif
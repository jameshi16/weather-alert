#include "WeatherDlg.hpp"

#include <iostream>
#include <cstring>

WeatherDialog::WeatherDialog(HINSTANCE hInstance, LPCTSTR windowTitle) {
    /* Setup the WNDCLASSEX object */
    wcex.cbSize         = sizeof(WNDCLASSEX); //size of struct
    wcex.style          = CS_HREDRAW | CS_VREDRAW; //class style. Horizontal Sizing redraw and Vertical Sizing redraw
    wcex.lpfnWndProc    = WeatherDlgProc; //the window procedure
    wcex.cbClsExtra     = 0; //do I need any extra bytes from the struct?
    wcex.cbWndExtra     = 0; //do I need any extra bytes for the window?
    wcex.hInstance      = hInstance; //the instance calling the window
    wcex.hIcon          = reinterpret_cast<HICON>(LoadImage(NULL, IDI_APPLICATION, IMAGE_ICON, 0, 0, LR_DEFAULTSIZE));
    wcex.hCursor        = reinterpret_cast<HCURSOR>(LoadImage(NULL, IDC_ARROW, IMAGE_CURSOR, 0, 0, LR_DEFAULTSIZE));
    wcex.hbrBackground  = COLOR_WINDOW + 1;
    wcex.lpszMenuName   = NULL; //no default menu
    wcex.lpszClassName  = _T("WeatherDlg");
    wcex.hIconSm        = NULL; //use icon at hIcon

    /* Register the class */
    if (!RegisterClassEx(&wcex)) {
        std::cerr << "WA GUI: Seems like I can't register myself." << std::endl;
    }

    /* Remember the window title */ //I choose to copy because if someone decided to pass a pointer from a limited lifetime object, I can still store it.
    _windowTitle = new TCHAR[sizeof windowTitle];
    std::memcpy(_windowTitle, windowTitle, (sizeof windowTitle) / sizeof(unsigned char)); //you might never know which systems use char as 2 bytes @.@
}

WeatherDialog::~WeatherDialog() {
    delete[] _windowTitle;
}

bool WeatherDialog::appear() {
    //hwnd = CreateWindow(wcex.lpszClassName, _windowTitle.c_str(), WS_OVERLAPPEDWINDOW)
}
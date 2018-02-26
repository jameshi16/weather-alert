#include "WeatherDlg.hpp"

#include <iostream>
#include <cstring>

WeatherDialog::WeatherDialog(HINSTANCE hInstance, LPCTSTR windowTitle, int nCmdShow) {
    /* Setup the WNDCLASSEX object */
    wcex.cbSize         = sizeof(WNDCLASSEX); //size of struct
    wcex.style          = CS_HREDRAW | CS_VREDRAW; //class style. Horizontal Sizing redraw and Vertical Sizing redraw
    wcex.lpfnWndProc    = WeatherDlgProc; //the window procedure
    wcex.cbClsExtra     = 0; //do I need any extra bytes from the struct?
    wcex.cbWndExtra     = 0; //do I need any extra bytes for the window?
    wcex.hInstance      = hInstance; //the instance calling the window
    wcex.hIcon          = reinterpret_cast<HICON>(LoadImage(NULL, IDI_APPLICATION, IMAGE_ICON, 0, 0, LR_DEFAULTSIZE));
    wcex.hCursor        = reinterpret_cast<HCURSOR>(LoadImage(NULL, IDC_ARROW, IMAGE_CURSOR, 0, 0, LR_DEFAULTSIZE));
    wcex.hbrBackground  = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    wcex.lpszMenuName   = NULL; //no default menu
    wcex.lpszClassName  = _T("WeatherDlg");
    wcex.hIconSm        = NULL; //use icon at hIcon

    /* Register the class */
    if (!RegisterClassEx(&wcex))
        std::cerr << "WA GUI: Seems like I can't register myself." << std::endl;

    /* Remember the window title */ //I choose to copy because if someone decided to pass a pointer from a limited lifetime object, I can still store it.
    _windowTitle = new TCHAR[sizeof windowTitle];
    std::memcpy(_windowTitle, windowTitle, (sizeof windowTitle) / sizeof(unsigned char)); //you might never know which systems use char as 2 bytes @.@

    /* nCmdShow store */
    this->nCmdShow = nCmdShow;
}

WeatherDialog::~WeatherDialog() {
    delete[] _windowTitle;
}

//returns false if it can't appear
bool WeatherDialog::appear(HWND parentWindow) {
    hwnd = CreateWindow(wcex.lpszClassName, _windowTitle, 
                        WS_OVERLAPPEDWINDOW,
                        CW_USEDEFAULT, CW_USEDEFAULT,
                        500, 100,
                        parentWindow,
                        NULL,
                        wcex.hInstance,
                        NULL);

    if (!hwnd) {
        std::cerr << "WA GUI: I could not create the window to appear." << std::endl;
        return false;
    }

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);
    return true;
}
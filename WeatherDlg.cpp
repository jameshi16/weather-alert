#include "WeatherDlg.hpp"

#include <iostream>
#include <cstring>

#include "contacter.hpp"
#include "json.hpp"
#include "weatherInfo/weatherInfo.hpp"

//for testing purposes
#include "alerter/alerter.hpp"
#include "reporter/reporter.hpp"

LRESULT CALLBACK WeatherDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    static HWND hwndAPIEdit;
    static HWND hwndLOCEdit;
    static HWND hwndButton;
    static Reporter* reporter = nullptr;

    switch (uMsg) {
        case WM_DESTROY:
            if (reporter == nullptr)
                delete reporter;
            PostQuitMessage(0);
            break;

        case WM_CREATE:
            hwndAPIEdit = CreateWindowEx(0, _T("EDIT"),
                                        NULL,
                                        WS_CHILD | WS_VISIBLE | ES_LEFT,
                                        0, 0, 500, 20, 
                                        hwnd,
                                        (HMENU) APIKEY_TEXTBOX,
                                        (HINSTANCE) GetWindowLong(hwnd, GWL_HINSTANCE),
                                        NULL);

            hwndLOCEdit = CreateWindowEx(0, _T("EDIT"),
                                        NULL,
                                        WS_CHILD | WS_VISIBLE | ES_LEFT,
                                        0, 20, 500, 20,
                                        hwnd,
                                        (HMENU) LOCATION_TEXTBOX,
                                        (HINSTANCE) GetWindowLong(hwnd, GWL_HINSTANCE),
                                        NULL);

            hwndButton = CreateWindow(_T("BUTTON"),
                                    _T("Set"),
                                    WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                                    10, 40, 100, 20,
                                    hwnd,
                                    NULL,
                                    (HINSTANCE) GetWindowLong(hwnd, GWL_HINSTANCE),
                                    NULL);

            SendMessage(hwndAPIEdit, WM_SETTEXT, 0, (LPARAM) _T("API Key here"));
            SendMessage(hwndLOCEdit, WM_SETTEXT, 0, (LPARAM) _T("Location here"));
            break;

        case WM_COMMAND:
            switch (wParam) {
                case BN_CLICKED: {
                    const int size_apikey = Edit_GetTextLength(hwndAPIEdit) + 1;
                    const int size_location = Edit_GetTextLength(hwndLOCEdit) + 1;

                    TCHAR* APIKey = new TCHAR[size_apikey];
                    TCHAR* Location = new TCHAR[size_location];

                    Edit_GetText(hwndAPIEdit, APIKey, size_apikey);
                    Edit_GetText(hwndLOCEdit, Location, size_location);

                    /* Creates a new WeatherStation object */
                    WeatherStation ws;
                    ws.setAPIKey(std::basic_string<TCHAR>(APIKey));
                    ws.setLocation(std::basic_string<TCHAR>(Location));
                    if (reporter != nullptr)
                        delete reporter; //deletes the old pointer

                    reporter = new Reporter(ws, Alerter(hwnd));
                    reporter->start(300); //allows the thing to sleep for 5 minutes

                    delete[] APIKey;
                    delete[] Location;
                    break;
                }

                default:
                    return DefWindowProc(hwnd, uMsg, wParam, lParam);
            }
        
        case WM_APP_PLAYER_EVENT: {
            HRESULT hr = reporter->getAlerter()->HandleEvent(wParam);
            reporter->releaseLock();

            if (FAILED(hr))
                std::cerr << "Can't handle alert event." << std::endl;
            break;
        }

        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
            break;
    }
    return 0;
}

WeatherDialog::WeatherDialog(HINSTANCE hInstance, LPCTSTR windowTitle, int nCmdShow) {
    /* Setup the WNDCLASSEX object */
    wcex.cbSize         = sizeof(WNDCLASSEX); //size of struct
    wcex.style          = CS_HREDRAW | CS_VREDRAW; //class style. Horizontal Sizing redraw and Vertical Sizing redraw
    wcex.lpfnWndProc    = WeatherDlgProc; //the window procedure
    wcex.cbClsExtra     = 0; //do not need extra bytes
    wcex.cbWndExtra     = 0; //do I need any extra bytes for the window?
    wcex.hInstance      = hInstance; //the instance calling the window
    wcex.hIcon          = reinterpret_cast<HICON>(LoadImage(NULL, IDI_APPLICATION, IMAGE_ICON, 0, 0, LR_DEFAULTSIZE));
    wcex.hCursor        = reinterpret_cast<HCURSOR>(LoadImage(NULL, IDC_ARROW, IMAGE_CURSOR, 0, 0, LR_DEFAULTSIZE));
    wcex.hbrBackground  = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    wcex.lpszMenuName   = NULL; //no default menu
    wcex.lpszClassName  = _T("WeatherDlg");
    wcex.hIconSm        = NULL; //use icon at hIcon

    /* Register the class */
    if (!RegisterClassEx(&wcex)) {
        std::cerr << "WA GUI: Seems like I can't register myself." << std::endl;
        std::cerr << "WA GUI: Detailed info: " << GetLastError() << std::endl;
    }

    /* Remember the window title */ //I choose to copy because if someone decided to pass a pointer from a limited lifetime object, I can still store it.
    _windowTitle = new TCHAR[_tcslen(windowTitle) + 1];
    std::memcpy(_windowTitle, windowTitle, _tcslen(windowTitle) + 1);

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
        std::cerr << "WA GUI: I could not create the window." << std::endl;
        std::cerr << "WA GUI: Detailed info: " << GetLastError() << std::endl;
        return false;
    }

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);
    return true;
}
#include "WeatherDlg.hpp"

#include <iostream>
#include <cstring>

#include "contacter.hpp"
#include "json.hpp"
#include "weatherInfo/weatherInfo.hpp"

LRESULT CALLBACK WeatherDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    PAINTSTRUCT ps; //the paint structure
    HDC hdc; //device context handle
    static HWND hwndAPIEdit;
    static HWND hwndLOCEdit;
    static HWND hwndButton;
    static WeatherInfo wi;

    switch (uMsg) {
        /*case WM_PAINT:
            hdc = BeginPaint(hwnd, &ps); //start painting, obtain the device context
            EndPaint(hwnd, &ps); //stop painting
            break;*/

        case WM_DESTROY:
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

                    std::string s_APIKey(APIKey);
                    std::string s_Location(Location);

                    std::cerr << "WA GUI: Setting up with API Key: " << APIKey << " in " << Location << std::endl;
                    /* Variables to get weather data */
                    Contacter contact;
                    JsonDecoder jd;

                    contact.contact(L"JamesLab Softwares", L"api.openweathermap.org", L"/data/2.5/weather?q=" + 
                                    std::wstring(s_Location.begin(), s_Location.end()) + L"&appid=" + std::wstring(s_APIKey.begin(), s_APIKey.end()));
                    contact.obtainData();
                    contact.severContact();

                    jd.decode(contact.getData());
                    wi.readWeatherData(jd);

                    std::cerr << "Status is: " << wi.weatherName << std::endl;

                    delete[] APIKey;
                    delete[] Location;
                    break;
                }

                default:
                    return DefWindowProc(hwnd, uMsg, wParam, lParam);
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

void WeatherDialog::message_loop() {
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}
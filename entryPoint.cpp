#include <iostream>

//Windows Headers
#include "windows.h"
#include "tchar.h"

#include "weatherInfo/weatherStation.hpp"
#include "WeatherDlg.hpp"

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
    WeatherStation ws;
    WeatherDialog wd(hInstance, _T("Weather Alert"), nShowCmd);
    wd.appear(&ws);
    

    /** The Message Loop **/
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    int x;
    std::cin >> x;
    return 0;
}
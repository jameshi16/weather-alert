#include <iostream>
#include <chrono>
#include <thread>

//Windows Headers
#include "windows.h"
#include "tchar.h"

#include "weatherInfo/weatherStation.hpp"
#include "WeatherDlg.hpp"
#include "alerter/alerter.hpp"

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
    WeatherDialog wd(hInstance, _T("Weather Alert"), nShowCmd);
    wd.appear();
    

    /** The Message Loop **/
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}
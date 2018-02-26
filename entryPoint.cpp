#include <iostream>

//Windows Headers
#include "windows.h"
#include "tchar.h"

#include "WeatherDlg.hpp"

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
    WeatherDialog wd(hInstance, _T("Weather Alert"), nShowCmd);
    wd.appear();
    wd.message_loop();

    int x;
    std::cin >> x;
    return 0;
}
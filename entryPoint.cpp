#include <iostream>

//Windows Headers
#include "windows.h"
#include "tchar.h"

#include "WeatherDlg.hpp"

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
    WeatherDialog wd(hInstance, _T("wassup"), nShowCmd);
    wd.appear();
    return 0;
}
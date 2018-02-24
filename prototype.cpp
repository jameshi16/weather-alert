#define PROTOTYPE

// The prototype compilation unit. I used this to see if I am able to contact Open Weather Map effectively.
#ifdef PROTOTYPE

#include <iostream>
#include <string>

#include "windows.h"
#include "contacter.hpp"
#include "json.hpp"

INT WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, INT nCmdShow) {
    JsonDecoder jd;
    std::string input;

    std::cin >> input;
    jd.decode(input);

    std::cout << jd["base"].value << std::endl;
    std::cout << "done" << std::endl;
    return 0;
}

#endif 
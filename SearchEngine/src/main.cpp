#include "Application/Application.h"
#include <iostream>
#include <Windows.h>

int main() {
    std::locale::global(std::locale("Russian"));
    std::setlocale(LC_ALL, "Russian");
    SetConsoleOutputCP(1251);
    SetConsoleCP(1251);

    try {
        Application app("Config.ini");

        #ifdef FULL_PROJECT_MODE
        app.run();
        #else
            #ifdef CONSOLE_SEARCH_MODE
            app.ConsoleSearch();
            #else
            app.HTMLSearch();
            #endif
        #endif

        return 0;
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
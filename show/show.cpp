#define _CRT_SECURE_NO_WARNINGS

#include <string>

#include <thread>
#include <windows.h>

#include "Draw.h"
#include "tools.h"
#include "magic_enum/magic_enum.hpp"

int main() {
    SetConsoleOutputCP(65001);

    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

    int width = 120;
    int height = 16;

    DrawScreen screen = DrawScreenBuilder::load_font("font/mingliu7.03/mingliu_Fixedsys_Excelsior.bin")
                        .set_screen_size(width * height)
                        .set_hConsole(hConsole)
                        .build();

    DisplayConfig builder = DisplayConfigBuilder::load_form_file("setting.json")
                            .set_screen_width(width)
                            .build();

    //screen.invoke_method<Marquee>(builder);

    screen.screen_clear();
    screen.invoke_method(IEffect::EffectEnum::Flash, &builder);
}

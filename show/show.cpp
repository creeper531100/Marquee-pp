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

    DisplayConfigBuilder builder = DisplayConfigBuilder::load_form_file("setting.json")
                            .set_screen_width(width)
                            .build();

    screen.screen_clear();
    screen.invoke_method(IEffect::EffectEnum::Flash, (uintptr_t)&builder);
    screen.delay(1000);
    screen.invoke_method("Slide", (uintptr_t)&builder.set_ws(L"你好"));
    screen.invoke_method("delay", (uintptr_t)"1000");

    screen.display<Marquee>(builder.set_x_end(100));
}

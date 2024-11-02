#define _CRT_SECURE_NO_WARNINGS

#include <fstream>
#include <sstream>
#include <string>

#include <thread>
#include <windows.h>

#include "Draw.h"
#include "tools.h"
#include "magic_enum/magic_enum.hpp"

int main() {
    SetConsoleOutputCP(65001);

    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_FONT_INFOEX fontInfo;
    fontInfo.cbSize = sizeof(CONSOLE_FONT_INFOEX);
    GetCurrentConsoleFontEx(hConsole, FALSE, &fontInfo);

    fontInfo.dwFontSize.Y = 14;

    SetCurrentConsoleFontEx(hConsole, FALSE, &fontInfo);

    RECT rect = {0, 0, 16 * 14 * 8 + 48, 32 * 7 + 64};
    MoveWindow(GetConsoleWindow(), rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, TRUE);

    CONSOLE_SCREEN_BUFFER_INFO console_buffer_info = {0};
    GetConsoleScreenBufferInfo(hConsole, &console_buffer_info);

    int width = console_buffer_info.srWindow.Right - console_buffer_info.srWindow.Left + 1;
    int height = 16;

    nlohmann::json json;

    SAOFU_TRY({
        std::ifstream ifs("setting.json");
        json = nlohmann::json::parse(ifs);
        })

    DrawScreenBuilder screen = DrawScreenBuilder::builder(width * height, hConsole)
                               .load_font("font/mingliu7.03/mingliu_Fixedsys_Excelsior.bin")
                               .build();


    for (auto& row : json["exec"]) {
        std::string effect = row["effect"];
        std::string arg = row["arg"];

        DisplayConfigBuilder param = DisplayConfigBuilder::builder(width)
                                     .set_ws(SaoFU::utf8_to_utf16(arg))
                                     .load_form_json(row)
                                     .build();

        if (!screen.create_instance(effect, (uintptr_t)&param)) {
            screen.invoke_method(effect, (uintptr_t)arg.c_str());
        }
    }
}

#define _CRT_SECURE_NO_WARNINGS
#define _CRT_NON_CONFORMING_SWPRINTFS
#include <fstream>
#include <future>

#include "tools.h"

#include <iostream>
#include <thread>
#include <windows.h>
#include <sstream>

#include <curl/curl.h>

#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")

class Marquee {
public:
    bool done = false;
    const int width;
    const int height;
private:
    size_t screen_size;

    wchar_t* screen;
    Font* font;
    HANDLE hConsole;
public:
    Marquee(int width, int height, wchar_t* screen, Font* font, HANDLE hConsole) :
        width(width), height(height), screen(screen), font(font), hConsole(hConsole) {
        this->screen_size = width * height;
    }

    Marquee& screen_clear() {
        DWORD dwBytesWritten;
        memset(screen, 0, screen_size * sizeof(wchar_t));
        WriteConsoleOutputCharacterW(hConsole, screen, screen_size, {0, 0}, &dwBytesWritten);
        return *this;
    }

    /**
     * 清除螢幕區域的文字，根據指定的清除方法（詳細在SaoFU::utils::TextClearMethod內）進行清除。
     *
     * @param param  清除參數，需設定以下參數：
     *               - screen_width: 螢幕寬度
     *               - x_offset: 偏移量
     *               - ws: 顯示文字
     * @param row  掃描索引
     * @param enable_clear  是否清除文字
     * @return 一個包含清除範圍的std::pair，first表示清除區域的起始位置，second表示結束位置
     */
    std::pair<int, int> clear_text_region(Param& param, int index = 0, bool clear_region = true) {
        std::pair<int, int> region;
        int x_begin = 0;
        int x_end = 0;

        switch (param.screen_clear_method) {
        case SaoFU::utils::TextClearMethod::ClearAllText:
            x_begin = 0;
            x_end = param.screen_width;
            break;
        case SaoFU::utils::TextClearMethod::ClearTextItself:
            x_begin = param.x_offset;
            x_end = SaoFU::count_size(param.ws);
            break;
        case SaoFU::utils::TextClearMethod::ClearTextBefore:
            x_begin = 0;
            x_end = SaoFU::count_size(param.ws) + param.x_offset;
            break;
        case SaoFU::utils::TextClearMethod::ClearTextAfter:
            x_begin = param.x_offset;
            x_end = param.screen_width - param.x_offset;
            break;
        case SaoFU::utils::TextClearMethod::ClearAll:
            memset(screen, 0, screen_size * sizeof(wchar_t));
            break;
        }

        region.first = x_begin;
        region.second = x_end;

        if (clear_region) {
            memset(screen + (width * index + x_begin), 0, x_end * sizeof(wchar_t));
        }

        return region;
    }

    //TODO 刷新偏移
    /**
     *  Param所需參數說明
     *  @param param wstring ws          你要顯示的字串
     *  @param param int x_begin         x開始位置
     *  @param param int x_end           x截止位置
     *  @param param int x_offset        x偏移
     *  其他參數說明
     *  @param param uint64_t delay_time 休眠時間
     */
    Marquee& marquee(Param& param) {
        DWORD dwBytesWritten;

        param.y_begin = 0;
        param.y_end = 16;
        param.y_offset = 0;

        for (int i = param.x_offset; i >= param.x_end; i--) {
            memset(screen, 0, screen_size * sizeof(wchar_t));
            param.x_offset = i;

            SaoFU::draw_text(font, &param, screen);
            WriteConsoleOutputCharacterW(hConsole, screen, screen_size, {0, 0}, &dwBytesWritten);

            if (i % param.step == 0) {
                delay(param.time);
            }

            if (done) {
                return *this;
            }
        }
        return *this;
    }

    /**
     *  Param所需參數說明
     *  @param param wstring ws          你要顯示的字串
     *  @param param int x_offset        x偏移
     *  其他參數說明
     *  @param param uint64_t delay_time 休眠時間
     */
    Marquee& slide(Param& param) {
        DWORD dwBytesWritten;
        param.y_end = 16;

        for (int i = 0; i < 16; i++) {
            for (int j = 0; j < 15; j++) {
                auto region = clear_text_region(param, j, false);
                memmove(screen + (width * j) + region.first, screen + (width * (j + 1)) + region.first,
                        region.second * sizeof(wchar_t));
                memset(screen + ((15 - i) * width) + region.first, 0, region.second * sizeof(wchar_t));
            }

            /*
             * 可以方便的將每一列往前移動一行，但不能控制偏移
             * memmove(screen, screen + width, (width * 15) * sizeof(wchar_t)); 
             * memset(screen + ((15 - i) * width), 0, width * sizeof(wchar_t));
             */

            param.y_begin = 15 - i;
            param.y_offset = 15 - i;

            SaoFU::draw_text(font, &param, screen);

            WriteConsoleOutputCharacterW(hConsole, screen, screen_size, {0, 0}, &dwBytesWritten);

            if (i % param.step == 0) {
                delay(param.time);
            }

            if (done) {
                return *this;
            }
        }

        return *this;
    }

    /**
     *  Param所需參數說明
     *  @param param wstring ws          你要顯示的字串
     *  @param param int x_begin         x開始位置 
     *  @param param int x_end           x截止位置
     *  @param param int x_offset        x偏移 
     *  其他參數說明
     *  @param param uint64_t delay_time 休眠時間
     */
    Marquee& flash(Param& param) {
        DWORD dwBytesWritten;
        param.y_begin = 0;
        param.y_offset = 0;

        for (int i = 0; i < 16; i++) {
            clear_text_region(param, i);
            param.y_end = i;

            SaoFU::draw_text(font, &param, screen);
            WriteConsoleOutputCharacterW(hConsole, screen, screen_size, {0, 0}, &dwBytesWritten);

            if (i % param.step == 0) {
                delay(param.time);
            }

            if (done) {
                return *this;
            }
        }

        return *this;
    }

    Marquee& delay(int time) {
        if (SaoFU::g_trigger == true || this->done == true) {
            this->done = true;
            SaoFU::g_trigger = 0;
            return *this;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(time));
        return *this;
    }

    Marquee& long_delay(int time) {
        for (int i = 0; i < time; i++) {
            if (delay(1).done) {
                return *this;
            }
        }
        return *this;
    }

    Marquee& jump_to_here() {
        this->done = false;
        return *this;
    }

    // 辅助函数，根据方法名调用相应的方法

    using MemberFunctionPointer = std::function<void(std::any, Param& param)>;
    using MethodMap = std::map<std::string, MemberFunctionPointer>;
    void invoke_method(const std::string& methodName, std::any args, Param& param) {
        MethodMap functionMap = {
            {"screen_clear", [this](std::any args, Param& param) {
                this->screen_clear();
            }},
            {"long_delay", [this](std::any args, Param& param) {
                ULONG64 value = std::any_cast<ULONG64>(args);
                this->long_delay(value);
            }},
            {"marquee", [this](std::any args, Param& param) {
                param.x_end = -SaoFU::count_size(param.ws);
                this->marquee(param);
            }},
            {"slide", [this](std::any args, Param& param) {
                this->slide(param);
            }},
            {"delay", [this](std::any args, Param& param) {
                ULONG64 value = std::any_cast<ULONG64>(args);
                this->delay(value);
            }},
        };

        functionMap[methodName](args, param);
    }
};

#include<nlohmann/json.hpp>
using Json = nlohmann::json;

void marquee_exec(Json json, Marquee& marquee) {
    for (auto& row : json["exec"]) {
        Param param;
        param.ws = SaoFU::get_time(SaoFU::utf8_to_utf16(row["str"]).c_str());
        param.screen_width = marquee.width;

        param.step = row["effect"]["step"];
        param.time = row["effect"]["time"];

        param.x_offset = std::map<std::string, int> {
            {"begin", 0},
            {"center", (param.screen_width / 2) - (SaoFU::count_size(param.ws) / 2)},
            {"end", marquee.width}
        }[row["effect"]["text_position"]];

        param.screen_clear_method = std::map<std::string, SaoFU::utils::TextClearMethod> {
            {"None", SaoFU::utils::TextClearMethod::None},
            {"ClearAllText", SaoFU::utils::TextClearMethod::ClearAllText},
            {"ClearTextItself", SaoFU::utils::TextClearMethod::ClearTextItself},
            {"ClearTextBefore", SaoFU::utils::TextClearMethod::ClearTextBefore},
            {"ClearTextAfter", SaoFU::utils::TextClearMethod::ClearTextAfter},
            {"ClearAll", SaoFU::utils::TextClearMethod::ClearAll}
        }[row["effect"]["clear_text_region"]];

        for (auto& cols : row["run"]) {
            std::stringstream ss((std::string)cols);
            std::string key;
            ULONG64 value = 0;
            ss >> key >> value;
            marquee.invoke_method(key, value, param);
        }
    }
}

int main() {
    SetConsoleOutputCP(65001);
    system("cls");

    std::ifstream ifs("setting.json");
    Json json = nlohmann::json::parse(ifs);

    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_FONT_INFOEX fontInfo;
    fontInfo.cbSize = sizeof(CONSOLE_FONT_INFOEX);
    GetCurrentConsoleFontEx(hConsole, FALSE, &fontInfo);

    fontInfo.dwFontSize.Y = 14;

    SetCurrentConsoleFontEx(hConsole, FALSE, &fontInfo);

    RECT rect = { 0, 0, 16 * 14 * 8 + 48, 32 * 7 + 64 };
    MoveWindow(GetConsoleWindow(), rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, TRUE);

    FILE* fp = fopen("font/mingliu7.03/mingliu_Fixedsys_Excelsior.bin", "rb+");
    fseek(fp, 0L, SEEK_END);

    int size = ftell(fp);
    char* buffer = new char[size];

    fseek(fp, 0L, SEEK_SET);
    fread(buffer, size, 1, fp);
    fclose(fp);

    Font* pack = (Font*)buffer;

    CONSOLE_SCREEN_BUFFER_INFO console_buffer_info = { 0 };
    GetConsoleScreenBufferInfo(hConsole, &console_buffer_info);

    int width = console_buffer_info.srWindow.Right - console_buffer_info.srWindow.Left + 1;
    int height = console_buffer_info.srWindow.Bottom - console_buffer_info.srWindow.Top;
    int screen_size = width * height;

    wchar_t* screen = new wchar_t[screen_size];
    Marquee marquee(width, height, screen, pack, hConsole);
    marquee.screen_clear();

    marquee_exec(json, marquee);
}

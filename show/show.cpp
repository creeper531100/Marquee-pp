﻿#define _CRT_SECURE_NO_WARNINGS
#define _CRT_NON_CONFORMING_SWPRINTFS
#include <fstream>
#include <future>

#include "tools.h"

#include <iostream>
#include <thread>
#include <windows.h>
#include <sstream>

#include <curl/curl.h>

using Json = nlohmann::json;

#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")

class Marquee {
public:
    const int width;
    const int height;
private:
    size_t screen_size;
    wchar_t* screen;
    Font* font;
    HANDLE hConsole;

    using MemberFunctionPointer = std::function<void(std::any, Param& param)>;
    using MethodMap = std::map<std::string, MemberFunctionPointer>;
    MethodMap method_map;
public:
    Marquee(int width, int height, wchar_t* screen, Font* font, HANDLE hConsole) :
        width(width), height(height), screen(screen), font(font), hConsole(hConsole) 
    {
        this->screen_size = width * height;

        method_map["screen_clear"] = [this](std::any args, Param& param) {
            this->screen_clear();
        };

        method_map["marquee"] = [this](std::any args, Param& param) {
            this->marquee(param);
        };

        method_map["slide"] = [this](std::any args, Param& param) {
            this->slide(param);
        };

        method_map["flash"] = [this](std::any args, Param& param) {
            this->flash(param);
        };

        method_map["delay"] = [this](std::any args, Param& param) {
            ULONG64 value = std::any_cast<ULONG64>(args);
            this->delay(value);
        };
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
            param.y_end = i + 1;

            SaoFU::draw_text(font, &param, screen);
            WriteConsoleOutputCharacterW(hConsole, screen, screen_size, {0, 0}, &dwBytesWritten);

            if (i % param.step == 0) {
                delay(param.time);
            }
        }

        return *this;
    }

    Marquee& delay(int time) {
        std::this_thread::sleep_for(std::chrono::milliseconds(time));
        return *this;
    }

    void invoke_method(const std::string& methodName, std::any args, Param& param) {
        auto it = method_map.find(methodName);
        if (it == method_map.end()) {
            throw std::runtime_error("找不到方法: " + methodName);
        }
        it->second(args, param);
    }
};

void marquee_exec(Json json, Marquee& marquee) {
    for (auto& row : json["exec"]) {
        Param param;
        param.ws = SaoFU::get_time(SaoFU::utf8_to_utf16(row["str"]).c_str());
        param.screen_width = marquee.width;

        param.step = row["effect"]["step"];
        param.time = row["effect"]["time"];

        std::map<std::string, int> begin_position = {
            { "begin", 0},
            { "center", (param.screen_width / 2) - (SaoFU::count_size(param.ws) / 2) },
            { "end", param.screen_width }
        };

        if (begin_position.find(row["effect"]["begin_position"]) != begin_position.end()) {
            param.x_offset = begin_position[row["effect"]["begin_position"]];
        }
        else {
            param.x_offset = std::stoi((std::string)row["effect"]["begin_position"]);
        }

        std::map<std::string, int> end_position {
            {"top", 0},
            {"center", (param.screen_width / 2) - (SaoFU::count_size(param.ws) / 2)},
            {"end", param.screen_width },
            {"last_char",-SaoFU::count_size(param.ws) }
        };

        if (end_position.find(row["effect"]["end_position"]) != end_position.end()) {
            param.x_end = end_position[row["effect"]["end_position"]];
        }
        else {
            param.x_end = std::stoi((std::string)row["effect"]["end_position"]);
        }

        param.screen_clear_method = std::map<std::string, SaoFU::utils::TextClearMethod> {
            #define X(method) {#method, SaoFU::utils::TextClearMethod::method},
                TEXT_CLEAR_METHOD_ENUM
            #undef X
        }[row["effect"]["clear_text_region"]];

        for (auto& cols : row["run"]) {
            std::stringstream ss((std::string)cols);
            std::string key;
            ULONG64 value = 0;
            ss >> key >> value;
            SAOFU_TRY({ marquee.invoke_method(key, value, param); });
        }
    }
}

int main() {
    SetConsoleOutputCP(65001);
    system("cls");
    Json json;

    SAOFU_TRY({
        std::ifstream ifs("setting.json");
        json = nlohmann::json::parse(ifs);
    })

    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_FONT_INFOEX fontInfo;
    fontInfo.cbSize = sizeof(CONSOLE_FONT_INFOEX);
    GetCurrentConsoleFontEx(hConsole, FALSE, &fontInfo);

    fontInfo.dwFontSize.Y = 14;

    SetCurrentConsoleFontEx(hConsole, FALSE, &fontInfo);

    RECT rect = { 0, 0, 16 * 14 * 8 + 48, 32 * 7 + 64 };
    MoveWindow(GetConsoleWindow(), rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, TRUE);

    FILE* fp = NULL;
    std::string font_path = json["font_path"];
    fp = fopen(font_path.c_str(), "rb+");

    if (!fp) {
        SaoFU::e_what(__LINE__, "找不到字體", 29);
        return 0;
    }

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

    while (1) {
        marquee_exec(json, marquee);
    }
}

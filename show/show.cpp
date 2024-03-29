﻿#define _CRT_SECURE_NO_WARNINGS
#define _CRT_NON_CONFORMING_SWPRINTFS
#include <fstream>
#include "tools.h"

#include <iostream>
#include <thread>
#include "fmt/format.h"

#include <windows.h>
#include <sstream>
#include "magic_enum/magic_enum.hpp"


void draw_text(Font* font, DisplayConfig* param, wchar_t* screen) {

    const int& glyph_width = 16 * param->glyph_width_factor;
    const int glyph_height = param->glyph_height;

    const int& y_begin = param->y_begin;
    const int& y_end = param->y_end;
    const int& y_offset = param->y_offset;

    const int& x_offset = param->x_offset;
    const int& width = param->screen_width;

    int start = 0;

    for (int i = 0; i < param->ws.length(); i++) {
        bool is_half_width = false;
        wchar_t ch = param->ws[i];

        // 畫出一個字元
        for (int row = y_begin; row < y_end; row++) {
            for (int cols = 0; cols < glyph_width; cols += param->glyph_width_offset) {
                // 超出畫面寬度就不用繼續畫了
                bool in_max_range = start + cols + x_offset < width;
                bool in_min_range = start + cols + x_offset >= 0;

                if (in_max_range && in_min_range) {
                    bool b = font[ch][row - y_offset] & 0x8000 >> cols / param->glyph_width_factor;
                    screen[x_offset + start + (row * width + cols)] = b ? param->fill_char : param->background;
                }
            }
        }

        for (int row = 0; row < param->glyph_height; row++) {
            // 判斷是否為半形字元(字元另一半是空的)
            is_half_width |= font[ch][row] & 0xFF;
        }

        // 計算下一個字元的起始位置
        start += (is_half_width ? glyph_width : glyph_width / 2);
    }
}

class Marquee {
public:
    const int width;
    const int height;
private:
    size_t screen_size;
    wchar_t* screen;
    Font* font;
    HANDLE hConsole;

    //請依照順序，不然虛表會對齊錯誤!!!
    enum class Methods {
        screen_clear,
        marquee,
        slide,
        flash,
        delay
    };

    using MethodHandler = void(*)(void*, uintptr_t);
public:
    Marquee(int width, int height, wchar_t* screen, Font* font, HANDLE hConsole) :
        width(width), height(height), screen(screen), font(font), hConsole(hConsole)
    {
        this->screen_size = width * height;
    }

    virtual void screen_clear() {
        DWORD dwBytesWritten;
        wmemset(screen, L' ', screen_size);
        WriteConsoleOutputCharacterW(hConsole, screen, screen_size, { 0, 0 }, &dwBytesWritten);
    }

    //TODO 刷新偏移
    /**
     *  Param所需參數說明
     *  @config config wstring ws          你要顯示的字串
     *  @config config int x_begin         x開始位置
     *  @config config int x_end           x截止位置
     *  @config config int x_offset        x偏移
     *  其他參數說明
     *  @config config uint64_t delay_time 休眠時間
     */
    virtual void marquee(DisplayConfig& config) {
        DWORD dwBytesWritten;

        config.y_begin = 0;
        config.y_end = 16;
        config.y_offset = 0;

        for (int i = config.x_offset; i >= -config.x_end; i--) {
            wmemset(screen, config.background, screen_size);
            config.x_offset = i;

            draw_text(font, &config, screen);
            WriteConsoleOutputCharacterW(hConsole, screen, screen_size, { 0, 0 }, &dwBytesWritten);

            if (i % config.step == 0) {
                delay(config.time);
            }
        }
    }

    /**
     *  Param所需參數說明
     *  @config config wstring ws          你要顯示的字串
     *  @config config int x_offset        x偏移
     *  其他參數說明
     *  @config config uint64_t delay_time 休眠時間
     */
    virtual void slide(DisplayConfig& config) {
        DWORD dwBytesWritten;
        config.y_end = 16;

        for (int i = 0; i < 16; i++) {
            for (int j = 0; j < 15; j++) {
                auto region = clear_text_region(config, j, false);
                wmemmove(screen + (width * j) + region.first, screen + (width * (j + 1)) + region.first, region.second);
                wmemset(screen + ((15 - i) * width) + region.first, config.background, region.second);
            }

            /*
             * 可以方便的將每一列往前移動一行，但不能控制偏移
             * memmove(screen, screen + width, (width * 15) * sizeof(wchar_t));
             * memset(screen + ((15 - i) * width), 0, width * sizeof(wchar_t));
             */

            config.y_begin = 15 - i;
            config.y_offset = 15 - i;

            draw_text(font, &config, screen);

            WriteConsoleOutputCharacterW(hConsole, screen, screen_size, { 0, 0 }, &dwBytesWritten);

            if (i % config.step == 0) {
                delay(config.time);
            }
        }
    }

    /**
     *  Param所需參數說明
     *  @config config wstring ws          你要顯示的字串
     *  @config config int x_begin         x開始位置
     *  @config config int x_end           x截止位置
     *  @config config int x_offset        x偏移
     *  其他參數說明
     *  @config config uint64_t delay_time 休眠時間
     */
    virtual void flash(DisplayConfig& config) {
        DWORD dwBytesWritten;
        config.y_begin = 0;
        config.y_offset = 0;

        for (int i = 0; i < 16; i++) {
            clear_text_region(config, i);
            config.y_end = i + 1;

            draw_text(font, &config, screen);
            WriteConsoleOutputCharacterW(hConsole, screen, screen_size, { 0, 0 }, &dwBytesWritten);

            if (i % config.step == 0) {
                delay(config.time);
            }
        }
    }

    virtual void delay(uint64_t param) {
        Variant<std::string> hi(param);
        int time = inspect_ptr_or_string(hi).and_then(convert_to_int<uintptr_t>).value_or(param);

        std::this_thread::sleep_for(std::chrono::milliseconds(time));
    }

    /**
     * 清除螢幕區域的文字，根據指定的清除方法（詳細在SaoFU::utils::TextClearMethod內）進行清除。
     *
     * @config config  清除參數，需設定以下參數：
     *               - screen_width: 螢幕寬度
     *               - x_offset: 偏移量
     *               - ws: 顯示文字
     * @config row  掃描索引
     * @config enable_clear  是否清除文字
     * @return 一個包含清除範圍的std::pair，first表示清除區域的起始位置，second表示結束位置
     */
    std::pair<int, int> clear_text_region(DisplayConfig& config, int index = 0, bool clear_region = true) {
        std::pair<int, int> region;
        int x_begin = 0;
        int x_end = 0;

        switch (config.clear_text_region) {
        case SaoFU::utils::TextClearMethod::ClearAllText:
            x_begin = 0;
            x_end = config.screen_width;
            break;
        case SaoFU::utils::TextClearMethod::ClearTextItself:
            x_begin = config.x_offset;
            x_end = SaoFU::count_size(config.ws);
            break;
        case SaoFU::utils::TextClearMethod::ClearTextBefore:
            x_begin = 0;
            x_end = SaoFU::count_size(config.ws) + config.x_offset;
            break;
        case SaoFU::utils::TextClearMethod::ClearTextAfter:
            x_begin = config.x_offset;
            x_end = config.screen_width - config.x_offset;
            break;
        case SaoFU::utils::TextClearMethod::ClearAll:
            wmemset(screen, config.background, screen_size);
            break;
        }

        region.first = x_begin;
        region.second = x_end;

        if (clear_region) {
            wmemset(screen + (width * index + x_begin), config.background, x_end);
        }

        return region;
    }

    //這樣寫應該會被扁
    void invoke_method(const std::string& methodName, ULONG64 param) {
        const uintptr_t* vtable = *(uintptr_t**)this;
        const auto e = magic_enum::enum_cast<Methods>(methodName);

        if (!e.has_value()) {
            throw std::runtime_error("找不到方法: " + methodName);
        }

        MethodHandler method = (MethodHandler)vtable[(int)e.value()];
        method(this, param);
    }
};

void display_config_init(Json& j, DisplayConfig& p) {
    Json effect = j["effect"];

    p.ws = SaoFU::get_time(SaoFU::utf8_to_utf16(j["ws"]).c_str());
    p.step = effect.value("step", p.step);
    p.time = effect.value("time", p.time);

    std::string x_offset = effect.value("x_offset", "begin");
    std::string x_begin = effect.value("x_begin", "begin");
    std::string x_end = effect.value("x_end", "end");
    std::string clear_text = effect.value("clear_text_region", "ClearAllText");

    p.x_begin = try_parse(x_begin).or_else(resolve_keyword, p, x_begin).value_or(0);
    p.x_offset = try_parse(x_offset).or_else(resolve_keyword, p, x_offset).value_or(0);
    p.x_end = try_parse(x_end).or_else(resolve_keyword, p, x_end).value_or(0);
    p.clear_text_region = magic_enum::enum_cast<SaoFU::utils::TextClearMethod>(clear_text).value();

    p.glyph_height = effect.value("glyph_height", p.glyph_height);
    p.glyph_width = effect.value("glyph_width", p.glyph_width);
    p.glyph_width_offset = effect.value("glyph_width_offset", p.glyph_width_offset);
    p.glyph_width_factor = effect.value("glyph_width_factor", p.glyph_width_factor);

    p.fill_char = effect.value("fill_char", p.fill_char);
    p.background = effect.value("background", p.background);

    p.y_begin = effect.value("y_begin", p.y_begin);
    p.y_end = effect.value("y_end", p.y_end);
    p.y_offset = effect.value("y_offset", p.y_offset);
}

int main() {
    SetConsoleOutputCP(65001);
    system("cls");

    FILE* fp = NULL;
    char ver_buffer[100] = { 0 };
    int ver;

    fp = _popen("powershell (Get-WmiObject Win32_OperatingSystem).Version", "r");
    fread(ver_buffer, sizeof(ver_buffer), 1, fp);
    _pclose(fp);

    sscanf_s(ver_buffer, "%*d.%*d.%d", &ver);
    printf("%s", ver_buffer);

    if (ver >= 22000) {
        printf(u8"windows11 請自己調整視窗大小\n", ver_buffer, ver);
        system("pause");
    }

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
    int height = 16;
    int screen_size = width * height;

    wchar_t* screen = new wchar_t[screen_size];
    Marquee marquee(width, height, screen, pack, hConsole);

    marquee.screen_clear();

    while (1) {
        for (auto& row : json["exec"]) {
            Json effect = row["effect"];
            DisplayConfig config;
            config.screen_width = width;
            display_config_init(row, config);

            for (auto& cols : row["run"]) {
                std::stringstream ss((std::string)cols);
                std::string key = "";
                std::string value = "";

                ss >> key >> value;
                uintptr_t ptr = is_empty_string<uintptr_t>(&value).value_or((uintptr_t)&config);

                marquee.invoke_method(key, ptr);
            }
        }
    }
}
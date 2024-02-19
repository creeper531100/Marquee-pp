#define _CRT_SECURE_NO_WARNINGS
#define _CRT_NON_CONFORMING_SWPRINTFS
#include <fstream>
#include "tools.h"

#include <iostream>
#include <thread>
#include <windows.h>
#include <sstream>

void draw_text(Font* font, DisplayConfig* param, wchar_t* screen) {
    const int& glyph_width = param->glyph_width;

    const int& y_begin = param->y_begin;
    const int& y_end = param->y_end;
    const int& y_offset = param->y_offset;

    const int& x_offset = param->x_offset;
    const int& width = param->screen_width;

    int start = 0;
    for (auto& ch : param->ws) {
        bool is_half_width = false;

        // 畫出一個字元
        for (int row = y_begin; row < y_end; row++) {
            for (int j = 0, pos = 0; j < 16; j++, pos += param->glyph_width_offset) {
                // 超出畫面寬度就不用繼續畫了
                bool in_max_range = start + pos + x_offset < width;
                bool in_min_range = start + pos + x_offset >= 0;

                if (in_max_range && in_min_range) {
                    screen[start + (row * width + pos) + x_offset] = font[ch][row - y_offset] & (0x8000 >> j) ? param->fill_char : param->background;
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

    using MemberFunctionPointer = std::function<void(std::any, DisplayConfig& config)>;
    using MethodMap = std::map<std::string, MemberFunctionPointer>;
    MethodMap method_map;
public:
    Marquee(int width, int height, wchar_t* screen, Font* font, HANDLE hConsole) :
        width(width), height(height), screen(screen), font(font), hConsole(hConsole) 
    {
        this->screen_size = width * height;

        method_map["screen_clear"] = [this](std::any args, DisplayConfig& config) {
            this->screen_clear();
        };

        method_map["marquee"] = [this](std::any args, DisplayConfig& config) {
            this->marquee(config);
        };

        method_map["slide"] = [this](std::any args, DisplayConfig& config) {
            this->slide(config);
        };

        method_map["flash"] = [this](std::any args, DisplayConfig& config) {
            this->flash(config);
        };

        method_map["delay"] = [this](std::any args, DisplayConfig& config) {
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

        switch (config.screen_clear_method) {
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
     *  @config config wstring ws          你要顯示的字串
     *  @config config int x_begin         x開始位置
     *  @config config int x_end           x截止位置
     *  @config config int x_offset        x偏移
     *  其他參數說明
     *  @config config uint64_t delay_time 休眠時間
     */
    Marquee& marquee(DisplayConfig& config) {
        DWORD dwBytesWritten;

        config.y_begin = 0;
        config.y_end = 16;
        config.y_offset = 0;

        for (int i = config.x_offset; i >= config.x_end; i--) {
            memset(screen, 0, screen_size * sizeof(wchar_t));
            config.x_offset = i;

            draw_text(font, &config, screen);
            WriteConsoleOutputCharacterW(hConsole, screen, screen_size, {0, 0}, &dwBytesWritten);

            if (i % config.step == 0) {
                delay(config.time);
            }
        }
        return *this;
    }

    /**
     *  Param所需參數說明
     *  @config config wstring ws          你要顯示的字串
     *  @config config int x_offset        x偏移
     *  其他參數說明
     *  @config config uint64_t delay_time 休眠時間
     */
    Marquee& slide(DisplayConfig& config) {
        DWORD dwBytesWritten;
        config.y_end = 16;

        for (int i = 0; i < 16; i++) {
            for (int j = 0; j < 15; j++) {
                auto region = clear_text_region(config, j, false);
                memmove(screen + (width * j) + region.first, screen + (width * (j + 1)) + region.first,
                        region.second * sizeof(wchar_t));
                memset(screen + ((15 - i) * width) + region.first, 0, region.second * sizeof(wchar_t));
            }

            /*
             * 可以方便的將每一列往前移動一行，但不能控制偏移
             * memmove(screen, screen + width, (width * 15) * sizeof(wchar_t)); 
             * memset(screen + ((15 - i) * width), 0, width * sizeof(wchar_t));
             */

            config.y_begin = 15 - i;
            config.y_offset = 15 - i;

            draw_text(font, &config, screen);

            WriteConsoleOutputCharacterW(hConsole, screen, screen_size, {0, 0}, &dwBytesWritten);

            if (i % config.step == 0) {
                delay(config.time);
            }
        }

        return *this;
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
    Marquee& flash(DisplayConfig& config) {
        DWORD dwBytesWritten;
        config.y_begin = 0;
        config.y_offset = 0;

        for (int i = 0; i < 16; i++) {
            clear_text_region(config, i);
            config.y_end = i + 1;

            draw_text(font, &config, screen);
            WriteConsoleOutputCharacterW(hConsole, screen, screen_size, {0, 0}, &dwBytesWritten);

            if (i % config.step == 0) {
                delay(config.time);
            }
        }

        return *this;
    }

    Marquee& delay(int time) {
        std::this_thread::sleep_for(std::chrono::milliseconds(time));
        return *this;
    }

    void invoke_method(const std::string& methodName, std::any args, DisplayConfig& config) {
        auto it = method_map.find(methodName);
        if (it == method_map.end()) {
            throw std::runtime_error("找不到方法: " + methodName);
        }
        it->second(args, config);
    }
};

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
        for (auto& row : json["exec"]) {
            Json effect = row["effect"];
            DisplayConfig config;

            config.ws = SaoFU::get_time(SaoFU::utf8_to_utf16(row["str"]).c_str());
            config.screen_width = width;
            config.step = effect["step"];
            config.time = effect["time"];

            DisplayConfigInitializer init(config);

            std::string begin_position = effect["begin_position"];
            std::string end_position = effect["end_position"];
            std::string clear_text = effect["clear_text_region"];

            config.x_offset = find_or_predict(init.begin_position, begin_position.c_str(), atoi);
            config.x_end = find_or_predict(init.end_position, end_position.c_str(), atoi);
            config.screen_clear_method = find_or_predict(init.clear_method, clear_text);

            for (auto& cols : row["run"]) {
                std::stringstream ss((std::string)cols);
                std::string key;
                ULONG64 value = 0;
                ss >> key >> value;
                SAOFU_TRY({ marquee.invoke_method(key, value, config); });
            }
        }
    }
}

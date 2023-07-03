#define _CRT_SECURE_NO_WARNINGS
#define _CRT_NON_CONFORMING_SWPRINTFS
#include <fstream>

#include "tools.h"

#include <iostream>
#include <thread>
#include <windows.h>

class Marquee {
public:
    bool done = false;

private:
    int width;
    int height;
    size_t screen_size;

    wchar_t* screen;
    Pack* font;
    HANDLE hConsole;

public:
    Marquee(int width, int height, wchar_t* screen, Pack* font, HANDLE hConsole) :
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
    Marquee& marquee(Param& param, const uint64_t delay_time = SaoFU::delay_step(1, 1)) {
        DWORD dwBytesWritten;

        param.y_begin = 0;
        param.y_end = 16;
        param.y_offset = 0;

        const uint32_t step = delay_time & 0xFFFFFFFF;
        const uint32_t time = delay_time >> 32;

        for (int i = param.x_offset; i >= param.x_end; i--) {
            memset(screen, 0, screen_size * sizeof(wchar_t));
            param.x_offset = i;

            SaoFU::draw_text(font, &param, screen);
            WriteConsoleOutputCharacterW(hConsole, screen, screen_size, {0, 0}, &dwBytesWritten);

            if (i % step == 0) {
                delay(time);
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
    Marquee& slide(Param& param, const uint64_t delay_time = SaoFU::delay_step(1, 1)) {
        DWORD dwBytesWritten;

        param.y_end = 16;

        const uint32_t step = delay_time & 0xFFFFFFFF;
        const uint32_t time = delay_time >> 32;

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

            if (i % step == 0) {
                delay(time);
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
    Marquee& flash(Param& param, const uint64_t delay_time = SaoFU::delay_step(1, 1)) {
        DWORD dwBytesWritten;
        param.y_begin = 0;
        param.y_offset = 0;

        const uint32_t step = delay_time & 0xFFFFFFFF;
        const uint32_t time = delay_time >> 32;

        for (int i = 0; i < 16; i++) {
            clear_text_region(param, i);
            param.y_end = i;

            SaoFU::draw_text(font, &param, screen);
            WriteConsoleOutputCharacterW(hConsole, screen, screen_size, {0, 0}, &dwBytesWritten);

            if (i % step == 0) {
                delay(time);
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
};


int main() {
    SetConsoleOutputCP(65001);

    std::ifstream t("setting.json");
    std::string str((std::istreambuf_iterator(t)), std::istreambuf_iterator<char>());

    SAOFU_TRY({
        SaoFU::g_setting = nlohmann::json::parse(str);
    });

    SaoFU::g_token = SaoFU::get_token();

    SAOFU_TRY({
        SaoFU::g_json = SaoFU::get_json(SaoFU::g_token, SaoFU::g_setting["url"]);
    });

    int i = 0;
    for(auto& row : SaoFU::g_json) {
        std::string PlateNumb = row["PlateNumb"];
        int Direction = row["Direction"];

        printf("(%02d) %-8s %s\n", i++, PlateNumb.c_str(), Direction ? u8"去程" : u8"回程");
    }

    printf(u8"選擇: ");
    std::cin >> SaoFU::g_index;

    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_FONT_INFOEX fontInfo;
    fontInfo.cbSize = sizeof(CONSOLE_FONT_INFOEX);
    GetCurrentConsoleFontEx(hConsole, FALSE, &fontInfo);

    fontInfo.dwFontSize.Y = 14; // 设置字体的高度

    SetCurrentConsoleFontEx(hConsole, FALSE, &fontInfo);

    RECT rect = { 0, 0, 16 * 14 * 8 + 48, 32 * 7 + 64 };
    MoveWindow(GetConsoleWindow(), rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, TRUE);


    std::string font_path = SaoFU::g_setting["font_path"];
    FILE* fp = fopen(font_path.c_str(), "rb+");
    fseek(fp, 0L, SEEK_END);
    int size = ftell(fp);

    char* buffer = new char[size];
    fseek(fp, 0L, SEEK_SET);
    fread(buffer, size, 1, fp);
    fclose(fp);
    Pack* pack = (Pack*)buffer;

    CONSOLE_SCREEN_BUFFER_INFO console_buffer_info = {0};
    GetConsoleScreenBufferInfo(hConsole, &console_buffer_info);

    int width = console_buffer_info.srWindow.Right - console_buffer_info.srWindow.Left + 1;
    int height = console_buffer_info.srWindow.Bottom - console_buffer_info.srWindow.Top;
    int screen_size = width * height;

    wchar_t* screen = new wchar_t[screen_size];
    Marquee marquee(width, height, screen, pack, hConsole);

    marquee.screen_clear();
    std::thread(SaoFU::listenForKeyboardEvents).detach();

    while (1) {
        while (1) {
            wchar_t wbuf[80];
            SaoFU::get_time(wbuf, L"%H:%M");

            Param param2;
            param2.screen_width = width;
            param2.ws = wbuf;
            param2.x_offset = (width / 2) - 36;
            param2.screen_clear_method = SaoFU::utils::TextClearMethod::ClearAllText;

            if (marquee.flash(param2).long_delay(150).done) {
                break;
            }

            param2.ws = L"歡迎搭乘台中市公車";
            param2.x_offset = width;
            param2.x_end = -SaoFU::count_size(param2.ws);

            if (marquee.marquee(param2).delay(100).done) {
                break;
            }
        }

        std::wstring chinese = SaoFU::utf8_to_utf16(SaoFU::g_json[SaoFU::g_index]["StopName"]["Zh_tw"]);
        std::wstring english = SaoFU::utf8_to_utf16(SaoFU::g_json[SaoFU::g_index]["StopName"]["En"]);

        marquee.screen_clear().jump_to_here();

        Param param;
        param.screen_width = width;
        param.ws = L"下一站";
        param.x_offset = (width / 2) - 48;
        param.screen_clear_method = SaoFU::utils::TextClearMethod::ClearAll;

        marquee.slide(param).long_delay(65).screen_clear();

        if (SaoFU::count_size(chinese) < width && SaoFU::count_size(english) < width) {
            param.ws = chinese;
            param.x_offset = 0;
            marquee.slide(param).long_delay(65);

            param.ws = english;
            marquee.slide(param).long_delay(65);

            param.ws = chinese;
            marquee.slide(param).long_delay(500);
        }
        else if (SaoFU::count_size(chinese) < width) {
            param.ws = chinese;
            param.x_offset = 0;

            marquee.slide(param).long_delay(65);

            param.ws = chinese + SaoFU::count_space(SaoFU::count_size(chinese), width) + english + L' ';
            param.screen_width = width;
            param.x_offset = 0;
            param.x_end = -SaoFU::count_size(param.ws);
            param.ws = param.ws + chinese;

            marquee.marquee(param).long_delay(500);
        }
        else {
            param.x_offset = (width / 2) - 48;
            param.ws = L"下一站" + SaoFU::count_space(param.x_offset + SaoFU::count_size(L"下一站"), width) + chinese +
                L'  ' + english + L'  ' + chinese;
            param.x_end = -SaoFU::count_size(param.ws);

            marquee.marquee(param);
        }
    }
}

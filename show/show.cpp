#define _CRT_SECURE_NO_WARNINGS
#define _CRT_NON_CONFORMING_SWPRINTFS
#include "tools.h"

#include <iostream>
#include <thread>
#include <windows.h>

class Marquee : Param {
public:
    using super = Param;
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
        width(width), height(height), screen(screen), font(font), hConsole(hConsole)
    {
        this->screen_size = width * height;
        super::screen_width = width;
    }

    Marquee& screen_clear() {
        DWORD dwBytesWritten;
        memset(screen, 0, screen_size * sizeof(wchar_t));
        WriteConsoleOutputCharacterW(hConsole, screen, screen_size, { 0, 0 }, &dwBytesWritten);
        return *this;
    }

    Marquee& screen_clear(Param* param) {
        switch (param->screen_clear_method) {
        case SaoFU::utils::TextClearMethod::ClearAllText:
            param->x_begin = 0;
            param->x_end = param->screen_width;
            break;
        case SaoFU::utils::TextClearMethod::ClearTextItself:
            param->x_begin = param->x_offset;
            param->x_end = SaoFU::count_size(param->ws);
            break;
        case SaoFU::utils::TextClearMethod::ClearTextBefore:
            param->x_begin = 0;
            param->x_end = SaoFU::count_size(param->ws) + param->x_offset;
            break;
        case SaoFU::utils::TextClearMethod::ClearTextAfter:
            param->x_begin = param->x_offset;
            param->x_end = param->screen_width - SaoFU::count_size(param->ws);
            break;
        }
        return *this;
    }

    //TODO 刷新偏移
    /*
     * ws
     * x_offset
     * x_end
     * delay_time
     */
    Marquee& marquee(Param& param, const uint64_t delay_time = SaoFU::delay_step(1, 1))
    {
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
            WriteConsoleOutputCharacterW(hConsole, screen, screen_size, { 0, 0 }, &dwBytesWritten);

            if (i % step == 0) {
                delay(time);
            }

            if (done) {
                return *this;
            }
        }
        return *this;
    }

    Marquee& slide(const std::wstring ws, const int x_offset = 0, const uint64_t delay_time = SaoFU::delay_step(1, 10)) {
        DWORD dwBytesWritten;

        super::ws = ws;
        super::x_offset = x_offset;
        super::y_end = 16;

        const uint32_t step = delay_time & 0xFFFFFFFF;
        const uint32_t time = delay_time >> 32;

        for (int i = 16; i >= 0; i--) {
            super::y_begin = i;
            super::y_offset = i;

            SaoFU::draw_text(font, this, screen);
            WriteConsoleOutputCharacterW(hConsole, screen, screen_size, { 0, 0 }, &dwBytesWritten);

            if (i % step == 0) {
                delay(time);
            }

            if (done) {
                return *this;
            }
        }

        return *this;
    }

    Marquee& slide2(const std::wstring ws, const int x_offset = 0, const uint64_t delay_time = SaoFU::delay_step(1, 1)) {
        DWORD dwBytesWritten;
        std::wstring old = super::ws;
        int old_x_offset = super::x_offset;

        const uint32_t step = delay_time & 0xFFFFFFFF;
        const uint32_t time = delay_time >> 32;

        super::y_offset = 0;

        for (int i = 0; i <= 16; i++) {
            super::ws = old;
            super::x_offset = old_x_offset;

            super::y_begin = 0;
            super::y_end = 17 - i;
            super::y_offset = -i;
            SaoFU::draw_text(font, this, screen);

            super::ws = ws;
            super::x_offset = x_offset;

            super::y_begin = super::y_end - 1;
            super::y_end = 16;
            super::y_offset = 16 - i;
            SaoFU::draw_text(font, this, screen);

            WriteConsoleOutputCharacterW(hConsole, screen, screen_size, { 0, 0 }, &dwBytesWritten);

            if (i % step == 0) {
                delay(time);
            }

            if (done) {
                return *this;
            }
        }

        return *this;
    }

    /*
     *  所需參數說明
     *  wstring ws          你要顯示的字串
     *  int x_begin         x開始位置 
     *  int x_end           x截止位置
     *  int x_offset        x偏移 
     *  uint64_t delay_time 休眠時間
     */
    Marquee& flash(Param &param, const uint64_t delay_time = SaoFU::delay_step(1, 1)) {
        DWORD dwBytesWritten;
        param.y_begin = 0;
        param.y_offset = 0;

        const uint32_t step = delay_time & 0xFFFFFFFF;
        const uint32_t time = delay_time >> 32;

        screen_clear(&param);

        for (int i = 0; i <= 16; i++) {
            memset(screen + (width * i + param.x_begin), 0, param.x_end * sizeof(wchar_t));
            param.y_end = i;

            SaoFU::draw_text(font, &param, screen);
            WriteConsoleOutputCharacterW(hConsole, screen, screen_size, { 0, 0 }, &dwBytesWritten);

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
        if (SaoFU::trigger == true || this->done == true) {
            this->done = true;
            SaoFU::trigger = 0;
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
    RECT rect = { 0, 0, 1920, 320 };
    MoveWindow(GetConsoleWindow(), rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, TRUE);

    int null;
    //std::cin >> null;

    FILE* fp = fopen("mingliu7.03/mingliu_Fixedsys_Excelsior.bin", "rb+");
    fseek(fp, 0L, SEEK_END);
    int size = ftell(fp);

    char* buffer = new char[size];
    fseek(fp, 0L, SEEK_SET);
    fread(buffer, size, 1, fp);
    fclose(fp);
    Pack* pack = (Pack*)buffer;

    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO console_buffer_info = { 0 };
    GetConsoleScreenBufferInfo(hConsole, &console_buffer_info);

    int width = console_buffer_info.srWindow.Right - console_buffer_info.srWindow.Left + 1;
    int height = console_buffer_info.srWindow.Bottom - console_buffer_info.srWindow.Top;
    int screen_size = width * height;

    wchar_t* screen = new wchar_t[screen_size];
    Marquee marquee(width, height, screen, pack, hConsole);

    marquee.screen_clear();
    std::thread(SaoFU::listenForKeyboardEvents).detach();

    wchar_t wbuf[80];
    while (1) {
        while (1) {
            std::wstring title = L"歡迎搭乘台中市公車";

            Param param2;
            param2.screen_width = width;
            param2.ws = title;
            param2.x_offset = width;
            param2.x_end = -SaoFU::count_size(title);

            if (marquee.marquee(param2).delay(100).done) {
                break;
            }
        }

        std::wstring chinese = L"台中科技大學";
        std::wstring english = L"Chongde Bridge";

        marquee.screen_clear().jump_to_here();

        marquee.slide(L"下一站", (width / 2) - 48)
            .delay(800)
            .screen_clear();

        marquee.slide(chinese, 0)
            .delay(800);
    }
}
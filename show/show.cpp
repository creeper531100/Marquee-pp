#define _CRT_SECURE_NO_WARNINGS
#define _CRT_NON_CONFORMING_SWPRINTFS

#include <iostream>
#include <string>
#include <windows.h>
#include <chrono>
#include <thread>

struct Pack {
    uint16_t font[16];
};

struct Param {
    int x_offset;
    int y_offset;
    int screen_width;
    std::wstring ws;
};

void draw_text(Pack* font, Param* param, wchar_t* screen) {
    constexpr int char_height = 16;
    constexpr int char_width = 32;
    constexpr int width_offset = 2;
    constexpr wchar_t fill_char = L'█';
    constexpr wchar_t background = L' ';

    int start = 0;

    for (int i = 0; i < param->ws.length(); i++) {
        bool is_half_width = false;
        wchar_t ch = param->ws[i];

        // 畫出一個字元
        for (int row = param->y_offset; row < char_height; row++) {
            for (int col = 0; col < char_width; col += width_offset) {
                // 超出畫面寬度就不用繼續畫了
                if (param->x_offset + (start + col) >= param->screen_width) {
                    break;
                }

                bool b = font[ch].font[row - param->y_offset] & 0x8000 >> col / 2;
                if (b && param->x_offset + col + start >= 0) {
                    screen[param->x_offset + start + (row * param->screen_width + col)] = fill_char;
                }
                else {
                    screen[param->x_offset + start + (row * param->screen_width + col)] = background;
                }
            }
        }

        for (int row = 0; row < char_height; row++) {
            // 判斷是否為半形字元(字元另一半是空的)
            is_half_width |= font[ch].font[row] & 0xFF;
        }

        // 計算下一個字元的起始位置
        start += (is_half_width ? 32 : 16);
    }
}

class Marquee : public Param {
public:
    using super = Param;

public:
    int width;
    int height;
    size_t screen_size;

    wchar_t* screen;
    Pack* font;
    HANDLE hConsole;
public:
    Marquee(int width, int height, wchar_t* screen, Pack* font, HANDLE hConsole) : width(width), height(height),
        screen(screen), font(font), hConsole(hConsole)
    {
        this->screen_size =  width * height;
        super::screen_width = width;
    }

    Marquee& screen_clear() {
        DWORD dwBytesWritten;
        memset(screen, 0, screen_size * sizeof(wchar_t));
        WriteConsoleOutputCharacterW(hConsole, screen, screen_size, { 0, 0 }, &dwBytesWritten);
        return *this;
    }

    Marquee& marquee(std::wstring ws, int len = 32) {
        DWORD dwBytesWritten;

        for (int i = width; i >= -len * (signed)ws.length(); i--) {
            memset(screen, 0, screen_size * sizeof(wchar_t));

            super::x_offset = i;
            super::y_offset = 0;
            super::ws = ws;

            draw_text(font, this, screen);
            WriteConsoleOutputCharacterW(hConsole, screen, screen_size, {0, 0}, &dwBytesWritten);
            std::this_thread::sleep_for(std::chrono::microseconds(10));
        }

        return *this;
    }

    Marquee& slide(std::wstring ws, int x_offset = 0) {
        DWORD dwBytesWritten;

        for (int i = 16; i >= 0; i--) {

            super::x_offset = x_offset;
            super::y_offset = i;
            super::ws = ws;

            draw_text(font, this, screen);
            WriteConsoleOutputCharacterW(hConsole, screen, screen_size, {0, 0}, &dwBytesWritten);
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }

        return *this;
    }

    Marquee& delay(int time) {
        std::this_thread::sleep_for(std::chrono::milliseconds(time));
        return *this;
    }
};

int main() {
    RECT rect = { 0, 0, 1920, 320 };
    MoveWindow(GetConsoleWindow(), rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, TRUE);

    int null;
    std::cin >> null;

    FILE* fp = fopen("mingliu_16x16.bin", "rb+");
    fseek(fp, 0L, SEEK_END);
    int size = ftell(fp);

    char* buffer = new char[size];
    fseek(fp, 0L, SEEK_SET);
    fread(buffer, size, 1, fp);
    Pack* pack = (Pack*)buffer;

    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO console_buffer_info = {0};
    GetConsoleScreenBufferInfo(hConsole, &console_buffer_info);

    int width = console_buffer_info.srWindow.Right - console_buffer_info.srWindow.Left + 1;
    int height = console_buffer_info.srWindow.Bottom - console_buffer_info.srWindow.Top;
    int screen_size = width * height;

    wchar_t* screen = new wchar_t[screen_size];
    Marquee marquee(width, height, screen, pack, hConsole);

    while (1) {
        time_t     now = time(0);
        struct tm  tstruct;
        wchar_t    buf[80];
        tstruct = *localtime(&now);
        wcsftime(buf, sizeof(buf), L"%H:%M:%S", &tstruct);

        marquee.screen_clear()
               .marquee(L"歡迎搭乘 豐原客運 55 路線 " + std::wstring(buf));

        marquee.screen_clear()
            .marquee(L"歡迎搭乘 豐原客運 55 路線 " + std::wstring(buf), 0);

        marquee.screen_clear()
            .marquee(L"55 路線 " + std::wstring(buf), -15);

        marquee.screen_clear()
            .marquee(buf, -25);

        marquee.screen_clear()
            .marquee(L"下一站: 崇德橋  Next: Chongde Bridge");
    }
}

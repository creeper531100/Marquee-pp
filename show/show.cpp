#define _CRT_SECURE_NO_WARNINGS
#define _CRT_NON_CONFORMING_SWPRINTFS

#include <bitset>
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

    for(int i = 0; i < param->ws.length(); i++) {
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
                if(b && param->x_offset + col + start >= 0) {
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

void marquee(Pack* font, HANDLE hConsole, int width, wchar_t* screen, size_t screen_size, std::wstring ws) {
    Param param{};
    DWORD dwBytesWritten;

    for (int i = width; i >= -32 * (signed)ws.length(); i--) {
        memset(screen, 0, screen_size * sizeof(wchar_t));
        param = { i, 0, width, ws };
        draw_text(font, &param, screen);
        WriteConsoleOutputCharacterW(hConsole, screen, screen_size, { 0, 0 }, &dwBytesWritten);
        std::this_thread::sleep_for(std::chrono::microseconds(10));
    }
}

void slide(Pack* font, HANDLE hConsole, int width, wchar_t* screen, size_t screen_size, std::wstring ws) {
    Param param{};
    DWORD dwBytesWritten;

    for (int i = 16; i >= 0; i--) {
        param = { 0, i, width, ws };
        draw_text(font, &param, screen);
        WriteConsoleOutputCharacterW(hConsole, screen, screen_size, { 0, 0 }, &dwBytesWritten);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

int main() {
    FILE* fp = fopen("unicode_16x16.bin", "rb+");
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
    DWORD dwBytesWritten = 0;
    Param param{};
    while (1) {
        memset(screen, 0, screen_size * sizeof(wchar_t));

        time_t     now = time(0);
        struct tm  tstruct;
        wchar_t    buf[80];
        tstruct = *localtime(&now);
        wcsftime(buf, sizeof(buf), L"%H:%M:%S", &tstruct);

        std::wstring ws = L"歡迎搭乘 豐原客運 55 路線 " + (std::wstring)buf;
        marquee(pack, hConsole, width, screen, screen_size, ws);

        slide(pack, hConsole, width, screen, screen_size, L"下一站");
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        memset(screen, 0, screen_size * sizeof(wchar_t));

        slide(pack, hConsole, width, screen, screen_size, L"崇德橋");
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        memset(screen, 0, screen_size * sizeof(wchar_t));

        slide(pack, hConsole, width, screen, screen_size, L"Bridge");
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        memset(screen, 0, screen_size * sizeof(wchar_t));

        slide(pack, hConsole, width, screen, screen_size, L"崇德橋");
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
}
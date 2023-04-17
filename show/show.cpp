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
    std::wstring ws; //字串
    int screen_width; //緩衝區大小

    int x_offset = 0; //文字在螢幕上的起始x座標
    int y_offset = 0; //文字在螢幕上的起始y座標

    int glyph_width_factor = 2; //字形寬度的倍數(數值越小越窄)
    int glyph_width_offset = 2; //字形寬度的偏移量

    wchar_t fill_char = L'█'; //文字填滿部分的符號
    wchar_t background = L' '; //文字未填滿部分的背景符號

    enum class Effect {
        FIASH,
        DEFAULT
    } effect;
};

void draw_text(Pack* font, Param* param, wchar_t* screen) {
    constexpr int glyph_height = 16;
    const int glyph_width = 16 * param->glyph_width_factor;

    int start = 0;

    for (int i = 0; i < param->ws.length(); i++) {
        bool is_half_width = false;
        wchar_t ch = param->ws[i];

        int row_begin = param->y_offset;
        int row_end = glyph_height;
        int row_count = param->y_offset;

        if(param->effect == Param::Effect::FIASH) {
            row_begin = 0;
            row_end = param->y_offset;
            row_count = 0;
        }

        // 畫出一個字元
        for (int row = row_begin; row < row_end; row++) {
            for (int col = 0; col < glyph_width; col += param->glyph_width_offset) {
                // 超出畫面寬度就不用繼續畫了
                if (param->x_offset + (start + col) >= param->screen_width) {
                    break;
                }

                bool b = font[ch].font[row - row_count] & 0x8000 >> col / param->glyph_width_factor;
                if (b && param->x_offset + col + start >= 0) {
                    screen[param->x_offset + start + (row * param->screen_width + col)] = param->fill_char;
                }
                else {
                    screen[param->x_offset + start + (row * param->screen_width + col)] = param->background;
                }
            }
        }

        for (int row = 0; row < glyph_height; row++) {
            // 判斷是否為半形字元(字元另一半是空的)
            is_half_width |= font[ch].font[row] & 0xFF;
        }

        // 計算下一個字元的起始位置
        start += (is_half_width ? glyph_width : glyph_width / 2);
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

    Marquee& marquee(std::wstring ws, int x_offset, int len) {
        DWORD dwBytesWritten;

        for (int i = x_offset; i >= len; i--) {
            memset(screen, 0, screen_size * sizeof(wchar_t));

            super::x_offset = i;
            super::y_offset = 0;
            super::ws = ws;
            super::effect = Effect::DEFAULT;

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
            super::effect = Effect::DEFAULT;

            draw_text(font, this, screen);
            WriteConsoleOutputCharacterW(hConsole, screen, screen_size, {0, 0}, &dwBytesWritten);
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }

        return *this;
    }

    Marquee& flash(std::wstring ws, int x_offset = 0) {
        DWORD dwBytesWritten;

        for (int i = 0; i <= 16; i++) {
            super::x_offset = x_offset;
            super::y_offset = i;
            super::ws = ws;
            super::effect = Effect::FIASH;

            draw_text(font, this, screen);
            WriteConsoleOutputCharacterW(hConsole, screen, screen_size, { 0, 0 }, &dwBytesWritten);
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }

        return *this;
    }

    Marquee& delay(int time) {
        std::this_thread::sleep_for(std::chrono::milliseconds(time));
        return *this;
    }
};

int count_size(std::wstring show) {
    int end = 0;
    for (int i = 0; i < show.length(); i++) {
        end += show[i] < 256 ? 16 : 32;
    }
    return end;
}

int main() {
    RECT rect = { 0, 0, 1920, 320 };
    MoveWindow(GetConsoleWindow(), rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, TRUE);

    int null;

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
    Marquee marquee(width, height, screen, pack, hConsole);
    marquee.screen_clear();

    while (1) {
        time_t     now = time(0);
        struct tm  tstruct;
        wchar_t    buf[80];
        tstruct = *localtime(&now);
        wcsftime(buf, sizeof(buf), L"%H:%M", &tstruct);

        std::wstring title = L"歡迎搭乘台中市公車";
        marquee.marquee(title, width, -count_size(title));

        marquee.slide(L"下一站", (width / 2) - 48)
               .delay(1000)
               .screen_clear()
               .delay(100);

        std::wstring ws = L"文化新村";
        int begin = (width / 2) - (ws.length() * 16);
        marquee.slide(ws, begin)
               .delay(1000);

        std::wstring english = L"Cultural Community";
        std::wstring show = ws + std::wstring(begin / 16, L' ') + english + L"  ";

        marquee.marquee(show + ws, begin, -count_size(show))
               .delay(2000);

        marquee.flash(std::wstring(16 * 8, L' '), 0);

        marquee.flash(buf, (width / 2) - 36)
            .delay(1000)
            .screen_clear()
            .delay(100);


        /*marquee.glyph_width_factor = 2;
        marquee.glyph_width_offset = 1;

        marquee.flash(L"12 雙 十 公 車 ").delay(700);

        marquee.glyph_width_factor = 1;
        marquee.glyph_width_offset = 1;

        marquee.flash(L"  明德高中 - 豐原高中 ", 44).delay(2500);

        marquee.glyph_width_factor = 1;
        marquee.glyph_width_offset = 1;

        marquee.flash(L"豐原火車站 - 洲際棒球場", 44).delay(2500);

        marquee.glyph_width_factor = 1;
        marquee.glyph_width_offset = 1;

        marquee.flash(L"台中火車站 - 一中商圈 ", 44).delay(2500);

        marquee.glyph_width_factor = 2;
        marquee.glyph_width_offset = 1;
        marquee.flash(L" 開車不超速 ", 44).delay(700);*/
    }
}

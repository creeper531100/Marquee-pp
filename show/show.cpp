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

    int row_begin = 0;
    int row_end = 16;
    int row_count = 0;
};

bool trigger = false;
void listenForKeyboardEvents() {
    while (true) {
        trigger = GetAsyncKeyState(VK_SPACE);
        std::this_thread::sleep_for(std::chrono::milliseconds(800));
    }
}

void draw_text(Pack* font, Param* param, wchar_t* screen) {
    constexpr int glyph_height = 16;
    const int glyph_width = 16 * param->glyph_width_factor;

    int start = 0;

    for (int i = 0; i < param->ws.length(); i++) {
        bool is_half_width = false;
        wchar_t& ch = param->ws[i];

        int& row_begin = param->row_begin;
        int& row_end = param->row_end;
        int& row_count = param->row_count;

        // 畫出一個字元
        for (int row = row_begin; row < row_end; row++) {
            for (int col = 0; col < glyph_width; col += param->glyph_width_offset) {
                // 超出畫面寬度就不用繼續畫了
                if (param->x_offset + (start + col) >= param->screen_width) {
                    break;
                }

                //TODO
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
    bool done = false;

    wchar_t* screen;
    Pack* font;
    HANDLE hConsole;
public:
    Marquee(int width, int height, wchar_t* screen, Pack* font, HANDLE hConsole) : width(width), height(height),
        screen(screen), font(font), hConsole(hConsole)
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

    Marquee& marquee(std::wstring ws, int x_offset, int len) {
        DWORD dwBytesWritten;
        super::y_offset = 0;
        super::ws = ws;

        super::row_begin = 0;
        super::row_end = 16;
        super::row_count = 0;

        for (int i = x_offset; i >= len; i--) {
            memset(screen, 0, screen_size * sizeof(wchar_t));
            super::x_offset = i;

            draw_text(font, this, screen);
            WriteConsoleOutputCharacterW(hConsole, screen, screen_size, { 0, 0 }, &dwBytesWritten);

            if (delay(10).done) {
                return *this;
            }
        }

        return *this;
    }

    Marquee& slide(std::wstring ws, int x_offset = 0) {
        DWORD dwBytesWritten;

        super::ws = ws;
        super::x_offset = x_offset;
        super::row_end = 16;

        for (int i = 16; i >= 0; i--) {
            super::y_offset = i;
            super::row_begin = i;
            super::row_count = i;

            draw_text(font, this, screen);
            WriteConsoleOutputCharacterW(hConsole, screen, screen_size, { 0, 0 }, &dwBytesWritten);

            if (delay(10).done) {
                return *this;
            }
        }

        return *this;
    }

    Marquee& slide2(std::wstring ws, int x_offset = 0) {
        DWORD dwBytesWritten;
        std::wstring old = super::ws;
        int old_x_offset = super::x_offset;

        super::row_count = 0;

        for (int i = 0; i <= 16; i++) {
            super::ws = old;
            super::y_offset = i;
            super::row_count = -i;
            super::row_begin = 0;
            super::row_end = 17 - i;
            super::x_offset = old_x_offset;
            draw_text(font, this, screen);

            super::y_offset = 16 - i;
            super::row_count = 16 - i;
            super::row_begin = super::row_end;
            super::row_end = 16;
            super::ws = ws;
            super::x_offset = x_offset;
            draw_text(font, this, screen);

            WriteConsoleOutputCharacterW(hConsole, screen, screen_size, { 0, 0 }, &dwBytesWritten);

            if (delay(1).done) {
                return *this;
            }
        }

        return *this;
    }

    Marquee& flash(std::wstring ws, int x_offset = 0) {
        DWORD dwBytesWritten;
        super::ws = ws;
        super::x_offset = x_offset;

        super::row_begin = 0;
        super::row_count = 0;

        for (int i = 0; i <= 16; i++) {
            super::y_offset = i;
            super::row_end = i;

            draw_text(font, this, screen);
            WriteConsoleOutputCharacterW(hConsole, screen, screen_size, { 0, 0 }, &dwBytesWritten);

            if(delay(1).done) {
                return *this;
            }
        }

        return *this;
    }

    Marquee& delay(int time) {
        if (trigger == true || this->done == true) {
            this->done = true;
            trigger = 0;
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
    std::cin >> null;

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
    std::thread(listenForKeyboardEvents).detach();

    wchar_t wbuf[80];
    while (1) {
        while(1) {
            time_t     now = time(0);
            tm  tstruct;
            char       buf[80];
            tstruct = *localtime(&now);
            strftime(buf, sizeof(buf), "%S", &tstruct);
            wcsftime(wbuf, sizeof(buf), L"%H:%M", &tstruct);

            std::wstring title = L"歡迎搭乘大順客運700路線";
            if (marquee.marquee(title, width, -count_size(title)).done){
                break;
            }

            if (marquee.flash(wbuf, (width / 2) - 36).long_delay(150).done) {
                break;
            }
        }

        marquee.screen_clear().jump_to_here();

        marquee.slide(L"下一站", (width / 2) - 48)
            .delay(800)
            .screen_clear()
            .delay(100);

        std::wstring chinese = L"黎明福科路口";
        std::wstring english = L"Liming-Fuke Intersection";

        int begin = (width / 2) - count_size(chinese) / 2;
        marquee.slide(chinese, begin)
            .long_delay(60)
            .delay(100);

        int a = (9 - chinese.length());
        std::wstring ans = chinese + std::wstring(a, L' ') + english;
        marquee.marquee(ans + std::wstring(a, L' ') + chinese, begin, -count_size(ans + std::wstring(a, L' ')))
               .long_delay(250);

        marquee.slide2(wbuf, (width / 2) - 36)
               .long_delay(60);

        marquee.flash(std::wstring(5, L' '), (width / 2) - 36);
    }
}
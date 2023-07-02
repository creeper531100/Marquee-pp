#include "tools.h"

#include <string>
#include <thread>
#include <windows.h>

namespace SaoFU {
    void listenForKeyboardEvents() {
        while (true) {
            trigger = GetAsyncKeyState(VK_SPACE);
            std::this_thread::sleep_for(std::chrono::milliseconds(800));
        }
    }

    std::wstring count_space(int count_size, int width) {
        return std::wstring(((width - count_size) / 16) + 1, L' ');
    }

    void draw_text(Pack* font, Param* param, wchar_t* screen) {
        constexpr int glyph_height = 16;
        const int glyph_width = 16 * param->glyph_width_factor;

        int start = 0;

        for (int i = 0; i < param->ws.length(); i++) {
            bool is_half_width = false;
            wchar_t& ch = param->ws[i];

            int& row_begin = param->y_begin;
            int& row_end = param->y_end;
            int& row_count = param->y_offset;

            // �e�X�@�Ӧr��
            for (int row = row_begin; row < row_end; row++) {
                for (int col = 0; col < glyph_width; col += param->glyph_width_offset) {
                    // �W�X�e���e�״N�����~��e�F
                    if (param->x_offset + (start + col) >= param->screen_width) {
                        break;
                    }

                    //TODO
                    bool b = font[ch][row - row_count] & 0x8000 >> col / param->glyph_width_factor;
                    if (b && param->x_offset + col + start >= 0) {
                        screen[param->x_offset + start + (row * param->screen_width + col)] = param->fill_char;
                    }
                    else {
                        screen[param->x_offset + start + (row * param->screen_width + col)] = param->background;
                    }
                }
            }

            for (int row = 0; row < glyph_height; row++) {
                // �P�_�O�_���b�Φr��(�r���t�@�b�O�Ū�)
                is_half_width |= font[ch][row] & 0xFF;
            }

            // �p��U�@�Ӧr�����_�l��m
            start += (is_half_width ? glyph_width : glyph_width / 2);
        }
    }
}

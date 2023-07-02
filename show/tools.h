#pragma once

#include <cstdint>
#include <string>

using Pack = uint16_t[16];
struct Param;

namespace SaoFU {
    inline bool trigger = false;
    void listenForKeyboardEvents();
    std::wstring count_space(int count_size, int width);
    void draw_text(Pack* font, Param* param, wchar_t* screen);

    constexpr int count_size(std::wstring_view show) {
        int end = 0;
        for (int i = 0; i < show.length(); i++) {
            end += show[i] < 256 ? 16 : 32;
        }
        return end;
    }

    constexpr uint64_t delay_step(uint32_t step, uint32_t delay) {
        return (uint64_t)delay << 32 | step;
    }
}

namespace SaoFU::utils {
    enum class TextClearMethod {
        None,
        ClearAllText,
        ClearTextItself,
        ClearTextBefore,
        ClearTextAfter
    };
}


struct Param {
    std::wstring ws; //字串
    int screen_width; //緩衝區大小

    int x_begin = 0; //文字在螢幕上起始x座標
    int x_end = screen_width; //文字在螢幕上截止x座標
    int x_offset = 0; //文字在螢幕x偏移位置

    int glyph_width_factor = 2; //字形寬度的倍數(數值越小越窄)
    int glyph_width_offset = 2; //字形寬度的偏移量

    wchar_t fill_char = L'█'; //文字填滿部分的符號
    wchar_t background = L' '; //文字未填滿部分的背景符號

    int y_begin = 0; //文字在螢幕上起始y座標
    int y_end = 16; //文字在螢幕上截止y座標
    int y_offset = 0; //文字在螢幕y偏移位置

    SaoFU::utils::TextClearMethod screen_clear_method = SaoFU::utils::TextClearMethod::ClearAllText;
};

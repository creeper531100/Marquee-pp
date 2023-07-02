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
    std::wstring ws; //�r��
    int screen_width; //�w�İϤj�p

    int x_begin = 0; //��r�b�ù��W�_�lx�y��
    int x_end = screen_width; //��r�b�ù��W�I��x�y��
    int x_offset = 0; //��r�b�ù�x������m

    int glyph_width_factor = 2; //�r�μe�ת�����(�ƭȶV�p�V��)
    int glyph_width_offset = 2; //�r�μe�ת������q

    wchar_t fill_char = L'�i'; //��r�񺡳������Ÿ�
    wchar_t background = L' '; //��r���񺡳������I���Ÿ�

    int y_begin = 0; //��r�b�ù��W�_�ly�y��
    int y_end = 16; //��r�b�ù��W�I��y�y��
    int y_offset = 0; //��r�b�ù�y������m

    SaoFU::utils::TextClearMethod screen_clear_method = SaoFU::utils::TextClearMethod::ClearAllText;
};

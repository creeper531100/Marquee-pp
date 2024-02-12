#pragma once
#define _CRT_SECURE_NO_WARNINGS

#include <cstdint>
#include <string>
#include <nlohmann/json.hpp>

using Font = uint16_t[16];
struct Param;

#define SAOFU_EXCEPTION(hr) SaoFU::e_what(__LINE__, __FILE__, hr)

#define SAOFU_TRY(fn) try fn catch(std::exception &e) { SaoFU::e_what(__LINE__, e.what(), 2); }

namespace SaoFU {
    std::wstring count_space(int count_size, int width);
    void draw_text(Font* font, Param* param, wchar_t* screen);

    std::wstring get_time(const wchar_t* fmt);
    std::wstring utf8_to_utf16(const std::string& str);
    std::string utf16_to_utf8(const std::wstring& utf16String);

    constexpr int count_size(std::wstring_view show) {
        int end = 0;
        for (int i = 0; i < show.length(); i++) {
            end += show[i] < 256 ? 16 : 32;
        }
        return end;
    }
}

namespace SaoFU::utils {
    /**
     * ��ܤ�r�M������k���T�|�����C
     */
    enum TextClearMethod {
        None,                 // ���M����r
        ClearAll,             // �M��������r
        ClearAllText,         // �M���Ҧ���r
        ClearTextItself,      // �M����r����
        ClearTextBefore,      // �M����r���e������
        ClearTextAfter        // �M����r���᪺����
    };
}

struct Param {
    std::wstring ws; //�r��
    int screen_width; //�w�İϤj�p

    int x_begin = 0; //��r�b�ù��W�_�lx�y��
    int x_end = screen_width; //��r�b�ù��W�I��x�y��
    int x_offset = 0; //��r�b�ù�x������m

    int glyph_height = 16; //�r�Ϊ�����(���e16�B�b�e16)
    int glyph_width = 32; //�r�Ϊ��e��(���e32�B�b�e16)
    int glyph_width_offset = 2; //�r�μe�ת������q

    wchar_t fill_char = L'�i'; //��r�񺡳������Ÿ�
    wchar_t background = L' '; //��r���񺡳������I���Ÿ�

    int y_begin = 0; //��r�b�ù��W�_�ly�y��
    int y_end = 16; //��r�b�ù��W�I��y�y��
    int y_offset = 0; //��r�b�ù�y������m

    int step = 1;
    int time = 1;

    int screen_clear_method = SaoFU::utils::TextClearMethod::ClearAllText;
};



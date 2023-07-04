#pragma once
#define _CRT_SECURE_NO_WARNINGS

#include <cstdint>
#include <string>
#include <nlohmann/json.hpp>

class CUrlHandle;
using Font = uint16_t[16];
struct Param;

#define SAOFU_EXCEPTION(hr) SaoFU::e_what(__LINE__, __FILE__, hr)

#define SAOFU_TRY(fn) try fn catch(std::exception &e) { SaoFU::e_what(__LINE__, e.what(), 2); }

namespace SaoFU {
    inline bool g_trigger = false;
    inline nlohmann::json g_setting;
}

namespace SaoFU {
    int query_plate_numb(nlohmann::json& json, std::string plate_numb);
    nlohmann::json query_stops(nlohmann::json& DisplayStopOfRouteUrl, std::string RouteName, int Direction);

    void listenForKeyboardEvents(std::string token, std::string plate_numb, nlohmann::json& json);
    std::wstring count_space(int count_size, int width);
    void draw_text(Font* font, Param* param, wchar_t* screen);

    void get_time(wchar_t* wbuf, const wchar_t* fmt);
    std::wstring utf8_to_utf16(const std::string& str);

    std::string get_token();
    std::string get_data(std::string token, std::string url);

    long e_what(int line, const char* file, long hr);

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

    int screen_clear_method = SaoFU::utils::TextClearMethod::ClearAllText;
};



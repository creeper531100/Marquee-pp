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
     * 表示文字清除的方法的枚舉類型。
     */
    enum TextClearMethod {
        None,                 // 不清除文字
        ClearAll,             // 清除全部文字
        ClearAllText,         // 清除所有文字
        ClearTextItself,      // 清除文字本身
        ClearTextBefore,      // 清除文字之前的部分
        ClearTextAfter        // 清除文字之後的部分
    };
}

struct Param {
    std::wstring ws; //字串
    int screen_width; //緩衝區大小

    int x_begin = 0; //文字在螢幕上起始x座標
    int x_end = screen_width; //文字在螢幕上截止x座標
    int x_offset = 0; //文字在螢幕x偏移位置

    int glyph_height = 16; //字形的高度(全寬16、半寬16)
    int glyph_width = 32; //字形的寬度(全寬32、半寬16)
    int glyph_width_offset = 2; //字形寬度的偏移量

    wchar_t fill_char = L'█'; //文字填滿部分的符號
    wchar_t background = L' '; //文字未填滿部分的背景符號

    int y_begin = 0; //文字在螢幕上起始y座標
    int y_end = 16; //文字在螢幕上截止y座標
    int y_offset = 0; //文字在螢幕y偏移位置

    int screen_clear_method = SaoFU::utils::TextClearMethod::ClearAllText;
};



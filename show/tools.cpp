#include "tools.h"

#include <string>
#include <thread>
#include <windows.h>

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#define CURL_STATICLIB

#pragma comment(lib, "ws2_32.lib")
#define _CRT_SECURE_NO_WARNINGS

#include <curl/curl.h>

namespace SaoFU {
    std::wstring count_space(int count_size, int width) {
        return std::wstring(((width - count_size) / 16) + 1, L' ');
    }

    void draw_text(Font* font, Param* param, wchar_t* screen) {
        const int& glyph_width = param->glyph_width;

        const int& y_begin = param->y_begin;
        const int& y_end = param->y_end;
        const int& y_offset = param->y_offset;

        const int& x_offset = param->x_offset;
        const int& width = param->screen_width;

        int start = 0;
        for (auto& ch : param->ws) {
            bool is_half_width = false;

            // 畫出一個字元
            for (int row = y_begin; row < y_end; row++) {
                for (int j = 0, pos = 0; j < 16; j++, pos += param->glyph_width_offset) {
                    // 超出畫面寬度就不用繼續畫了
                    bool in_max_range = start + pos + x_offset < width;
                    bool in_min_range = start + pos + x_offset >= 0;

                    if (in_max_range && in_min_range) {
                        screen[start + (row * width + pos) + x_offset] = font[ch][row - y_offset] & (0x8000 >> j) ? param->fill_char : param->background;
                    }
                }
            }

            for (int row = 0; row < param->glyph_height; row++) {
                // 判斷是否為半形字元(字元另一半是空的)
                is_half_width |= font[ch][row] & 0xFF;
            }

            // 計算下一個字元的起始位置
            start += (is_half_width ? glyph_width : glyph_width / 2);
        }
    }

    std::wstring get_time(const wchar_t* fmt) {
        wchar_t wbuf[100]; // Assuming a fixed buffer size for simplicity, adjust as needed
        time_t now = time(0);
        wcsftime(wbuf, sizeof(wbuf), fmt, localtime(&now));
        return std::wstring(wbuf);
    }

    std::wstring utf8_to_utf16(const std::string& str) {
        if (str.empty())
            return std::wstring();

        size_t charsNeeded = ::MultiByteToWideChar(CP_UTF8, 0,
                                                   str.data(), (int)str.size(), NULL, 0);
        if (charsNeeded == 0) {
            //e_what(__LINE__, "Failed converting UTF-8 string to UTF-16", 15);
            throw std::runtime_error("Failed converting UTF-8 string to UTF-16");
        }

        std::vector<wchar_t> buffer(charsNeeded);
        int charsConverted = ::MultiByteToWideChar(CP_UTF8, 0,
                                                   str.data(), (int)str.size(), &buffer[0], buffer.size());
        if (charsConverted == 0) {
            //e_what(__LINE__, "Failed converting UTF-8 string to UTF-16", 15);
            throw std::runtime_error("Failed converting UTF-8 string to UTF-16");
        }

        return std::wstring(&buffer[0], charsConverted);
    }

    std::string utf16_to_utf8(const std::wstring& utf16String) {
        int utf8Length = WideCharToMultiByte(CP_UTF8, 0, utf16String.data(), -1, nullptr, 0, nullptr, nullptr);
        if (utf8Length == 0) {
            // Error handling, possibly throw an exception
            return "";
        }
        std::vector<char> buffer(utf8Length);
        WideCharToMultiByte(CP_UTF8, 0, utf16String.data(), -1, buffer.data(), utf8Length, nullptr, nullptr);
        return std::string(buffer.data());
    }


    HRESULT e_what(int line, const char* file, HRESULT hr) {
        LPSTR p_msgbuf = nullptr;
        DWORD msg_len = FormatMessageA(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL,
            hr,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPSTR)&p_msgbuf, 0, NULL
        );

        char buf[100] = {'\0'};
        if (msg_len) {
            sprintf(buf, "%s\n\"%s\"\n[line] %d\n[code] 0x%08X\n", p_msgbuf, file, line, hr);
        }

        MessageBoxA(0, buf, std::to_string(hr).c_str(), MB_ICONERROR);

        LocalFree(p_msgbuf);
        return hr;
    }
}

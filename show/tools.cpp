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

size_t my_write(void* buf, size_t size, size_t nmemb, void* param) {
    std::string* text = (std::string*)param;
    size_t totalsize = size * nmemb;
    text->append((char*)buf, totalsize);
    return totalsize;
}

namespace SaoFU {
    std::string get_token() {
        std::string client_id = SaoFU::g_setting["client_id"];
        std::string client_secret = SaoFU::g_setting["client_secret"];
        std::string data = "grant_type=client_credentials&client_id=" + client_id + "&client_secret=" + client_secret;

        CURL* curl = curl_easy_init();
        std::string responseBuffer = "";

        if (!curl) {
            SAOFU_EXCEPTION(20);
        }

        curl_easy_setopt(curl, CURLOPT_URL, "https://tdx.transportdata.tw/auth/realms/TDXConnect/protocol/openid-connect/token");
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, my_write);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseBuffer);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());

        curl_easy_perform(curl);
        curl_easy_cleanup(curl);

        nlohmann::json api_token = nlohmann::json::parse(responseBuffer);
        return api_token["access_token"];
    }

    std::string get_data(std::string token, std::string url) {
        CURL* curl = curl_easy_init();
        curl_slist* headers = NULL;
        std::string responseBuffer = "";

        if (!curl) {
            SAOFU_EXCEPTION(20);
        }

        headers = curl_slist_append(headers, "Content-Type: application/json");
        headers = curl_slist_append(headers, ("Authorization: Bearer " + token).c_str());

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, my_write);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseBuffer);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_perform(curl);

        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);

        return responseBuffer;
    }

    int old_sequence = 0;
    void listenForKeyboardEvents(std::string token, int index, nlohmann::json& json) {
        while (true) {
            try {
                std::string str = get_data(token, g_setting["url"]);

                json = nlohmann::json::parse(str);
                int sequence = json[index]["StopSequence"];

                g_trigger = sequence != old_sequence;

                old_sequence = sequence;
            }
            catch (std::exception& e) {
                MessageBoxA(0, e.what(), 0, 0);
            }
            std::this_thread::sleep_for(std::chrono::seconds(2));
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

            // 畫出一個字元
            for (int row = row_begin; row < row_end; row++) {
                for (int col = 0; col < glyph_width; col += param->glyph_width_offset) {
                    // 超出畫面寬度就不用繼續畫了
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
                // 判斷是否為半形字元(字元另一半是空的)
                is_half_width |= font[ch][row] & 0xFF;
            }

            // 計算下一個字元的起始位置
            start += (is_half_width ? glyph_width : glyph_width / 2);
        }
    }

    void get_time(wchar_t* wbuf, const wchar_t* fmt) {
        time_t now = time(0);
        wcsftime(wbuf, sizeof(wbuf), fmt, localtime(&now));
    }

    std::wstring utf8_to_utf16(const std::string& str) {
        if (str.empty())
            return std::wstring();

        size_t charsNeeded = ::MultiByteToWideChar(CP_UTF8, 0,
                                                   str.data(), (int)str.size(), NULL, 0);
        if (charsNeeded == 0) {
            e_what(__LINE__, "Failed converting UTF-8 string to UTF-16", 15);
            throw std::runtime_error("Failed converting UTF-8 string to UTF-16");
        }

        std::vector<wchar_t> buffer(charsNeeded);
        int charsConverted = ::MultiByteToWideChar(CP_UTF8, 0,
                                                   str.data(), (int)str.size(), &buffer[0], buffer.size());
        if (charsConverted == 0) {
            e_what(__LINE__, "Failed converting UTF-8 string to UTF-16", 15);
            throw std::runtime_error("Failed converting UTF-8 string to UTF-16");
        }

        return std::wstring(&buffer[0], charsConverted);
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

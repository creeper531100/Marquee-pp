﻿#include "tools.h"

#include <string>
#include <thread>
#include <windows.h>
#include <sstream>
#include "fmt/format.h"

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

        char buf[100] = { '\0' };
        if (msg_len) {
            sprintf(buf, "%s\n\"%s\"\n[line] %d\n[code] 0x%08X\n", p_msgbuf, file, line, hr);
        }

        MessageBoxA(0, buf, std::to_string(hr).c_str(), MB_ICONERROR);

        LocalFree(p_msgbuf);
        return hr;
    }
}

Maybe<int> calc(int pos, DisplayConfig& param) {
    auto wstr = param.ws.substr(0, pos);
    return SaoFU::count_size(wstr);
}

Maybe<std::string*> is_select(bool sel, std::string* retn) {
    return sel ? retn : Maybe<std::string*>();
}

Maybe<int> get_category(DisplayConfig& param, std::string value) {
    std::stringstream ss(value);
    std::string key = "";
    std::string val = "";
    ss >> key >> val;

    int center_value = is_select(key == "center", &val)
        .and_then(has_param<std::string*>)
        .and_then(convert_to_int<std::string*>)
        .and_then(calc, param)
        .value_or(SaoFU::count_size(param.ws));


    int char_value = is_select(key == "char", &val)
        .and_then(has_param<std::string*>)
        .and_then(convert_to_int<std::string*>)
        .and_then(calc, param)
        .value_or(SaoFU::count_size(param.ws));

    SaoFU::utils::CategoryToValueMap it = {
        { "top", 0 },
        { "begin", 0 },
        { "center", (param.screen_width / 2) - center_value / 2},
        { "end", param.screen_width },
        { "char", char_value }
    };

    if (it.find(key) != it.end()) {
        return it[key];
    };

    return Maybe<int>();
}

Maybe<uintptr_t> is_ptr(Variant<std::string> str) { // Accepts Bello<T> instead of Bello<std::string>
    if (str.has_value()) { // Changed to hasValue() to avoid conflict
        return str.get_ptr(); // Returning ptr if not a string pointer
    }
    return Maybe<uintptr_t>();
}

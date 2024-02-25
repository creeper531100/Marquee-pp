#pragma once
#define _CRT_SECURE_NO_WARNINGS

#include <cstdint>
#include <string>
#include <optional>
#include <nlohmann/json.hpp>

using Json = nlohmann::json;
using Font = uint16_t[16];

struct DisplayConfig;

#define SAOFU_EXCEPTION(hr) SaoFU::e_what(__LINE__, __FILE__, hr)

#define SAOFU_TRY(fn) try fn catch(std::exception &e) { \
    SaoFU::e_what(__LINE__, e.what(), 2); }

namespace SaoFU {
    std::wstring count_space(int count_size, int width);

    std::wstring get_time(const wchar_t* fmt);
    std::wstring utf8_to_utf16(const std::string& str);
    std::string utf16_to_utf8(const std::wstring& utf16String);

    long e_what(int line, const char* file, long hr);

    constexpr int count_size(std::wstring_view show) {
        int end = 0;
        for (int i = 0; i < show.length(); i++) {
            end += show[i] < 256 ? 16 : 32;
        }
        return end;
    }
}

namespace SaoFU::utils {
    enum class TextClearMethod {
        None,
        ClearAll,
        ClearAllText,
        ClearTextItself,
        ClearTextBefore,
        ClearTextAfter,
    };

    using CategoryToValueMap = std::unordered_map<std::string, int>;
}

struct DisplayConfig {
    std::wstring ws; //字串
    int screen_width; //緩衝區大小

    int x_begin = 0; //文字在螢幕上起始x座標
    int x_end = screen_width; //文字在螢幕上截止x座標
    int x_offset = 0; //文字在螢幕x偏移位置

    int glyph_height = 16; //字形的高度(全寬16、半寬16)
    int glyph_width = 32; //字形的寬度(全寬32、半寬16)
    int glyph_width_offset = 2; //字形寬度的偏移量
    int glyph_width_factor = 2; //字形寬度的倍數(數值越小越窄)

    wchar_t fill_char = L'█'; //文字填滿部分的符號
    wchar_t background = L' '; //文字未填滿部分的背景符號

    int y_begin = 0; //文字在螢幕上起始y座標
    int y_end = 16; //文字在螢幕上截止y座標
    int y_offset = 0; //文字在螢幕y偏移位置

    int step = 1;
    int time = 1;

    SaoFU::utils::TextClearMethod clear_text_region = SaoFU::utils::TextClearMethod::ClearAllText;
};

template <typename T>
class Variant {
public:
    union Parma {
        uint64_t ptr;
        uint32_t has_value[2];
        T* str;
    };

    Parma value;

    Variant() {
        this->value.ptr = 0;
    }

    Variant(uint64_t value) {
        this->value.ptr = value;
    }

    bool operator==(const Variant& other) const {
        return value.ptr == other.value.ptr;
    }

    bool hasValue() { // Changed the name to avoid conflict
        return value.has_value[1];
    }

    T get() {
        return *value.str;
    }

    uint64_t get_ptr() {
        return value.ptr;
    }
};

template <typename T>
class Maybe : public std::optional<T> {
public:
    using std::optional<T>::optional;

    template <typename U>
    Maybe<U> map(std::function<Maybe<U>(T)> fn) {
        if (!(*this)) {
            return Maybe<U>(); // 返回一個空的 Maybe<U>
        }
        return fn(**this); // 呼叫函式 fn，並返回其結果
    }

    template <typename F, typename... Args>
    Maybe<T> or_else(F fn, Args&&... args) {
        if (!(*this)) {
            return fn(std::forward<Args>(args)...); // 如果当前 Maybe 为空，则调用 fn 并返回其结果
        }
        return *this; // 如果当前 Maybe 不为空，则返回当前 Maybe
    }
};

Maybe<int> get_category(DisplayConfig& param, std::string value);
Maybe<uintptr_t> is_ptr(Variant<std::string> str);
Maybe<uintptr_t> has_param(std::string* str);
Maybe<int> ConvertToInt(uintptr_t str);

template<typename It, typename Val, typename Pred>
int find_or_predict(It it, Val value, Pred pred) {
    return it.find(value) != it.end() ? it[value] : pred(value);
}
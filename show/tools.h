#pragma once
#define _CRT_SECURE_NO_WARNINGS

#include <cstdint>
#include <string>

#include <type_traits>

#include <optional>
#include <variant>
#include <nlohmann/json.hpp>

#include "def.h"

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

    using KeywordReplacementMap = std::unordered_map<std::string, int>;
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

struct DisplayConfigBuilder : public DisplayConfig {
    DisplayConfigBuilder() {};

    static DisplayConfigBuilder load_form_file(const std::string& file);
    DisplayConfig&& build();


    SETTER(ws);
    SETTER(screen_width);
    SETTER(x_begin);
    SETTER(x_end);
    SETTER(x_offset);
    SETTER(glyph_height);
    SETTER(glyph_width);
    SETTER(glyph_width_offset);
    SETTER(glyph_width_factor);
    SETTER(fill_char);
    SETTER(background);
    SETTER(y_begin);
    SETTER(y_end);
    SETTER(y_offset);
    SETTER(step);
    SETTER(time);
    SETTER(clear_text_region);
};

template <typename _Ty>
class Maybe : public std::optional<_Ty> {
public:
    using std::optional<_Ty>::optional;
    using super = std::optional<_Ty>;

    Maybe(const std::optional<_Ty>& opt) : super(opt) {}
    Maybe(std::optional<_Ty>&& opt) : super(std::move(opt)) {}

    template <class _Fn, typename... Args>
    constexpr auto and_then(_Fn&& _Func, Args&&... args) const& {
        using _Uty = std::invoke_result_t<_Fn, const _Ty&, Args&&...>;

        if (super::has_value()) {
            return std::invoke(std::forward<_Fn>(_Func), static_cast<const _Ty&>(super::value()), std::forward<Args>(args)...);
        }
        return _Uty{};
    }

    template <class _Fn, typename... Args>
    constexpr Maybe<_Ty> or_else(_Fn&& _Func, Args&&... args)&& {
        if (super::has_value()) {
            return std::move(*this);
        }
        else {
            return std::forward<_Fn>(_Func)(std::forward<Args>(args)...);
        }
    }
};

Maybe<int> resolve_keyword(DisplayConfig& param, std::string value);

template<typename T, typename U>
Maybe<T> inspect_is_string(std::variant<T, U> str) {
    T* var = std::get_if<T>(&str);
    if (var) {
        return *var;
    }
    return std::nullopt;
}

Maybe<int> try_parse(std::string str);
Maybe<int> convert_to_int(std::string str);

inline Maybe<std::string> is_empty_string(const std::string& str) {
    if (!str.empty()) {
        return str;
    }
    return std::nullopt;
}


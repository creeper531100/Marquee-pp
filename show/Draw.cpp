﻿#include "tools.h"
#include "Draw.h"

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <thread>
#include <windows.h>
#include "magic_enum/magic_enum.hpp"

#include "Effect.h"


DrawScreen::DrawScreen(DrawScreen&& other) noexcept:
    screen(other.screen),
    font(other.font),
    screen_size(other.screen_size),
    hConsole(other.hConsole) {
    other.screen = nullptr;
    other.font = nullptr;
}

DrawScreen& DrawScreen::operator=(DrawScreen&& other) noexcept {
    if (this != &other) {
        free(font);
        delete[] screen;

        screen = other.screen;
        font = other.font;
        screen_size = other.screen_size;
        hConsole = other.hConsole;

        other.screen = nullptr; // 释放原对象的指针
        other.font = nullptr;
    }
    return *this;
}

void DrawScreen::draw_text(DisplayConfig* param) {
    const int& glyph_width = 16 * param->glyph_width_factor;
    const int glyph_height = param->glyph_height;

    const int& y_begin = param->y_begin;
    const int& y_end = param->y_end;
    const int& y_offset = param->y_offset;

    const int& x_offset = param->x_offset;
    const int& width = param->screen_width;

    int start = 0;

    for (int i = 0; i < param->param_ws.length(); i++) {
        bool is_half_width = false;
        wchar_t ch = param->param_ws[i];

        // 畫出一個字元
        for (int row = y_begin; row < y_end; row++) {
            for (int cols = 0; cols < glyph_width; cols += param->glyph_width_offset) {
                // 超出畫面寬度就不用繼續畫了
                bool in_max_range = start + cols + x_offset < width;
                bool in_min_range = start + cols + x_offset >= 0;

                if (in_max_range && in_min_range) {
                    bool b = font[ch][row - y_offset] & 0x8000 >> cols / param->glyph_width_factor;
                    screen[x_offset + start + (row * width + cols)] = b ? param->fill_char : param->background;
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


void DrawScreen::delay(uintptr_t arg) {
    auto time = is_ptr<const char*>(arg).and_then(try_parse).value_or(arg);
    std::this_thread::sleep_for(std::chrono::milliseconds(time));
}


void DrawScreen::screen_clear() {
    DWORD dwBytesWritten;
    wmemset(screen, L' ', screen_size);
    WriteConsoleOutputCharacterW(hConsole, screen, screen_size, {0, 0}, &dwBytesWritten);
}

std::pair<int, int> DrawScreen::clear_text_region(DisplayConfig& config, int index, bool clear_region) {
    std::pair<int, int> region;
    int x_begin = 0;
    int x_end = 0;

    switch (config.clear_text_region) {
    case SaoFU::utils::TextClearMethod::ClearAllText:
        x_begin = 0;
        x_end = config.screen_width;
        break;
    case SaoFU::utils::TextClearMethod::ClearTextItself:
        x_begin = config.x_offset;
        x_end = SaoFU::count_size(config.param_ws);
        break;
    case SaoFU::utils::TextClearMethod::ClearTextBefore:
        x_begin = 0;
        x_end = SaoFU::count_size(config.param_ws) + config.x_offset;
        break;
    case SaoFU::utils::TextClearMethod::ClearTextAfter:
        x_begin = config.x_offset;
        x_end = config.screen_width - config.x_offset;
        break;
    case SaoFU::utils::TextClearMethod::ClearAll:
        wmemset(screen, config.background, screen_size);
        break;
    }

    region.first = x_begin;
    region.second = x_end;

    if (clear_region) {
        wmemset(screen + (config.screen_width * index + x_begin), config.background, x_end);
    }

    return region;
}

DrawScreen::~DrawScreen() {
    if (font) {
        free(font);
    }
    if (screen) {
        delete[] screen;
    }
}


DrawScreenBuilder DrawScreenBuilder::builder(size_t size, HANDLE hConsole) {
    DrawScreenBuilder builder;
    builder.screen = new wchar_t[size];
    builder.screen_size = size;
    builder.hConsole = hConsole;

    return builder;
}

DrawScreenBuilder& DrawScreenBuilder::load_font(const std::string& path) {
    FILE* fp = fopen(path.c_str(), "rb");
    if (!fp) {
        throw SaoFU::e_what(__LINE__, "找不到字體", 29);
    }

    fseek(fp, 0L, SEEK_END);
    size_t font_size = ftell(fp);
    fseek(fp, 0L, SEEK_SET);

    this->font = (Font*)malloc(font_size);
    fread(this->font, font_size, 1, fp);
    fclose(fp);

    return *this;
}

DrawScreen&& DrawScreenBuilder::build() {
    return std::move(*this);
}

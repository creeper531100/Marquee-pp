#include "Effect.h"
#include "tools.h"
#include "Draw.h"

#include <cwchar>
#include <windows.h>

void Marquee::show(DisplayConfig& config, DrawScreen& scr) {
    DWORD dwBytesWritten;

    config.y_begin = 0;
    config.y_end = 16;
    config.y_offset = 0;

    for (int i = config.x_offset; i >= -config.x_end; i--) {
        wmemset(scr.screen, config.background, scr.screen_size);
        config.x_offset = i;

        scr.draw_text(&config);
        WriteConsoleOutputCharacterW(scr.hConsole, scr.screen, scr.screen_size, {0, 0}, &dwBytesWritten);

        if (i % config.step == 0) {
            scr.delay(config.time);
        }
    }
}

void Slide::show(DisplayConfig& config, DrawScreen& scr) {
    DWORD dwBytesWritten;
    config.y_end = 16;

    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 15; j++) {
            auto region = scr.clear_text_region(config, j, false);
            wmemmove(scr.screen + (config.screen_width * j) + region.first, scr.screen + (config.screen_width * (j + 1)) + region.first, region.second);
            wmemset(scr.screen + ((15 - i) * config.screen_width) + region.first, config.background, region.second);
        }

        /*
         * 可以方便的將每一列往前移動一行，但不能控制偏移
         * memmove(scr, scr + width, (width * 15) * sizeof(wchar_t));
         * memset(scr + ((15 - i) * width), 0, width * sizeof(wchar_t));
         */

        config.y_begin = 15 - i;
        config.y_offset = 15 - i;

        scr.draw_text(&config);

        WriteConsoleOutputCharacterW(scr.hConsole, scr.screen, scr.screen_size, { 0, 0 }, &dwBytesWritten);

        if (i % config.step == 0) {
            scr.delay(config.time);
        }
    }
}

void Flash::show(DisplayConfig& config, DrawScreen& scr) {
    DWORD dwBytesWritten;
    config.y_begin = 0;
    config.y_offset = 0;

    for (int i = 0; i < 16; i++) {
        scr.clear_text_region(config, i);
        config.y_end = i + 1;

        scr.draw_text(&config);
        WriteConsoleOutputCharacterW(scr.hConsole, scr.screen, scr.screen_size, { 0, 0 }, &dwBytesWritten);

        if (i % config.step == 0) {
            scr.delay(config.time);
        }
    }
}

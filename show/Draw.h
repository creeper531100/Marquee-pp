#pragma once
#include <memory>
#include <string>
#include <windows.h>

#include "def.h"
#include "Effect.h"

struct DrawScreenBuilder;
class DrawScreen {
public:
    DrawScreen() {}

    DrawScreen(DrawScreen&& other) noexcept;

    DrawScreen(const DrawScreen&) = delete;

    DrawScreen& operator=(DrawScreen&& other) noexcept;

    template <typename T>
    DrawScreen& display(DisplayConfig& param) {
        std::unique_ptr<IEffect> marquee = std::make_unique<T>();
        marquee->invoke(param, *this);
        return *this;
    }

    std::pair<int, int> clear_text_region(DisplayConfig& config, int index = 0, bool clear_region = true);

    virtual void delay(uintptr_t param);
    virtual void screen_clear();
    virtual void draw_text(DisplayConfig* param);

    ~DrawScreen();

    HANDLE hConsole = nullptr;
    wchar_t* screen = nullptr;
    size_t screen_size;
    Font* font = nullptr;
};


class DrawScreenBuilder : public DrawScreen {
public:
    DrawScreenBuilder() {};
    DrawScreenBuilder(DrawScreen&& config) : DrawScreen(std::move(config)) {};

    static DrawScreenBuilder builder(size_t size, HANDLE hConsole);

    SETTER(hConsole);

    DrawScreenBuilder& load_font(const std::string& path);

    DrawScreen&& build();
};
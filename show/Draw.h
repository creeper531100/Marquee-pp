#pragma once
#include <memory>
#include <string>
#include <variant>
#include <windows.h>

#include "def.h"
#include "Effect.h"
#include "magic_enum/magic_enum.hpp"

class DrawScreen {
public:
    DrawScreen();

    DrawScreen(DrawScreen&& other) noexcept;

    DrawScreen(const DrawScreen&) = delete;

    DrawScreen& operator=(DrawScreen&& other) noexcept;

    void draw_text(DisplayConfig* param);


    template <typename T>
    DrawScreen& display(DisplayConfig& param) {
        std::unique_ptr<IEffect> marquee = std::make_unique<T>();
        marquee->show(param, *this);
        return *this;
    }

    std::pair<int, int> clear_text_region(DisplayConfig& config, int index = 0, bool clear_region = true);

    DrawScreen& invoke_method(std::variant<std::string, IEffect::EffectEnum> effect_name, uintptr_t param);

    virtual void delay(uintptr_t param);
    virtual void screen_clear();

    ~DrawScreen();

    HANDLE hConsole = nullptr;
    wchar_t* screen = nullptr;
    size_t screen_size;
    Font* font = nullptr;
};


class DrawScreenBuilder : public DrawScreen {
public:
    DrawScreenBuilder& set_screen_size(const size_t size);
    SETTER(hConsole);

    static DrawScreenBuilder load_font(const std::string& path);
    DrawScreen&& build();
};

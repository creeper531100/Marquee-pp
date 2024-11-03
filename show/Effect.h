#pragma once
#include <vector>
#include <functional>
#include <memory>
#include <string>
#include <variant>

template <typename T, typename... Ts>
struct EffectFactory {
    static std::vector<std::function<std::unique_ptr<T>()>> list;
};

template <typename T, typename... Ts>
std::vector<std::function<std::unique_ptr<T>()>> EffectFactory<T, Ts...>::list = {
    []() { return std::make_unique<Ts>(); }...
};

struct DisplayConfig;
struct DrawScreen;

struct Marquee;
struct Slide;
struct Flash;
struct DelegateDrawScreen;

struct IEffect {
    enum MethodEnum {
        DrawScreenMethodEnum = 3
    };

    enum struct EffectEnum {
        Unknown = -1,
        Marquee,
        Slide,
        Flash,

        delay        = DrawScreenMethodEnum | 0x00,
        screen_clear = DrawScreenMethodEnum | 0x10
    };

    EffectEnum effect_enum;

    using EffectList = EffectFactory<IEffect, Marquee, Slide, Flash, DelegateDrawScreen>;

    virtual void invoke(DisplayConfig& config, DrawScreen& screen) = 0;

    static std::unique_ptr<IEffect> create_instance(std::variant<std::string, EffectEnum> effect_name);
};


class Marquee : public IEffect {
public:
    void invoke(DisplayConfig& config, DrawScreen& scr) override;
};

class Slide : public IEffect {
public:
    void invoke(DisplayConfig& config, DrawScreen& scr) override;
};

class Flash : public IEffect {
public:
    void invoke(DisplayConfig& config, DrawScreen& scr) override;
};

class DelegateDrawScreen : public IEffect {
public:
    void invoke(DisplayConfig& config, DrawScreen& scr) override;
};

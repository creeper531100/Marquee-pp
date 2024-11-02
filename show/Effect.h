#pragma once
#include <vector>
#include <functional>
#include <memory>

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

struct IEffect {

    enum class EffectEnum {
        Marquee,
        Slide,
        Flash,

        delay,
        screen_clear
    };

    using EffectList = EffectFactory<IEffect, Marquee, Slide, Flash>;

    virtual void show(DisplayConfig& config, DrawScreen& screen) = 0;
};


class Marquee : public IEffect {
public:
    void show(DisplayConfig& config, DrawScreen& scr) override;
};

class Slide : public IEffect {
public:
    void show(DisplayConfig& config, DrawScreen& scr) override;
};

class Flash : public IEffect {
public:
    void show(DisplayConfig& config, DrawScreen& scr) override;
};

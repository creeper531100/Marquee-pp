#pragma once

#include <cstdint>
#include <nlohmann/json.hpp>
#include <windows.h>

using Font = uint16_t[16];
using Json = nlohmann::json;
struct DisplayConfig;

#define SAOFU_EXCEPTION(hr) SaoFU::e_what(__LINE__, __FILE__, hr)

#define SAOFU_TRY(fn) try fn catch(std::exception &e) { \
    SaoFU::e_what(__LINE__, e.what(), 2); }
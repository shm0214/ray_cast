#pragma once
#ifndef __NR_RENDER_SETTINGS_MANAGER_HPP__
#define __NR_RENDER_SETTINGS_MANAGER_HPP__

#include "scene/Camera.hpp"

namespace NRenderer {
struct RenderSettings {
    enum class Acc { NONE, BVH };
    unsigned int width;
    unsigned int height;
    unsigned int depth;
    unsigned int samplesPerPixel;
    float russianRoulette;
    Acc acc = Acc::NONE;
    RenderSettings()
        : width(500),
          height(500),
          depth(4),
          samplesPerPixel(16),
          russianRoulette(0.8) {}
};
struct AmbientSettings {
    enum class Type { CONSTANT, ENVIROMENT_MAP };
    Type type = Type::CONSTANT;
    RGB ambient = {0, 0, 0};
    Handle mapTexture = {};
};
struct RenderSettingsManager {
    Camera camera = {};
    RenderSettings renderSettings = {};
    AmbientSettings ambientSettings = {};
};

}  // namespace NRenderer

#endif
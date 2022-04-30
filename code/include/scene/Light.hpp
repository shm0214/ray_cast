#pragma once
#ifndef __NR_LIGHT_HPP__
#define __NR_LIGHT_HPP__

#include <numbers>
#include "geometry/vec.hpp"

#include <memory>
#include <string>

#include "Model.hpp"

namespace NRenderer {
struct Light {
    enum class Type {
        POINT = 0x0,  // 点光
        SPOT,         // 聚光
        DIRECTIONAL,  // 平行光
        AREA          // 面积光
    };
    // 类型
    Type type;
    // 光源实体的索引
    Index entity;
    Light(Type type) : type(type), entity(0) {}
    Light() = default;
};
SHARE(Light);

struct AreaLight {
    // position u v 与 plain 相同
    Vec3 radiance = {1, 1, 1};
    Vec3 position = {};
    Vec3 u = {};
    Vec3 v = {};
    // 有个问题就是法向量的正负怎么确定
    Vec3 normal = {};
    bool hasNormal = false;
    float area = 0;
    float getArea() {
        if (area == 0)
            area = glm::length(glm::cross(u, v));
        return area;
    }
    Vec3 getNormal() {
        if (!hasNormal) {
            normal = glm::normalize(glm::cross(u, v));
            hasNormal = true;
        }
        return normal;
    }
};
SHARE(AreaLight);

struct PointLight {
    Vec3 intensity = {1, 1, 1};
    Vec3 position = {};
};
SHARE(PointLight);

struct DirectionalLight {
    Vec3 irradiance = {1, 1, 1};
    Vec3 direction = {};
};
SHARE(DirectionalLight);

struct SpotLight {
    Vec3 intensity = {1, 1, 1};
    Vec3 position = {};
    Vec3 direction = {};
    float hotSpot = 3.1415926f / 4.f;
    float fallout = 3.1415926f / 3.f;
};
SHARE(SpotLight);
}  // namespace NRenderer

#endif
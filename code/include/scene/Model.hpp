#pragma once
#ifndef __NR_MODEL_HPP__
#define __NR_MODEL_HPP__

#include <string>
#include <vector>

#include "geometry/vec.hpp"

#include "Material.hpp"
#include "common/macros.hpp"
#include "scene/Bounds.hpp"
namespace NRenderer {
using namespace std;

// Entity也不知道是啥 应该就是个父类
struct Entity {
    enum class EntityType { ENTITY, SPHERE, TRIANGLE, PLANE, MESH };
    EntityType type = EntityType::ENTITY;
    Handle material;
    Bounds boundingBox;
    float area;
    bool hasBound = false;
    bool hasArea = false;
    virtual void calBoundingBox() {}
    virtual void calArea() {}
    Bounds getBoundingBox() {
        if (!hasBound) {
            hasBound = true;
            calBoundingBox();
        }
        return boundingBox;
    }
    float getArea() {
        if (!hasArea) {
            hasArea = true;
            calArea();
        }
        return area;
    }
};
SHARE(Entity);

struct Sphere : public Entity {
    // 球的方向(可以当作极轴的方向)
    Vec3 direction = {0, 0, 1};
    Vec3 position = {0, 0, 0};
    float radius = {0};
    Sphere() { type = EntityType::SPHERE; }
    void calBoundingBox() {
        Vec3 r = {radius, radius, radius};
        boundingBox = Bounds(position - r, position + r);
    }
    void calArea() {
        constexpr float PI = 3.1415926535898f;
        area = 4 * PI * radius * radius;
    }
};
SHARE(Sphere);

struct Triangle : public Entity {
    union {
        struct {
            Vec3 v1;
            Vec3 v2;
            Vec3 v3;
        };
        Vec3 v[3];
    };
    Vec3 normal;
    Triangle() : v1(), v2(), v3(), normal(0, 0, 1) {
        type = EntityType::TRIANGLE;
    }
    Triangle(Vec3 v1, Vec3 v2, Vec3 v3) : v1(v1), v2(v2), v3(v3) {
        type = EntityType::TRIANGLE;
    };
    void calBoundingBox() { boundingBox = Bounds::Union(Bounds(v1, v2), v3); }
    void calArea() { area = glm::length(glm::cross(v2 - v1, v3 - v1)) / 2; }
};
SHARE(Triangle);

struct Plane : public Entity {
    Vec3 normal = {0, 0, 1};
    // position 应该是平面的一个顶点
    Vec3 position = {};
    // u, v 是两条边
    Vec3 u = {};
    Vec3 v = {};
    Plane() { type = EntityType::PLANE; }
    void calBoundingBox() {
        Vec3 v2 = position + u;
        Vec3 v3 = position + v;
        Vec3 v4 = position + u + v;
        boundingBox = Bounds::Union(Bounds(position, v2), Bounds(v3, v4));
    }
    void calArea() { area = glm::length(glm::cross(u, v)); }
};
SHARE(Plane);

struct Node {
    enum class Type { SPHERE = 0x0, TRIANGLE = 0X1, PLANE = 0X2, MESH = 0X3 };
    Type type = Type::SPHERE;
    Index entity;
    Index model;
};
SHARE(Node);

struct Model {
    vector<Index> nodes;
    // 在世界坐标的位置
    Vec3 translation = {0, 0, 0};
    // 缩放
    Vec3 scale = {1, 1, 1};
};
SHARE(Model);
}  // namespace NRenderer

#endif
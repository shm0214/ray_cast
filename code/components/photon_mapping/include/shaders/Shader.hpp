#pragma once
#ifndef __SHADER_HPP__
#define __SHADER_HPP__

#include "common/macros.hpp"
#include "geometry/vec.hpp"
#include "scene/Scene.hpp"

#include "Scattered.hpp"

namespace PhotonMapping {
using namespace NRenderer;
using namespace std;

constexpr float PI = 3.1415926535898f;

class Shader {
   protected:
    Material& material;
    vector<Texture>& textureBuffer;

   public:
    Shader(Material& material, vector<Texture>& textures)
        : material(material), textureBuffer(textures) {}
    virtual Scattered shade(const Ray& ray,
                            const Vec3& hitPoint,
                            const Vec3& normal) const = 0;
    virtual Vec3 eval(const Vec3& in,
                      const Vec3& out,
                      const Vec3& normal) const = 0;
};
SHARE(Shader);
}  // namespace PhotonMapping

#endif
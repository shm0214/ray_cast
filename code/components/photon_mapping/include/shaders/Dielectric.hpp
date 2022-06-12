#pragma once
#ifndef __DIELECTRIC_HPP__
#define __DIELECTRIC_HPP__

#include "Shader.hpp"

namespace PhotonMapping {
class Dielectric : public Shader {
   private:
    float ior = 1;
    Vec3 absorbed = {};

   public:
    Dielectric(Material& material, vector<Texture>& textures);

    Vec3 scatter(const Ray& ray,
                 const Vec3& hitPoint,
                 const Vec3& normal) const;
    Scattered shade(const Ray& ray,
                    const Vec3& hitPoint,
                    const Vec3& normal) const;
    Vec3 eval(const Vec3& in, const Vec3& out, const Vec3& normal) const;
    float fresnel(float cos, float ratio) const;
};

}  // namespace PhotonMapping

#endif
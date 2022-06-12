#pragma once
#ifndef __METAL_HPP__
#define __METAL_HPP__

#include "Shader.hpp"

namespace PathTracer {
class metal : public Shader {
    //光滑金属的fuzz为0
   private:
    Vec3 albedo;
    float fuzz;
   public:
    metal(Material& material, vector<Texture>& textures,double fuzz);
    Scattered shade(const Ray& ray,
                    const Vec3& hitPoint,
                    const Vec3& normal) const;
    Vec3 eval(const Vec3& in, const Vec3& out, const Vec3& normal) const;
};
}  // namespace PathTracer

#endif
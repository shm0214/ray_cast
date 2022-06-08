#include "shaders/Dielectric.hpp"
#include "samplers/SamplerInstance.hpp"

namespace PathTracer {
Dielectric::Dielectric(Material& material, vector<Texture>& textures)
    : Shader(material, textures) {
    absorbed =
        (*(material.getProperty<Property::Wrapper::RGBType>("absorbed"))).value;
    ior = (*(material.getProperty<Property::Wrapper::FloatType>("ior"))).value;
}
Vec3 Dielectric::scatter(const Ray& ray,
                         const Vec3& hitPoint,
                         const Vec3& normal) const {
    float cos = clamp(glm::dot(-ray.direction, normal), 1, -1);
    float sin = sqrt(1 - cos * cos);
    Vec3 n = normal;
    float ratio = 1 / ior;
    if (cos < 0) {
        // 此时从内部向外部
        cos = -cos;
        n = -n;
        ratio = ior;
    }
    if (ratio * sin > 1 ||
        fresnel(cos, ratio) >
            defaultSamplerInstance<UniformSampler>().sample1d()) {
        return ray.direction - 2 * glm::dot(ray.direction, n) * n;
    } else {
        Vec3 r1 = ratio * (ray.direction + cos * n);
        Vec3 r2 = -sqrt(1 - distance2(r1)) * n;
        return r1 + r2;
    }
}
Scattered Dielectric::shade(const Ray& ray,
                            const Vec3& hitPoint,
                            const Vec3& normal) const {
    auto dir = scatter(ray, hitPoint, normal);
    auto attenuation = absorbed;
    return {Ray{hitPoint, dir}, attenuation, Vec3{0}, 0};
}

float Dielectric::fresnel(float cos, float ratio) const {
    float r = (1 - ratio) / (1 + ratio);
    r = r * r;
    return r + (1 - r) * pow(1 - cos, 5);
}

Vec3 Dielectric::eval(const Vec3& in,
                      const Vec3& out,
                      const Vec3& normal) const {
    return {};
}
}  // namespace PathTracer
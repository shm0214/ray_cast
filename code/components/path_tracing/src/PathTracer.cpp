#include "server/Server.hpp"

#include "PathTracer.hpp"

#include <chrono>
#include "VertexTransformer.hpp"
#include "intersections/intersections.hpp"

#include <string>
#include "glm/gtc/matrix_transform.hpp"

namespace PathTracer {
RGB PathTracerRenderer::gamma(const RGB& rgb) {
    return glm::sqrt(rgb);
}

void PathTracerRenderer::renderTask(RGBA* pixels,
                                    int width,
                                    int height,
                                    int off,
                                    int step) {
    for (int i = off; i < height; i += step) {
        for (int j = 0; j < width; j++) {
            Vec3 color{0, 0, 0};
            for (int k = 0; k < samples; k++) {
                auto r = defaultSamplerInstance<UniformInSquare>().sample2d();
                float rx = r.x;
                float ry = r.y;
                float x = (float(j) + rx) / float(width);
                float y = (float(i) + ry) / float(height);
                auto ray = camera.shoot(x, y);
                color += trace(ray);
            }
            color /= samples;
            color = gamma(color);
            pixels[(height - i - 1) * width + j] = {color, 1};
        }
    }
}

auto PathTracerRenderer::render() -> RenderResult {
    // shaders
    shaderPrograms.clear();
    ShaderCreator shaderCreator{};
    for (auto& m : scene.materials) {
        shaderPrograms.push_back(shaderCreator.create(m, scene.textures));
    }

    RGBA* pixels = new RGBA[width * height]{};

    // 局部坐标转换成世界坐标
    VertexTransformer vertexTransformer{};
    vertexTransformer.exec(spScene);
    if (acc == RenderSettings::Acc::BVH) {
        getServer().logger.log("Building BVH...");
        auto start = chrono::system_clock::now();
        scene.buildBVH();
        auto end = chrono::system_clock::now();
        auto duration = duration_cast<chrono::microseconds>(end - start);
        getServer().logger.log(std::format(
            "Done. Time: {}s", double(duration.count()) *
                                   chrono::microseconds::period::num /
                                   chrono::microseconds::period::den));
    }

    const auto taskNums = 8;
    thread t[taskNums];
    for (int i = 0; i < taskNums; i++) {
        t[i] = thread(&PathTracerRenderer::renderTask, this, pixels, width,
                      height, i, taskNums);
    }
    for (int i = 0; i < taskNums; i++) {
        t[i].join();
    }
    getServer().logger.log("Done...");
    return {pixels, width, height};
}

void PathTracerRenderer::release(const RenderResult& r) {
    auto [p, w, h] = r;
    delete[] p;
}

HitRecord PathTracerRenderer::closestHitObject(const Ray& r) {
    HitRecord closestHit = nullopt;
    if (acc == RenderSettings::Acc::NONE) {
        float closest = FLOAT_INF;
        for (auto& s : scene.sphereBuffer) {
            auto hitRecord = Intersection::xSphere(r, s, 0.000001, closest);
            if (hitRecord && hitRecord->t < closest) {
                closest = hitRecord->t;
                closestHit = hitRecord;
            }
        }
        for (auto& t : scene.triangleBuffer) {
            auto hitRecord = Intersection::xTriangle(r, t, 0.000001, closest);
            if (hitRecord && hitRecord->t < closest) {
                closest = hitRecord->t;
                closestHit = hitRecord;
            }
        }
        for (auto& p : scene.planeBuffer) {
            auto hitRecord = Intersection::xPlane(r, p, 0.000001, closest);
            if (hitRecord && hitRecord->t < closest) {
                closest = hitRecord->t;
                closestHit = hitRecord;
            }
        }
    } else if (acc == RenderSettings::Acc::BVH)
        closestHit = Intersection::closestHitObject(scene.bvh, r);
    return closestHit;
}

tuple<float, Vec3> PathTracerRenderer::closestHitLight(const Ray& r) {
    Vec3 v = {};
    HitRecord closest = getHitRecord(FLOAT_INF, {}, {}, {});
    for (auto& a : scene.areaLightBuffer) {
        auto hitRecord = Intersection::xAreaLight(r, a, 0.000001, closest->t);
        if (hitRecord && closest->t > hitRecord->t) {
            closest = hitRecord;
            v = a.radiance;
        }
    }
    return {closest->t, v};
}

HitRecord PathTracerRenderer::sampleLight() const {
    float emitAreaSum = 0;
    for (auto& a : scene.areaLightBuffer) {
        emitAreaSum += a.getArea();
    }
    auto random = defaultSamplerInstance<UniformSampler>();
    float p = random.sample1d() * emitAreaSum;
    emitAreaSum = 0;
    for (auto& a : scene.areaLightBuffer) {
        auto area = a.getArea();
        emitAreaSum += area;
        if (p <= emitAreaSum) {
            Vec3 pos =
                a.position + random.sample1d() * a.u + random.sample1d() * a.v;
            return getHitRecord(0, pos, a.getNormal(), 1 / area, a.radiance,
                                {});
        }
    }
    return nullopt;
}

inline void print(const std::string str, const Vec3& v) {
    getServer().logger.log(str + ": " + std::format("{} {} {}", v.x, v.y, v.z));
}

inline void print(const std::string str, const float& f) {
    getServer().logger.log(str + ": " + std::to_string(f));
}

inline bool testEqual(const Vec3& a, const Vec3& b, float epsilon = 0.0005f) {
    return glm::length(a - b) < epsilon;
}

inline bool testEqual(const float& a, const float& b, float epsilon = 0.0005f) {
    return fabs(a - b) < epsilon;
}

RGB PathTracerRenderer::getAmbientColor(const Ray& r) {
    if (scene.ambient.type == Ambient::Type::CONSTANT)
        return scene.ambient.constant;
    else if (scene.ambient.type == Ambient::Type::ENVIRONMENT_MAP) {
        return Vec3{0};
    }
}

RGB PathTracerRenderer::trace(const Ray& r) {
    auto hitObject = closestHitObject(r);
    auto [t, emitted] = closestHitLight(r);
    // hit object
    if (hitObject && hitObject->t < t) {
        auto hit = sampleLight();
        Vec3 L_dir = {};
        Vec3 L_indir = {};
        auto mtlHandle = hitObject->material;
        switch (scene.materials[mtlHandle.index()].type) {
            case 0: {
                if (hit) {
                    auto& intersection = hit.value();
                    Vec3 lightPoint = intersection.hitPoint;
                    float lightPdf = intersection.pdf;
                    auto hitPoint = hitObject.value().hitPoint;
                    Vec3 ws = glm::normalize(lightPoint - hitPoint);
                    Vec3 NN = intersection.normal;
                    Vec3 N = hitObject.value().normal;
                    Vec3 wo = -r.direction;
                    auto temp = closestHitObject(Ray(hitPoint, ws));
                    auto [t, radiance] = closestHitLight(Ray(hitPoint, ws));
                    auto testPoint = hitPoint + t * ws;
                    float ws_dot_NN = glm::dot(-ws, NN);
                    if ((!temp.has_value() || temp->t > t ||
                         testEqual(temp->t, t)) &&
                        testEqual(testPoint, lightPoint) && ws_dot_NN > 0) {
                        L_dir =
                            radiance *
                            shaderPrograms[mtlHandle.index()]->eval(wo, ws, N) *
                            glm::dot(ws, N) * ws_dot_NN /
                            (float)pow(glm::length(lightPoint - hitPoint), 2) /
                            lightPdf;
                    }
                }
                float russianRoulette = scene.renderOption.russianRoulette;
                if (defaultSamplerInstance<UniformSampler>().sample1d() <
                    russianRoulette) {
                    auto scattered = shaderPrograms[mtlHandle.index()]->shade(
                        r, hitObject->hitPoint, hitObject->normal);
                    auto scatteredRay = scattered.ray;
                    auto temp = closestHitObject(scatteredRay);
                    auto [t, radiance] = closestHitLight(scatteredRay);
                    if (temp && temp->t < t && !testEqual(temp->t, t)) {
                        auto attenuation = scattered.attenuation;
                        auto emitted = scattered.emitted;
                        float n_dot_in =
                            glm::dot(hitObject->normal, scatteredRay.direction);
                        float pdf = scattered.pdf;
                        auto next = trace(Ray(scatteredRay));
                        L_indir = clamp(next) * attenuation * n_dot_in / pdf /
                                  russianRoulette;
                    }
                }
                return L_indir + L_dir;
            }
            case 2:
            case 3: {
                auto scattered = shaderPrograms[mtlHandle.index()]->shade(
                    r, hitObject->hitPoint, hitObject->normal);
                auto ray = scattered.ray;
                auto attenuation = scattered.attenuation;
                float n_dot_in = glm::dot(hitObject->normal, ray.direction);
                //print("n_dot_in", n_dot_in);
                if (defaultSamplerInstance<UniformSampler>().sample1d() <
                    russianRoulette) {
                    auto res = attenuation * trace(ray) * fabs(n_dot_in) /
                               russianRoulette;
                    return res;
                }
            }
        }
        return {};
    } else if (t != FLOAT_INF) {
        return emitted;
    } else {
        return Vec3{0};
    }
}
}  // namespace PathTracer
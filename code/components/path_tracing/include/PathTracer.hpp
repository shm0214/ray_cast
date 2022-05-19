#pragma once
#ifndef __SIMPLE_PATH_TRACER_HPP__
#define __SIMPLE_PATH_TRACER_HPP__

#include "Camera.hpp"
#include "Ray.hpp"
#include "intersections/HitRecord.hpp"
#include "scene/Scene.hpp"

#include "../app/include/manager/RenderSettingsManager.hpp"
#include "shaders/ShaderCreator.hpp"

#include <tuple>
namespace PathTracer {
using namespace NRenderer;
using namespace std;

class PathTracerRenderer {
   public:
   private:
    SharedScene spScene;
    Scene& scene;

    unsigned int width;
    unsigned int height;
    unsigned int depth;
    unsigned int samples;

    float russianRoulette;
    RenderSettings::Acc acc;

    using SCam = PathTracer::Camera;
    SCam camera;

    vector<SharedShader> shaderPrograms;

   public:
    PathTracerRenderer(SharedScene spScene)
        : spScene(spScene), scene(*spScene), camera(spScene->camera) {
        width = scene.renderOption.width;
        height = scene.renderOption.height;
        depth = scene.renderOption.depth;
        samples = scene.renderOption.samplesPerPixel;
        russianRoulette = scene.renderOption.russianRoulette;
        acc = scene.renderOption.acc;
    }
    ~PathTracerRenderer() = default;

    using RenderResult = tuple<RGBA*, unsigned int, unsigned int>;
    RenderResult render();
    void release(const RenderResult& r);

   private:
    void renderTask(RGBA* pixels, int width, int height, int off, int step);

    RGB gamma(const RGB& rgb);
    RGB trace(const Ray& ray);
    HitRecord closestHitObject(const Ray& r);
    tuple<float, Vec3> closestHitLight(const Ray& r);
    HitRecord sampleLight() const;
};
}  // namespace PathTracer

#endif
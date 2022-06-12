#pragma once
#ifndef __SIMPLE_PATH_TRACER_HPP__
#define __SIMPLE_PATH_TRACER_HPP__

#include "Camera.hpp"
#include "Ray.hpp"
#include "intersections/HitRecord.hpp"
#include "photon.hpp"
#include "photonMap.hpp"
#include "scene/Scene.hpp"

#include "../app/include/manager/RenderSettingsManager.hpp"
#include "shaders/ShaderCreator.hpp"

#include <tuple>
namespace PhotonMapping {
using namespace NRenderer;
using namespace std;

class PhotonMappingRenderer {
   public:
   private:
    SharedScene spScene;
    Scene& scene;
    PhotonMap map;

    unsigned int width;
    unsigned int height;
    unsigned int depth;
    unsigned int samples;

    float russianRoulette;
    RenderSettings::Acc acc;

    using SCam = PhotonMapping::Camera;
    SCam camera;

    vector<SharedShader> shaderPrograms;

   public:
    PhotonMappingRenderer(SharedScene spScene)
        : spScene(spScene), scene(*spScene), camera(spScene->camera) {
        width = scene.renderOption.width;
        height = scene.renderOption.height;
        depth = scene.renderOption.depth;
        samples = scene.renderOption.samplesPerPixel;
        russianRoulette = scene.renderOption.russianRoulette;
        acc = scene.renderOption.acc;
    }
    ~PhotonMappingRenderer() = default;

    using RenderResult = tuple<RGBA*, unsigned int, unsigned int>;
    RenderResult render();
    void release(const RenderResult& r);

   private:
    void renderTask(RGBA* pixels, int width, int height, int off, int step);

    RGB gamma(const RGB& rgb);
    RGB trace(const Ray& ray);
    RGB color(const Ray& ray, int depth);
    RGB getAmbientColor(const Ray& ray);
    HitRecord closestHitObject(const Ray& r);
    tuple<float, Vec3> closestHitLight(const Ray& r);
    HitRecord sampleLight() const;
    tuple<Vec3, Vec3> genPhoton(AreaLight light);
    void tracePhoton(const Ray& ray, int depth, Vec3 power);
    void genPhotonMap();
};
}  // namespace PhotonMapping

#endif
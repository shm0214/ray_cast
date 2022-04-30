#include "Camera.hpp"
#include "component/RenderComponent.hpp"
#include "scene/Scene.hpp"
#include "server/Server.hpp"

#include "PathTracer.hpp"

using namespace std;
using namespace NRenderer;

namespace PathTracer {
class Adapter : public RenderComponent {
    void render(SharedScene spScene) {
        PathTracerRenderer renderer{spScene};
        auto renderResult = renderer.render();
        auto [pixels, width, height] = renderResult;
        getServer().screen.set(pixels, width, height);
        renderer.release(renderResult);
    }
};
}  // namespace PathTracer

const static string description =
    "A Path Tracer. "
    "Only some simple primitives and materials(Lambertian) are supported."
    "\nPlease use scene file : cornel_area_light.scn\n"
    "Add Russian Roulette and Sampling the light\n"
    "But with little flaw : use clamp to solve";

REGISTER_RENDERER(PathTracer, description, PathTracer::Adapter);
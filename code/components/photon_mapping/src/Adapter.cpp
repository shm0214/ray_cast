#include "Camera.hpp"
#include "component/RenderComponent.hpp"
#include "scene/Scene.hpp"
#include "server/Server.hpp"

#include "PhotonMapping.hpp"

using namespace std;
using namespace NRenderer;

namespace PhotonMapping {
class Adapter : public RenderComponent {
    void render(SharedScene spScene) {
        PhotonMappingRenderer renderer{spScene};
        auto renderResult = renderer.render();
        auto [pixels, width, height] = renderResult;
        getServer().screen.set(pixels, width, height);
        renderer.release(renderResult);
    }
};
}  // namespace PhotonMapping

const static string description = "PhotonMapping";

REGISTER_RENDERER(PhotonMapping, description, PhotonMapping::Adapter);
#include "Scene.hpp"

namespace sigel
{
    void DefaultScene::onEnter(ResourceManager &rm, PipelineManager &pm)
    {
        uint32_t tex  = rm.createTextureImage("../flo.jpg");
        uint32_t mesh = rm.loadMesh(cube_vertices, cube_indices);
        uint32_t defaultPipeline = pm.getPipelineID("default");
        objects.push_back({ defaultPipeline, mesh, tex });
    }

    void DefaultScene::onExit(ResourceManager &rm, PipelineManager &pm)
    {
        objects.clear();
    }

    void DefaultScene::onUpdate(float dt)
    {
        elapsed += dt;
        for (size_t i = 0; i < objects.size(); i++) {
            glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(i * 1.5f, 0.0f, 0.0f));
            objects[i].transform = glm::rotate(model, elapsed * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        }
    }
}
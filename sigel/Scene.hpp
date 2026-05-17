#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "IScene.hpp"
namespace sigel
{
    class DefaultScene : public IScene {
        std::vector<SceneObject> objects;
        float elapsed = 0.0f;
        // Camera camera;
    public:
        void onEnter(ResourceManager& rm, PipelineManager& pm) override {
            uint32_t tex  = rm.createTextureImage("../flo.jpg");
            uint32_t mesh = rm.loadMesh(cube_vertices, cube_indices);
            objects.push_back({ 0, mesh, tex });
        }

        void onExit(ResourceManager&, PipelineManager&) override {
            objects.clear();
        }

        void onUpdate(float dt) override 
        {
            elapsed += dt;
            for (size_t i = 0; i < objects.size(); i++) {
                glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(i * 1.5f, 0.0f, 0.0f));
                objects[i].transform = glm::rotate(model, elapsed * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
            }
        }

        const std::vector<SceneObject>& getObjects() const override { return objects; }
        // const Camera&                   getCamera()  const override { return camera; }

    };
}
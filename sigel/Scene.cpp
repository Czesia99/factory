#include "Scene.hpp"
#include "SigelEngine.hpp"

namespace sigel
{
    void Scene::onSetup()
    {
        uint32_t flotex = ResourceManager::get().createTextureImage("../assets/textures/flo.jpg");
        uint32_t mesh = ResourceManager::get().createMesh(cube_vertices, cube_indices);
        uint32_t defaultPipeline = PipelineManager::get().getPipelineID("default");

        SceneObject object;
        object.pipelineID = defaultPipeline;
        object.meshes.push_back({ mesh, flotex });

        objects.push_back(object);
        objects[0].transform.rotation = glm::vec3{0.0f, 0.0f, 0.0f};

        glfwSetInputMode(SigelEngine::get().window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }

    void Scene::onUpdate(float dt)
    {
        elapsed += dt;

        auto& input = SigelEngine::get().inputManager;
        if (input.isPressed(GLFW_KEY_LEFT_ALT)) SigelEngine::get().editor.swapMode();

        if (SigelEngine::get().editor.display) { return; }

        if (input.isHeld(GLFW_KEY_W)) camera.processKeyboardMovement(FORWARD, dt);
        if (input.isHeld(GLFW_KEY_S)) camera.processKeyboardMovement(BACKWARD, dt);
        if (input.isHeld(GLFW_KEY_A)) camera.processKeyboardMovement(LEFT, dt);
        if (input.isHeld(GLFW_KEY_D)) camera.processKeyboardMovement(RIGHT, dt);
        if (input.isHeld(GLFW_KEY_SPACE)) camera.processKeyboardMovement(UP, dt);
        if (input.isHeld(GLFW_KEY_LEFT_CONTROL)) camera.processKeyboardMovement(DOWN, dt);

        if (input.isPressed(GLFW_KEY_ESCAPE)) glfwSetWindowShouldClose(SigelEngine::get().window, true);

        camera.processMouseMovement(input.getMouseDeltaX(), input.getMouseDeltaY());
    }
}

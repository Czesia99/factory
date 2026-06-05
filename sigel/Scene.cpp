#include "Scene.hpp"
#include "SigelEngine.hpp"

namespace sigel
{
    void DefaultScene::onEnter(ResourceManager &rm, PipelineManager &pm)
    {
        // uint32_t casstex  = rm.createTextureImage("../cassgare.jpg");
        uint32_t flotex  = rm.createTextureImage("../flo.jpg");
        uint32_t mesh = rm.loadMesh(cube_vertices, cube_indices);
        uint32_t defaultPipeline = pm.getPipelineID("default");
        objects.push_back({ defaultPipeline, mesh, flotex });
        // objects.push_back({ defaultPipeline, mesh, flotex });

        // objects[1].transform = glm::translate
        objects[0].transform.rotation = glm::vec3{0.0f, 0.0f, 0.0f};

        glfwSetInputMode(SigelEngine::get().window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }

    void DefaultScene::onExit(ResourceManager &rm, PipelineManager &pm)
    {
        objects.clear();
    }

    void DefaultScene::onUpdate(float dt)
    {
        // elapsed += dt;
        // for (size_t i = 0; i < objects.size(); i++) {
        //     glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(i * 1.5f, 0.0f, - 4.0f));
        //     objects[i].transform = glm::rotate(model, elapsed * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        // }

        auto& input = SigelEngine::get().inputManager; 
        if (input.isHeld(GLFW_KEY_W)) camera.processKeyboardMovement(FORWARD, dt);
        if (input.isHeld(GLFW_KEY_S)) camera.processKeyboardMovement(BACKWARD, dt);
        if (input.isHeld(GLFW_KEY_A)) camera.processKeyboardMovement(LEFT, dt);
        if (input.isHeld(GLFW_KEY_D)) camera.processKeyboardMovement(RIGHT, dt);
        if (input.isHeld(GLFW_KEY_SPACE)) camera.processKeyboardMovement(UP, dt);
        if (input.isHeld(GLFW_KEY_LEFT_CONTROL)) camera.processKeyboardMovement(DOWN, dt);

        if (input.isPressed(GLFW_KEY_LEFT_ALT))  SigelEngine::get().drawScene("testscene");
        if (input.isPressed(GLFW_KEY_ESCAPE)) glfwSetWindowShouldClose(SigelEngine::get().window, true);

        camera.processMouseMovement(input.getMouseDeltaX(), input.getMouseDeltaY());
    }
}
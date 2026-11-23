#include "TestScene.hpp"
#include <mvel/MvelEngine.hpp>
#include <mvel/Loader.hpp>
#include <print>

namespace factory
{
    void TestScene::onSetup()
    {
        // uint32_t flotex = ResourceManager::get().createTextureImage("../assets/textures/flo.jpg");
        // uint32_t cubemesh = ResourceManager::get().createMesh(cube_vertices, cube_indices);
        uint32_t defaultPipeline = PipelineManager::get().getPipelineID("default");

        PipelineConfig pbrconfig
        {
            .name = "pbr",
            .shaderPath = "../mvel/shaders/pbr.spv"
        };

        uint32_t pbrPipeline =  PipelineManager::get().createPipeline(pbrconfig);

        glfwSetInputMode(MvelEngine::get().window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }

    void TestScene::onEnter()
    {
        uint32_t pbr = PipelineManager::get().getPipelineID("pbr");
        auto &c = MvelEngine::get().coordinator;

        Entity ak = createEntity();
        c.AddComponent(ak, Transform{glm::vec3(-4.0f, 0.0f, 0.0f)});
        c.AddComponent(ak, RenderComponent{.pipelineID = pbr, .meshes = loadAssimpModel("../assets/models/ak47/ak.obj")});
        status("TESTSCENE", "enter test scene");
    }

    void TestScene::onExit()
    {
        status("TESTSCENE", "exit test scene");
    }


    void TestScene::onUpdate(float dt)
    {
        // elapsed += dt;
        // for (size_t i = 0; i < objects.size(); i++) {
        //     glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(i * 1.5f, 0.0f, - 4.0f));
        //     objects[i].transform = glm::rotate(model, elapsed * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        // }

        auto& input = MvelEngine::get().inputManager;
        if (input.isPressed(GLFW_KEY_LEFT_ALT))  MvelEngine::get().editor.swapMode();

        if (MvelEngine::get().editor.display) { return; }

        if (input.isHeld(GLFW_KEY_W)) camera.processKeyboardMovement(FORWARD, dt);
        if (input.isHeld(GLFW_KEY_S)) camera.processKeyboardMovement(BACKWARD, dt);
        if (input.isHeld(GLFW_KEY_A)) camera.processKeyboardMovement(LEFT, dt);
        if (input.isHeld(GLFW_KEY_D)) camera.processKeyboardMovement(RIGHT, dt);
        if (input.isHeld(GLFW_KEY_SPACE)) camera.processKeyboardMovement(UP, dt);
        if (input.isHeld(GLFW_KEY_LEFT_CONTROL)) camera.processKeyboardMovement(DOWN, dt);

        if (input.isPressed(GLFW_KEY_TAB))  MvelEngine::get().drawScene("default");
        if (input.isPressed(GLFW_KEY_ESCAPE)) glfwSetWindowShouldClose(MvelEngine::get().window, true);

        camera.processMouseMovement(input.getMouseDeltaX(), input.getMouseDeltaY());
    }
}

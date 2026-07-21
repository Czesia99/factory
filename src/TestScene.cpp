#include "TestScene.hpp"
#include "../sigel/SigelEngine.hpp"

namespace factory
{
    void TestScene::onEnter(ResourceManager &rm, PipelineManager &pm)
    {
        // uint32_t casstex  = rm.createTextureImage("../cassgare.jpg");
        uint32_t chips_tex = rm.createTextureImage("../assets/models/chipsbag/chips_audran.png");
        uint32_t flotex  = rm.createTextureImage("../assets/textures/flo.jpg");
        uint32_t mesh = rm.createMesh(cube_vertices, cube_indices);
        uint32_t defaultPipeline = pm.getPipelineID("default");

        SceneObject object;

        object.pipelineID = defaultPipeline;
        object.meshes.push_back({
            mesh,
            flotex
        });

        objects.push_back(object);

        SceneObject car;
        car.pipelineID = defaultPipeline;
        car.meshes = SigelEngine::get().loadTinyModel("../assets/models/chipsbag/chips2.obj");
        // car.meshes[0].textureID = chips_tex;
        car.transform.scale *= 10.0f;
        objects.push_back(car);

        // objects[1].transform = glm::translate
        objects[0].transform.rotation = glm::vec3{0.0f, 0.0f, 0.0f};

        glfwSetInputMode(SigelEngine::get().window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }

    void TestScene::onExit(ResourceManager &rm, PipelineManager &pm)
    {
        objects.clear();
    }

    void TestScene::onUpdate(float dt)
    {
        // elapsed += dt;
        // for (size_t i = 0; i < objects.size(); i++) {
        //     glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(i * 1.5f, 0.0f, - 4.0f));
        //     objects[i].transform = glm::rotate(model, elapsed * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        // }

        auto& input = SigelEngine::get().inputManager; 
        if (input.isPressed(GLFW_KEY_LEFT_ALT))  SigelEngine::get().editor.swapMode();
        
        if (SigelEngine::get().editor.display) { return; }

        if (input.isHeld(GLFW_KEY_W)) camera.processKeyboardMovement(FORWARD, dt);
        if (input.isHeld(GLFW_KEY_S)) camera.processKeyboardMovement(BACKWARD, dt);
        if (input.isHeld(GLFW_KEY_A)) camera.processKeyboardMovement(LEFT, dt);
        if (input.isHeld(GLFW_KEY_D)) camera.processKeyboardMovement(RIGHT, dt);
        if (input.isHeld(GLFW_KEY_SPACE)) camera.processKeyboardMovement(UP, dt);
        if (input.isHeld(GLFW_KEY_LEFT_CONTROL)) camera.processKeyboardMovement(DOWN, dt);

        if (input.isPressed(GLFW_KEY_TAB))  SigelEngine::get().drawScene("default");
        if (input.isPressed(GLFW_KEY_ESCAPE)) glfwSetWindowShouldClose(SigelEngine::get().window, true);

        camera.processMouseMovement(input.getMouseDeltaX(), input.getMouseDeltaY());
    }
}
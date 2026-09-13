#include "TestScene.hpp"
#include "../sigel/SigelEngine.hpp"
#include <sigel/Loader.hpp>
#include <print>

namespace factory
{
    void TestScene::onSetup()
    {
        uint32_t flotex = ResourceManager::get().createTextureImage("../assets/textures/flo.jpg");
        uint32_t mesh = ResourceManager::get().createMesh(cube_vertices, cube_indices);
        uint32_t defaultPipeline = PipelineManager::get().getPipelineID("default");

        PipelineConfig pbrconfig
        {
            .name = "pbr",
            .shaderPath = "../sigel/shaders/pbr.spv"
        };

        uint32_t pbrPipeline =  PipelineManager::get().createPipeline(pbrconfig);


        SceneObject cube;

        cube.pipelineID = defaultPipeline;
        cube.meshes.push_back({
            mesh,
            flotex
        });

        cube.transform.position = glm::vec3(-2.5f, 2.0f, 0.0f);
        objects.push_back(cube);


        SceneObject chips;
        chips.pipelineID = defaultPipeline;
        chips.meshes = loadAssimpModel("../assets/models/chipsbag/chips2.obj");
        objects.push_back(chips);

        // SceneObject chips2;
        // chips2.pipelineID = defaultPipeline;
        // chips2.meshes = loadAssimpModel("../assets/models/chipsbag/chips2.obj");
        // chips2.transform.position = glm::vec3(-4.5f, 0.0f, 0.0f);
        // objects.push_back(chips2);

        // SceneObject sofa;
        // sofa.pipelineID = pbrPipeline;
        // sofa.meshes = loadAssimpModel("../assets/models/sofa/curvesofa.fbx");
        // sofa.transform.position = glm::vec3(0.5f, 0.0f, -5.0f);
        // sofa.transform.scale *= 0.01f;
        // objects.push_back(sofa);

        SceneObject ak;
        ak.pipelineID = pbrPipeline;
        ak.meshes = loadAssimpModel("../assets/models/ak47/ak.obj");
        ak.transform.position = glm::vec3(-4.0f, 0.0f, 0.0f);
        // ak.transform.scale *= 0.01f;
        objects.push_back(ak);

        // SceneObject building;
        // building.pipelineID = pbrPipeline;
        // building.meshes = loadAssimpModel("../assets/models/building/old_residential_building.fbx");
        // building.transform.scale *= 0.01f;
        // objects.push_back(building);

        glfwSetInputMode(SigelEngine::get().window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }

    void TestScene::onEnter()
    {
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

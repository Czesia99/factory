#include "Scene.hpp"
#include "Loader.hpp"
#include "SigelEngine.hpp"
namespace sigel
{
    Entity Scene::createEntity()
    {
        Entity e = SigelEngine::get().coordinator.CreateEntity();
        entities.push_back(e);
        return e;
    }

    void Scene::destroyEntities()
    {
        auto& coordinator = SigelEngine::get().coordinator;
        for (Entity e : entities)
            coordinator.DestroyEntity(e);
        entities.clear();
    }

    void Scene::onSetup()
    {
        uint32_t flotex = ResourceManager::get().createTextureImage("../assets/textures/flo.jpg");
        uint32_t mesh = ResourceManager::get().createMesh(cube_vertices, cube_indices);
        uint32_t defaultPipeline = PipelineManager::get().getPipelineID("default");

        glfwSetInputMode(SigelEngine::get().window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }

    void Scene::onEnter()
    {
        Entity ak = createEntity();
        auto& c = SigelEngine::get().coordinator;
        c.AddComponent(ak, Transform{glm::vec3(-4.0f, 0.0f, 0.0f)});
        c.AddComponent(ak, RenderComponent{.pipelineID = 0, .meshes = loadAssimpModel("../assets/models/chipsbag/chips2.obj")});
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

        if (input.isPressed(GLFW_KEY_TAB))  SigelEngine::get().drawScene("testscene");
        if (input.isPressed(GLFW_KEY_ESCAPE)) glfwSetWindowShouldClose(SigelEngine::get().window, true);

        camera.processMouseMovement(input.getMouseDeltaX(), input.getMouseDeltaY());
    }
}

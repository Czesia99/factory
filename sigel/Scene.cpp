#include "Scene.hpp"
#include "SigelEngine.hpp"

namespace sigel
{
    void DefaultScene::onEnter(ResourceManager &rm, PipelineManager &pm)
    {
        printf("ENTER DEFAULT SCENE\n");
        // uint32_t casstex  = rm.createTextureImage("../cassgare.jpg");
        uint32_t flotex  = rm.createTextureImage("../flo.jpg");
        uint32_t mesh = rm.loadMesh(cube_vertices, cube_indices);
        uint32_t defaultPipeline = pm.getPipelineID("default");
        objects.push_back({ defaultPipeline, mesh, flotex });
        // objects.push_back({ defaultPipeline, mesh, flotex });

        // objects[1].transform = glm::translate
        objects[0].transform.rotation = glm::vec3{0.0f, 0.0f, 0.0f};

    }

    void DefaultScene::onExit(ResourceManager &rm, PipelineManager &pm)
    {
        printf("EXIT DEFAULT SCENE\n");
        objects.clear();
    }

    void DefaultScene::onUpdate(float dt)
    {
        // elapsed += dt;
        // for (size_t i = 0; i < objects.size(); i++) {
        //     glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(i * 1.5f, 0.0f, - 4.0f));
        //     objects[i].transform = glm::rotate(model, elapsed * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        // }
    }

    void DefaultScene::processInput(const MovementInput &input, float dt)
    {
        if (input.moveForward)  camera.processKeyboardMovement(FORWARD, dt);
        if (input.moveBackward) camera.processKeyboardMovement(BACKWARD, dt);
        if (input.moveLeft)     camera.processKeyboardMovement(LEFT, dt);
        if (input.moveRight)    camera.processKeyboardMovement(RIGHT, dt);
        if (input.moveUp)       camera.processKeyboardMovement(UP, dt);
        if (input.moveDown)     camera.processKeyboardMovement(DOWN, dt);
        // if (input.changeScene)  SigelEngine::get().queueScene("testscene");
    }

    void DefaultScene::keyCallback(int key, int scancode, int action, int mods)
    {
        if (key == GLFW_KEY_LEFT_ALT && action == GLFW_PRESS)
        {
            SigelEngine::get().queueScene("testscene");
        }
    }

    void DefaultScene::mouseCallback(float dx, float dy)
    {
        camera.processMouseMovement(dx, dy);
    }
}
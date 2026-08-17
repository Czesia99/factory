#include "SigelEngine.hpp"
#include <iostream>
#include <chrono>

#ifdef IMGUI_IMPL_VULKAN_USE_VOLK
#define VOLK_IMPLEMENTATION
#include <volk.h>
#endif

#include <tiny_obj_loader.h>

namespace sigel
{
    void SigelEngine::run()
    {
        try {
            mainLoop();
        } catch (const std::exception& e) {
            std::cerr << "Fatal: " << e.what() << "\n";
        }
        cleanup();
    }

    void SigelEngine::initWindow()
    {
        glfwInit();

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        window = glfwCreateWindow(WIDTH, HEIGHT, "FACTORY", nullptr, nullptr);

        glfwSetWindowUserPointer(window, this);
        glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
        glfwSetKeyCallback(window, keyCallbackWrapper);
        glfwSetCursorPosCallback(window, mouseCallbackWrapper);
    }

    void SigelEngine::initEngine()
    {
        vctx.init(window);
        editor.init(window, vctx);
        status("CORE", "Vulkan context ready");
        addScene("default", new DefaultScene());
    }

    void SigelEngine::mainLoop()
    {
        auto last = std::chrono::high_resolution_clock::now();
        while (!glfwWindowShouldClose(window)) {
            inputManager.update();
            glfwPollEvents();
            auto now = std::chrono::high_resolution_clock::now();
            float dt = std::chrono::duration<float>(now - last).count();
            last = now;

            if (!nextActiveScene && !activeScene)
            {
                status("ENGINE", "no scene provided, loading default scene");
                drawScene("default");
            }

            if (nextActiveScene)
            {
                loadScene(nextActiveScene);
                nextActiveScene = nullptr;
            }

            if (activeScene)
            {
                activeScene->onUpdate(dt);
                editor.update(activeScene);
            }

            vctx.renderer.drawFrame(*activeScene, editor.display);
        }
        vctx.waitIdle();
    }

    void SigelEngine::cleanup()
    {
        editor.cleanup();

        vctx.renderer.cleanupRenderObjects();
        vctx.resourceManager.cleanup();

        for (auto& [name, scene] : scenes)
        {
            if (scene)
            {
                scene->onDestroy();
                delete scene;
            }
        }

        // scenes.clear();
        vctx.clean();

        glfwDestroyWindow(window);
        glfwTerminate();
    }

    void SigelEngine::addScene(const std::string& name, IScene* scene)
    {
        scenes[name] = scene;
    }

    void SigelEngine::drawScene(const std::string& name)
    {
        if (scenes.contains(name))
        {
            status("SCENE MANAGER", "Drawing [" + name + "] Scene");
            nextActiveScene = scenes.at(name);
        }
        else
        {
            status("SCENE MANAGER", "Scene [" + name + "] does not exist");
        }
    }

    void SigelEngine::loadScene(IScene* scene)
    {
        vctx.waitIdle();

        if (activeScene)
        {
            activeScene->onExit();
            vctx.renderer.cleanupRenderObjects();
        }

        if (!scene->isSetup)
        {
            scene->onSetup();
            scene->isSetup = true;
        }

        scene->onEnter();
        vctx.renderer.prepareScene(*scene);
        activeScene = scene;
    }

    void SigelEngine::framebufferResizeCallback(GLFWwindow* window, int width, int height)
    {
        auto app = reinterpret_cast<SigelEngine*>(glfwGetWindowUserPointer(window));
        app->vctx.renderer.framebufferResized = true;
    }

    void SigelEngine::keyCallbackWrapper(GLFWwindow* window, int key, int scancode, int action, int mods)
    {
        auto app = reinterpret_cast<SigelEngine*>(glfwGetWindowUserPointer(window));
        app->inputManager.onKey(key, action);
    }

    void SigelEngine::mouseCallbackWrapper(GLFWwindow* window, double x, double y)
    {
        auto app = reinterpret_cast<SigelEngine*>(glfwGetWindowUserPointer(window));
        app->inputManager.onMouseMove(static_cast<float>(x), static_cast<float>(y));
    }

    std::vector<SubMesh> SigelEngine::loadTinyModel(std::string path)
    {
		tinyobj::attrib_t                attrib;
		std::vector<tinyobj::shape_t>    shapes;
		std::vector<tinyobj::material_t> materials;
		std::string                      warn, err;

        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;

        std::unordered_map<Vertex, uint32_t> uniqueVertices{};

        std::vector<SubMesh> meshes;

        std::string base_dir = "";
        size_t pos = path.find_last_of("/\\");
        if (pos != std::string::npos)
        {
            base_dir = path.substr(0, pos + 1);
        }

        if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.c_str(), base_dir.c_str()))
        {
            throw std::runtime_error(warn + err);
        }

        std::cout << materials.size() << " materials found in " << path << std::endl;

        for (const auto &shape : shapes)
        {
            for (const auto &index : shape.mesh.indices)
            {
                Vertex vertex{};

                vertex.pos = {
                    attrib.vertices[3 * index.vertex_index + 0],
                    attrib.vertices[3 * index.vertex_index + 1],
                    attrib.vertices[3 * index.vertex_index + 2]
                };

                vertex.texCoord = {
                    attrib.texcoords[2 * index.texcoord_index + 0],
                    1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
                };

				auto [it, inserted] = uniqueVertices.insert({vertex, static_cast<uint32_t>(vertices.size())});
				if (inserted)
				{
                    vertices.push_back(vertex);
				}

				indices.push_back(it->second);
            }

            uint32_t mesh = vctx.resourceManager.createMesh(vertices, indices);
            uint32_t texid = vctx.resourceManager.createTextureImage(base_dir + '/' + materials[shape.mesh.material_ids[0]].diffuse_texname);
            meshes.push_back({mesh, texid});
        }
        return meshes;
    }
}

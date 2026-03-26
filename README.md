# factory
experiment zone for graphic programming with vulkan

TODO:
    - PipelineManager 
        -PipelineConfig struct / PipelineInstance
        - std::vector<PipelineInstance> pipelines;
        - add pipelineID to gameobject
    Renderer
        - move descriptorSet / uniformBuffer
    Scene
        - scene system
        - std::vector scene in engine
        - add / remove / update (dt)
        - hold the gameobjects (move from renderer)
        - give &scene to renderer
        - camera
        - glfw callbacks


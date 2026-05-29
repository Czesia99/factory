#pragma once
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

namespace sigel
{
    struct CamSettings
    {
        glm::vec3 initial_pos = glm::vec3(0.0f, 0.0f, 5.0f);
        glm::vec3 pos = initial_pos;
        glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
        glm::vec3 front = glm::vec3(0.0f, 0.0f, -1.0f);
        glm::vec3 right = glm::vec3(1.0f, 0.0f, 0.0f);

        float yaw = -90.0f;
        float pitch = 0.0f;
        float fov = 60.0f;

        float speed = 2.0f;
        float sensitivity = 0.1f;

        float near_plane = 0.1f;
        float far_plane = 300.0f;
    };

    enum CamDirection {
        FORWARD,
        BACKWARD,
        LEFT,
        RIGHT,
        UP,
        DOWN
    };
    
    const glm::vec3 WORLD_UP = glm::vec3(0.0f, 1.0f, 0.0f);

    class Camera
    {
        public:
            CamSettings cam;

            bool fps;
            bool movement_lock;
            bool constrain_pitch = true;
    
        private:
        
        public:
            Camera();
            Camera(CamSettings conf);

            glm::mat4 getViewMatrix() const;
            glm::mat4 getProjectionMatrix(float aspect) const;

            void processKeyboardMovement(CamDirection direction, float delta_time);
            void processMouseMovement(float xoffset, float yoffset);
            
        private:
            void updateCameraVectors();
    };
}
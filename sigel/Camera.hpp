#pragma once
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

namespace sigel
{
    enum CamDirection {
        FORWARD,
        BACKWARD,
        LEFT,
        RIGHT,
        UP,
        DOWN
    };

    const float YAW             =  -90.0f;
    const float PITCH           =  0.0f;
    const float SENSITIVITY     =  0.1f;
    const float FOV             =  45.0f;
    const glm::vec3 DEFAULT_POS = glm::vec3(0.0f, 0.0f, 0.0f);
    const glm::vec3 FRONT       =  glm::vec3(0.0f, 0.0f, -1.0f);
    const glm::vec3 WORLD_UP    =  glm::vec3(0.0f, 1.0f, 0.0f);

    class Camera
    {
        public:
            glm::vec3 position;
            glm::vec3 initial_pos;
            glm::vec3 front;
            glm::vec3 up;
            glm::vec3 right;
            glm::vec3 world_up;

            float yaw;
            float pitch;
            float movement_speed;
            float sensitivity;
            float fov;

            float near_plane;
            float far_plane;

            int width;
            int height;

            bool fps;
            bool movement_lock;
            bool constrain_pitch = true;
    
        private:
        
        public:
            Camera() 
            {
                position = glm::vec3(0.0f, 2.0f, 5.0f);
                initial_pos = position;
                up = glm::vec3(0.0f, 1.0f, 0.0f);     
                front = glm::vec3(0.0f, 0.0f, -1.0f);
                right = glm::vec3(1.0f, 0.0f, 0.0f);
                world_up = glm::vec3(0.0f, 1.0f, 0.0f);

                yaw = -90.0f;
                pitch = 0.0f;
                fov = 60.0f;

                movement_speed = 2.0f;
                sensitivity = 0.1f;

                near_plane = 0.1f;
                far_plane = 300.0f;
                
                fps = false;
                movement_lock = false;
                
                updateCameraVectors();
            }

            glm::mat4 getViewMatrix() const
            {
                return glm::lookAt(position, position + front, up);
            }

            glm::mat4 getProjectionMatrix(float aspect) const
            {
                glm::mat4 proj = glm::perspective(glm::radians(fov), aspect, near_plane, far_plane);
                proj[1][1] *= -1.0f;    
                return proj;            
            }

            void processKeyboardMovement(CamDirection direction, float delta_time)
            {
                float velocity = movement_speed * delta_time;
                if (direction == FORWARD)
                    position += front * velocity;
                if (direction == BACKWARD)
                    position -= front * velocity;
                if (direction == LEFT)
                    position -= right * velocity;
                if (direction == RIGHT)
                    position += right * velocity;
                if (direction == UP)
                    position += WORLD_UP * velocity;
                if (direction == DOWN)
                    position -= WORLD_UP * velocity;
                if (fps)
                    position.y = initial_pos.y;
            }

            void processMouseMovement(float xoffset, float yoffset)
            {
                if (movement_lock == true)
                    return;

                xoffset *= sensitivity;
                yoffset *= sensitivity;

                yaw   += xoffset;
                pitch -= yoffset;

                if (constrain_pitch)
                {
                    if (pitch > 89.0f)
                        pitch = 89.0f;
                    if (pitch < -89.0f)
                        pitch = -89.0f;
                }

                updateCameraVectors();
            }

            void updateCameraVectors()
            {
                glm::vec3 nfront;
                nfront.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
                nfront.y = sin(glm::radians(pitch));
                nfront.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
                front = glm::normalize(nfront);
                right = glm::normalize(glm::cross(front, world_up));
                up = glm::normalize(glm::cross(right, front));
            }


        private:
    };
}
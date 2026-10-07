#include "Camera.hpp"

namespace sigel
{
    Camera::Camera() : settings{CamSettings {}}
    {
        movement_lock = false;

        updateCameraVectors();
    }

    Camera::Camera(CamSettings conf) : settings(conf)
    {
        movement_lock = false;

        updateCameraVectors();
    }

    glm::mat4 Camera::getViewMatrix() const
    {
        return glm::lookAt(settings.pos, settings.pos + settings.front, settings.up);
    }

    glm::mat4 Camera::getProjectionMatrix(float aspect) const
    {
        glm::mat4 proj = glm::perspective(glm::radians(settings.fov), aspect, settings.near_plane, settings.far_plane);
        return proj;
    }

    void Camera::processKeyboardMovement(CamDirection direction, float delta_time)
    {
        float velocity = settings.speed * delta_time;
        if (direction == FORWARD)
            settings.pos += settings.front * velocity;
        if (direction == BACKWARD)
            settings.pos -= settings.front * velocity;
        if (direction == LEFT)
            settings.pos -= settings.right * velocity;
        if (direction == RIGHT)
            settings.pos += settings.right * velocity;
        if (direction == UP)
            settings.pos += settings.up * velocity; //* -1.0f;
        if (direction == DOWN)
            settings.pos -= settings.up * velocity;// * -1.0f;
    }

    void Camera::processMouseMovement(float dx, float dy)
    {
        if (movement_lock == true)
            return;

        dx *= settings.sensitivity;
        dy *= settings.sensitivity;

        settings.yaw   += dx;
        settings.pitch -= dy; //* -1.0f;


        if (constrain_pitch)
        {
            if (settings.pitch > 89.0f)
                settings.pitch = 89.0f;
            if (settings.pitch < -89.0f)
                settings.pitch = -89.0f;
        }

        updateCameraVectors();
    }

    void Camera::updateCameraVectors()
    {
        glm::vec3 nfront;
        nfront.x = cos(glm::radians(settings.yaw)) * cos(glm::radians(settings.pitch));
        nfront.y = sin(glm::radians(settings.pitch));
        nfront.z = sin(glm::radians(settings.yaw)) * cos(glm::radians(settings.pitch));
        settings.front = glm::normalize(nfront);
        settings.right = glm::normalize(glm::cross(settings.front, WORLD_UP));
        settings.up = glm::normalize(glm::cross(settings.right, settings.front));
    }
}

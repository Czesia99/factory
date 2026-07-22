#include "Camera.hpp"

namespace sigel
{
    Camera::Camera() : cam{CamSettings {}}
    {
        movement_lock = false;

        updateCameraVectors();
    }

    Camera::Camera(CamSettings conf) : cam(conf)
    {
        movement_lock = false;

        updateCameraVectors();
    }

    glm::mat4 Camera::getViewMatrix() const
    {
        return glm::lookAt(cam.pos, cam.pos + cam.front, cam.up);
    }

    glm::mat4 Camera::getProjectionMatrix(float aspect) const
    {
        glm::mat4 proj = glm::perspective(glm::radians(cam.fov), aspect, cam.near_plane, cam.far_plane);
        return proj;
    }

    void Camera::processKeyboardMovement(CamDirection direction, float delta_time)
    {
        float velocity = cam.speed * delta_time;
        if (direction == FORWARD)
            cam.pos += cam.front * velocity;
        if (direction == BACKWARD)
            cam.pos -= cam.front * velocity;
        if (direction == LEFT)
            cam.pos -= cam.right * velocity;
        if (direction == RIGHT)
            cam.pos += cam.right * velocity;
        if (direction == UP)
            cam.pos += cam.up * velocity; //* -1.0f;
        if (direction == DOWN)
            cam.pos -= cam.up * velocity;// * -1.0f;
    }

    void Camera::processMouseMovement(float dx, float dy)
    {
        if (movement_lock == true)
            return;

        dx *= cam.sensitivity;
        dy *= cam.sensitivity;

        cam.yaw   += dx;
        cam.pitch -= dy; //* -1.0f;


        if (constrain_pitch)
        {
            if (cam.pitch > 89.0f)
                cam.pitch = 89.0f;
            if (cam.pitch < -89.0f)
                cam.pitch = -89.0f;
        }

        updateCameraVectors();
    }

    void Camera::updateCameraVectors()
    {
        glm::vec3 nfront;
        nfront.x = cos(glm::radians(cam.yaw)) * cos(glm::radians(cam.pitch));
        nfront.y = sin(glm::radians(cam.pitch));
        nfront.z = sin(glm::radians(cam.yaw)) * cos(glm::radians(cam.pitch));
        cam.front = glm::normalize(nfront);
        cam.right = glm::normalize(glm::cross(cam.front, WORLD_UP));
        cam.up = glm::normalize(glm::cross(cam.right, cam.front));
    }
}

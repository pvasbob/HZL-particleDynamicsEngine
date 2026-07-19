#include "renderer/Camera.h"

#include <glm/geometric.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace hzl::renderer
{
    Camera::Camera(
        glm::vec3 position,
        glm::vec3 target,
        glm::vec3 up
    )
        : position_(position),
          target_(target),
          up_(up)
    {
    }

    void Camera::moveForward(float distance)
    {
        position_ += forward() * distance;
    }

    void Camera::strafe(float distance)
    {
        const glm::vec3 movement = right() * distance;

        position_ += movement;
        target_ += movement;
    }

    glm::mat4 Camera::viewMatrix() const
    {
        return glm::lookAt(position_, target_, up_);
    }

    glm::vec3 Camera::forward() const
    {
        return glm::normalize(target_ - position_);
    }

    glm::vec3 Camera::right() const
    {
        return glm::normalize(glm::cross(forward(), up_));
    }
}

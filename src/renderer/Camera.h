#pragma once

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

namespace hzl::renderer
{
    class Camera
    {
    public:
        Camera(
            glm::vec3 position,
            glm::vec3 target,
            glm::vec3 up
        );

        void moveForward(float distance);
        void strafe(float distance);

        glm::mat4 viewMatrix() const;

    private:
        glm::vec3 forward() const;
        glm::vec3 right() const;

        glm::vec3 position_;
        glm::vec3 target_;
        glm::vec3 up_;
    };
}

#include "scene/Container.h"

#include <glm/gtc/matrix_transform.hpp>

namespace hzl::scene
{
    Container::Container()
        : floorModelMatrix_(1.0f),
          leftWallModelMatrix_(1.0f),
          rightWallModelMatrix_(1.0f),
          backWallModelMatrix_(1.0f)
    {
        physicsSettings_.gravity = glm::vec3(0.0f, -0.6f, 0.0f);
        physicsSettings_.floorY = -1.0f;
        physicsSettings_.leftWallX = -1.5f;
        physicsSettings_.rightWallX = 1.5f;
        physicsSettings_.backWallZ = -1.5f;
        physicsSettings_.frontWallZ = 1.5f;
        physicsSettings_.restitution = 0.75f;
        physicsSettings_.damping = 0.999f;
        physicsSettings_.restingSpeed = 0.02f;
        physicsSettings_.groundFriction = 0.98f;
        physicsSettings_.horizontalRestingSpeed = 0.01f;
        physicsSettings_.particleRadius = 0.02f;
        physicsSettings_.particleRestitution = 1.0f;

        constexpr float floorThickness = 0.1f;
        constexpr float wallThickness = 0.1f;
        constexpr float wallHeight = 2.0f;
        constexpr float wallDepth = 3.0f;

        floorModelMatrix_ = glm::scale(
            glm::translate(
                glm::mat4(1.0f),
                glm::vec3(
                    0.0f,
                    physicsSettings_.floorY - floorThickness * 0.5f,
                    0.0f
                )
            ),
            glm::vec3(3.0f, floorThickness, 3.0f)
        );

        leftWallModelMatrix_ = glm::scale(
            glm::translate(
                glm::mat4(1.0f),
                glm::vec3(
                    physicsSettings_.leftWallX - wallThickness * 0.5f,
                    physicsSettings_.floorY + wallHeight * 0.5f,
                    0.0f
                )
            ),
            glm::vec3(wallThickness, wallHeight, wallDepth)
        );

        rightWallModelMatrix_ = glm::scale(
            glm::translate(
                glm::mat4(1.0f),
                glm::vec3(
                    physicsSettings_.rightWallX + wallThickness * 0.5f,
                    physicsSettings_.floorY + wallHeight * 0.5f,
                    0.0f
                )
            ),
            glm::vec3(wallThickness, wallHeight, wallDepth)
        );

        backWallModelMatrix_ = glm::scale(
            glm::translate(
                glm::mat4(1.0f),
                glm::vec3(
                    0.0f,
                    physicsSettings_.floorY + wallHeight * 0.5f,
                    physicsSettings_.backWallZ - wallThickness * 0.5f
                )
            ),
            glm::vec3(wallDepth, wallHeight, wallThickness)
        );
    }

    const hzl::simulation::ParticleSystemSettings&
    Container::physicsSettings() const
    {
        return physicsSettings_;
    }

    const glm::mat4& Container::floorModelMatrix() const
    {
        return floorModelMatrix_;
    }

    const glm::mat4& Container::leftWallModelMatrix() const
    {
        return leftWallModelMatrix_;
    }

    const glm::mat4& Container::rightWallModelMatrix() const
    {
        return rightWallModelMatrix_;
    }

    const glm::mat4& Container::backWallModelMatrix() const
    {
        return backWallModelMatrix_;
    }
}

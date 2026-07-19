#pragma once

#include "simulation/ParticleSystem.h"

#include <glm/mat4x4.hpp>

namespace hzl::scene
{
    class Container
    {
    public:
        Container();

        const hzl::simulation::ParticleSystemSettings& physicsSettings() const;

        const glm::mat4& floorModelMatrix() const;
        const glm::mat4& leftWallModelMatrix() const;
        const glm::mat4& rightWallModelMatrix() const;
        const glm::mat4& backWallModelMatrix() const;

    private:
        hzl::simulation::ParticleSystemSettings physicsSettings_;

        glm::mat4 floorModelMatrix_;
        glm::mat4 leftWallModelMatrix_;
        glm::mat4 rightWallModelMatrix_;
        glm::mat4 backWallModelMatrix_;
    };
}

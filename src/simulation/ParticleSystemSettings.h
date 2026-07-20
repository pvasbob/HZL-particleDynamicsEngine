#pragma once

#include <glm/vec3.hpp>

namespace hzl::simulation
{
    struct ParticleSystemSettings
    {
        glm::vec3 gravity{ 0.0f, -0.6f, 0.0f };

        float floorY = -1.0f;
        float leftWallX = -1.5f;
        float rightWallX = 1.5f;
        float backWallZ = -1.5f;
        float frontWallZ = 1.5f;

        float restitution = 0.75f;
        float damping = 0.999f;
        float restingSpeed = 0.02f;
        float groundFriction = 0.98f;
        float horizontalRestingSpeed = 0.01f;
        float particleRadius = 0.02f;
        float particleRestitution = 1.0f;
    };
}

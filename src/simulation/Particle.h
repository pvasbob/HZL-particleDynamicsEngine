#pragma once

#include <glm/vec3.hpp>

namespace hzl::simulation {

    struct Particle 
    {
        glm::vec3 position;
        glm::vec3 velocity;
        glm::vec3 acceleration{0.0f, 0.0f, 0.0f};
    };
    
}
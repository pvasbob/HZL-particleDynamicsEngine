#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <cstdlib>

#include <iostream>
#include <vector>
#include <cstddef>


void framebufferSizeCallback(GLFWwindow*, int width, int height)
{
    glViewport(0, 0, width, height);
    std::cout << "width: " << width << "  " <<  "height: " << height << '\n';
}


constexpr const char* vertexShaderSource = R"(
    #version 330 core

    layout (location = 0) in vec3 aPosition;
    uniform mat4 uMvp;
    out vec3 vertexColor;

    void main() 
    {
        gl_Position = uMvp * vec4(aPosition, 1.0);
        vertexColor = aPosition + vec3(0.5);
    }

)";

constexpr const char* fragmentShaderSource = R"(
    #version 330 core

    in vec3 vertexColor;
    out vec4 fragmentColor;

    void main()
    {
        fragmentColor = vec4(vertexColor, 1.0);
    }

)";


GLuint compileShader(GLenum shaderType, const char* source)
{
    const GLuint shader = glCreateShader(shaderType);

    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    GLint compiled = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);

    if (compiled == GL_FALSE)
    {
        char log[1024];
        glGetShaderInfoLog(
            shader,
            static_cast<GLsizei>(sizeof(log)),
            nullptr,
            log
        );

        std::cerr << "Shader compilation failed:\n"
                  << log
                  << '\n';

        glDeleteShader(shader);
        return 0;

    }

    return shader;
}

struct Particle 
{
    glm::vec3 position;
    glm::vec3 velocity;
    glm::vec3 acceleration{0.0f, 0.0f, 0.0f};
};


int main() 
{
    if (glfwInit() == GLFW_FALSE)
    {
        std::cerr << "Failed to initialize GLFW. \n";
        return EXIT_FAILURE; 
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(
        1280,
        720,
        "HZL Particle Dynamics Engine",
        nullptr,
        nullptr
    );

    if (window == nullptr)
    {
        std::cerr << "Failed to create GLFW window. \n";
        glfwTerminate();
        return EXIT_FAILURE;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

    const int version = gladLoadGL(glfwGetProcAddress);
    if (version == 0)
    {
        std::cerr << "Failed to load OpenGL functions. \n";
        glfwDestroyWindow(window);
        glfwTerminate();
        return EXIT_FAILURE;
    }

    std::cout << "Loaded OpenGL "
                << GLAD_VERSION_MAJOR(version) 
                << "."
                << GLAD_VERSION_MINOR(version) 
                << "\n";            

    glEnable(GL_DEPTH_TEST);


    // constexpr float triangleVertices[] = {
    //      0.0f,  0.6f, 0.0f,
    //     -0.6f, -0.5f, 0.0f,
    //      0.6f, -0.5f, 0.0f
    // };


    constexpr float cubeVertices[]
    {
        // Back face
        -0.5f, -0.5f, -0.5f,
         0.5f,  0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f,  0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,
        -0.5f,  0.5f, -0.5f,

        // Front face
        -0.5f, -0.5f,  0.5f,
         0.5f, -0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f,
        -0.5f, -0.5f,  0.5f,

        // Left face
        -0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,
        -0.5f, -0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f,

        // Right face
         0.5f,  0.5f,  0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f,  0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f,  0.5f,  0.5f,
         0.5f, -0.5f,  0.5f,

        // Bottom face
        -0.5f, -0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f, -0.5f,  0.5f,
         0.5f, -0.5f,  0.5f,
        -0.5f, -0.5f,  0.5f,
        -0.5f, -0.5f, -0.5f,

        // Top face
        -0.5f,  0.5f, -0.5f,
         0.5f,  0.5f,  0.5f,
         0.5f,  0.5f, -0.5f,
         0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f, -0.5f,
        -0.5f,  0.5f,  0.5f
    };


    GLuint vertexBuffer = 0;

    GLuint vertexArray = 0;
    glGenVertexArrays(1, &vertexArray);

    glGenBuffers(1, &vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    

    glBufferData(
        GL_ARRAY_BUFFER,
        sizeof(cubeVertices),
        cubeVertices,
        GL_STATIC_DRAW
    );


    glBindVertexArray(vertexArray);
    glVertexAttribPointer(
        0,
        3,
        GL_FLOAT,
        GL_FALSE,
        3*sizeof(float),
        nullptr
    );
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);


    const GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderSource);

    if (vertexShader == 0)
    {
        glDeleteVertexArrays(1, &vertexArray);
        glDeleteBuffers(1, &vertexBuffer);
        glfwDestroyWindow(window);
        glfwTerminate();

        return EXIT_FAILURE;
    }

    const GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);

    if (fragmentShader == 0)
    {
        glDeleteShader(vertexShader);
        glDeleteVertexArrays(1, &vertexArray);
        glDeleteBuffers(1, &vertexBuffer);
        glfwDestroyWindow(window);
        glfwTerminate();

        return EXIT_FAILURE;
    }

    const GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    GLint linked = GL_FALSE;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &linked);

    if (linked == GL_FALSE)
    {
        char log[1024];
        glGetProgramInfoLog(
            shaderProgram,
            static_cast<GLsizei>(sizeof(log)),
            nullptr,
            log
        );

        std::cerr << "Shader program linking failed:\n"
                  << log
                  << '\n';

        glDeleteProgram(shaderProgram);
        glDeleteShader(fragmentShader);
        glDeleteShader(vertexShader);
        glDeleteVertexArrays(1, &vertexArray);
        glDeleteBuffers(1, &vertexBuffer);
        glfwDestroyWindow(window);
        glfwTerminate();

        return EXIT_FAILURE;
    }

    glDeleteShader(fragmentShader);
    glDeleteShader(vertexShader);

    const GLint mvpLocation = glGetUniformLocation(shaderProgram, "uMvp");
    // const glm::mat4 mvp(1.0f);


    glm::vec3 cameraPosition(0.0f, 0.0f, 2.0f);
    glm::vec3 cameraTarget(0.0f, 0.0f, 0.0f);
    const glm::vec3 cameraUp(0.0f, 1.0f, 0.0f);
    
    
    // std::vector<Particle> particles
    // {
        // {
            // glm::vec3(-0.25f, 0.35f, 0.80f),
            // glm::vec3( 0.20f, 0.00f, 0.00f)
        // },
        // {
            // glm::vec3( 0.25f, 0.35f, 0.80f),
            // glm::vec3(-0.20f, 0.00f, 0.00f)
        // },
        // {
            // glm::vec3(0.00f, 0.75f, 0.80f),
            // glm::vec3(0.00f, 0.00f, 0.00f)
        // }
    // };

    std::vector<Particle> particles;
    // 
    constexpr int particleCountX = 5;         
    constexpr int particleCountY = 4;     
    constexpr int particleCountZ = 3;     
    constexpr float initialSpacing = 0.23f;    

    particles.reserve(
        particleCountX * 
        particleCountY *
        particleCountZ
    );

    for (int yIndex = 0; yIndex < particleCountY; ++yIndex)
    {
        for (int zIndex = 0; zIndex < particleCountZ; ++zIndex)
        {
            for (int xIndex = 0; xIndex < particleCountX; ++xIndex)
            {
                const glm::vec3 position(
                    (xIndex - 2) * initialSpacing,
                    0.20f + yIndex * initialSpacing,
                    0.50f + zIndex * initialSpacing
                );

                const glm::vec3 velocity(
                    -position.x * 1.5f,
                    0.0f,
                    -(position.z - 0.70f) * 0.40f
                );

                particles.push_back({position, velocity});
            }
        }
    }

    std::vector<glm::vec3> particlePositions;
    particlePositions.reserve(particles.size());

    for (const Particle& particle: particles)
    {
        particlePositions.push_back(particle.position);
    }

    GLuint particleVertexArray = 0;
    GLuint particleVertexBuffer = 0;

    glGenVertexArrays(1, &particleVertexArray);
    glGenBuffers(1, &particleVertexBuffer);

    glBindVertexArray(particleVertexArray);
    glBindBuffer(GL_ARRAY_BUFFER, particleVertexBuffer);

    glBufferData(
        GL_ARRAY_BUFFER,
        static_cast<GLsizeiptr>(
            particlePositions.size() * sizeof(glm::vec3)
        ),
        particlePositions.data(),
        GL_DYNAMIC_DRAW
    );

    glVertexAttribPointer(
        0, 
        3,
        GL_FLOAT,
        GL_FALSE,
        sizeof(glm::vec3),
        nullptr
    );
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    float lastFrameTime = static_cast<float>(glfwGetTime());

    constexpr float simulationStep = 1.0f/120.0f;
    float simulationAccumulator = 0.0f;

    const glm::vec3 gravity(0.0f, -0.6f, 0.0f);

    const float floorY = -1.0f;
    const float leftWallX = -1.5f;
    const float rightWallX = 1.5f;
    const float backWallZ= -1.5f;
    const float frontWallZ= 1.5f;

    const float restitution = 0.75f;
    const float damping = 0.999f;
    const float restingSpeed = 0.02f;
    const float groundFriction = 0.98f;
    const float horizontalRestingSpeed = 0.01f;
    const float particleRadius = 0.02f;
    const float floorThickness = 0.1f;
    const float wallThickness = 0.1f;
    const float wallHeight = 2.0f;
    const float wallDepth = 3.0f;
    const float particleRestitution = 1.0f; //0.85f;


    const glm::mat4 floorModelMatrix = glm::scale(
        glm::translate(
            glm::mat4(1.0f),
            glm::vec3(0.0f, floorY - floorThickness * 0.5f, 0.0f)
        ),
        glm::vec3(3.0f, floorThickness, 3.0f)
    );


    const glm::mat4 leftWallModelMatrix = glm::scale(
        glm::translate(
            glm::mat4(1.0f),
            glm::vec3(
                leftWallX - wallThickness * 0.5f,
                floorY + wallHeight * 0.5f,
                0.0f
            )
        ),
        glm::vec3(wallThickness, wallHeight, wallDepth)
    );

    const glm::mat4 rightWallModelMatrix = glm::scale(
        glm::translate(
            glm::mat4(1.0f),
            glm::vec3(
                rightWallX + wallThickness * 0.5f,
                floorY + wallHeight * 0.5f,
                0.0f
            )
        ),
        glm::vec3(wallThickness, wallHeight, wallDepth)
    );

    const glm::mat4 backWallModelMatrix = glm::scale(
        glm::translate(
            glm::mat4(1.0f),
            glm::vec3(
                0.0f, 
                floorY + wallHeight * 0.5f,
                backWallZ - wallThickness * 0.5f
            )
        ),
        glm::vec3(wallDepth, wallHeight, wallThickness)
    );


    while(glfwWindowShouldClose(window) == GLFW_FALSE)
    {
        glfwPollEvents();

        const float currentFrameTime = static_cast<float>(glfwGetTime());
        const float deltaTime = currentFrameTime - lastFrameTime;
        lastFrameTime = currentFrameTime;
        
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }

        const float cameraSpeed = 2.0f;

        const glm::vec3 forward = 
            glm::normalize(cameraTarget - cameraPosition);

        const glm::vec3 right =
            glm::normalize(glm::cross(forward, cameraUp)); 

        const glm::vec3 sidewaysMovement = right * cameraSpeed * deltaTime;
          

        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        {
            cameraPosition += forward * cameraSpeed * deltaTime;
        }

        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        {
            cameraPosition -= forward * cameraSpeed * deltaTime;
        }

        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        {
            cameraPosition -= right * cameraSpeed * deltaTime;
            cameraTarget -= sidewaysMovement;
        }

        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        {
            cameraPosition += right * cameraSpeed * deltaTime;
            cameraTarget += sidewaysMovement;
        }

        simulationAccumulator += deltaTime;

        while (simulationAccumulator >= simulationStep)
        {
            // Single particle moiton rules. 
            for (Particle& particle : particles)
            {
                // particle self status change 
                particle.acceleration = gravity;
                particle.velocity += particle.acceleration * simulationStep;
                particle.velocity *= damping;
                particle.position += particle.velocity * simulationStep;

                // particle interaction with floor
                if (particle.position.y <= floorY + particleRadius)
                {
                    particle.position.y = floorY + particleRadius;

                    if (particle.velocity.y < 0.0f) 
                    {
                        particle.velocity.y = -particle.velocity.y * restitution;
                    }

                    if (particle.velocity.y <= restingSpeed)
                    {
                        particle.velocity.y = 0.0f;

                        particle.velocity.x *= groundFriction;
                        particle.velocity.z *= groundFriction;

                        if(particle.velocity.x > -horizontalRestingSpeed &&
                           particle.velocity.x <  horizontalRestingSpeed)
                        {
                            particle.velocity.x = 0.0f;
                        }
                        
                        if(particle.velocity.z > -horizontalRestingSpeed &&
                           particle.velocity.z <  horizontalRestingSpeed)
                        {
                            particle.velocity.z = 0.0f;
                        }
                            
                    }
                }

                if(particle.position.x <= leftWallX + particleRadius)
                {
                    particle.position.x = leftWallX + particleRadius;

                    if(particle.velocity.x < 0.0f)
                    {
                        particle.velocity.x = -particle.velocity.x * restitution;
                    }
                }
                else if (particle.position.x >= rightWallX - particleRadius)
                {
                    particle.position.x = rightWallX - particleRadius;

                    if(particle.velocity.x > 0.0f)
                    {
                        particle.velocity.x = -particle.velocity.x * restitution;
                    }
                }

                if(particle.position.z <= backWallZ + particleRadius)
                {
                    particle.position.z = backWallZ + particleRadius;

                    if(particle.velocity.z < 0.0f)
                    {
                        particle.velocity.z = -particle.velocity.z * restitution;
                    }
                }
                else if(particle.position.z >= frontWallZ - particleRadius)
                {
                    particle.position.z = frontWallZ - particleRadius;

                    if(particle.velocity.z > 0.0f)
                    {
                        particle.velocity.z = -particle.velocity.z * restitution;
                    }
                }

            }

            // two particles collision rules:
            for(std::size_t firstIndex = 0;
                firstIndex < particles.size();
                ++firstIndex)

            {
                for(std::size_t secondIndex = firstIndex +1;
                    secondIndex < particles.size();
                    ++secondIndex )
                {
                    Particle& firstParticle = particles[firstIndex];
                    Particle& secondParticle = particles[secondIndex];

                    const glm::vec3 separation = 
                        secondParticle.position  - firstParticle.position;
                    
                    const float distance = glm::length(separation);
                    const float minimumDistance = 2.0f * particleRadius;

                    if(distance < minimumDistance)
                    {
                        glm::vec3 collisionNormal(1.0f, 0.0f, 0.0f);

                        if(distance > 0.0001f)
                        {
                            collisionNormal = separation/distance;
                        }

                        const float overlap = minimumDistance - distance;

                        firstParticle.position -=
                            collisionNormal * (overlap * 0.5f);

                        secondParticle.position +=
                            collisionNormal * (overlap * 0.5f);

                        const glm::vec3 relativeVelocity =
                            secondParticle.velocity - firstParticle.velocity;
                        
                        const float velocityAlongNormal = 
                            glm::dot(relativeVelocity, collisionNormal);

                        if(velocityAlongNormal < 0.0f)
                        {
                            const float impulseMagnitude = 
                                -(1.0f + particleRestitution) * velocityAlongNormal * 0.5f;
                            
                            const glm::vec3 impulse = 
                                impulseMagnitude * collisionNormal;


                            firstParticle.velocity -= impulse;
                            secondParticle.velocity += impulse;
                        }

                    }
                }
            }
            

            simulationAccumulator -= simulationStep;
        }

        particlePositions.clear();

        for (const Particle& particle : particles)
        {
            particlePositions.push_back(particle.position);
        }

        glBindBuffer(GL_ARRAY_BUFFER, particleVertexBuffer);

        glBufferSubData(
            GL_ARRAY_BUFFER,
            0,
            static_cast<GLsizeiptr>(particlePositions.size() * sizeof(glm::vec3)),
            particlePositions.data()
        );

        glBindBuffer(GL_ARRAY_BUFFER, 0);



        int framebufferWidth = 0;
        int framebufferHeight = 0;
        glfwGetFramebufferSize(window, &framebufferWidth , &framebufferHeight);

        if (framebufferWidth == 0 || framebufferHeight == 0)
        {
            continue;
        }


        glClearColor(0.04f, 0.07f, 0.12f, 1.0f);
        // glClear(GL_COLOR_BUFFER_BIT);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shaderProgram);

        const float elapsedSeconds = currentFrameTime;

        glm::mat4 modelMatrix(1.0f);

        modelMatrix = glm::rotate(
            modelMatrix,
            elapsedSeconds,
            glm::vec3(1.0f, 1.0f, 0.5f)
        );

        glm::mat4 viewMatrix = glm::lookAt(
            cameraPosition,
            cameraTarget,
            cameraUp
        );

        const float aspectRatio = 
        static_cast<float>(framebufferWidth)/ 
        static_cast<float>(framebufferHeight);

        glm::mat4 projectionMatrix = glm::perspective(
            glm::radians(45.0f),
            aspectRatio,
            0.1f,
            100.0f
        );

        glm::mat4 mvp =   
            projectionMatrix *
            viewMatrix *
            modelMatrix;

        glUniformMatrix4fv(
            mvpLocation,
            1, 
            GL_FALSE,
            glm::value_ptr(mvp)
        );

        glBindVertexArray(vertexArray);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // render floor
        const glm::mat4 floorMvp = projectionMatrix * viewMatrix * floorModelMatrix;
        glUniformMatrix4fv(
            mvpLocation,
            1, GL_FALSE,
            glm::value_ptr(floorMvp)
        );
        glBindVertexArray(vertexArray);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // render leftWall
        const glm::mat4 leftWallMvp = 
            projectionMatrix * viewMatrix * leftWallModelMatrix;
        
        glUniformMatrix4fv(
            mvpLocation,
            1,
            GL_FALSE,
            glm::value_ptr(leftWallMvp)
        );

        glBindVertexArray(vertexArray);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // render rightWall
        const glm::mat4 rightWallMvp = 
            projectionMatrix * viewMatrix * rightWallModelMatrix;
        
        glUniformMatrix4fv(
            mvpLocation,
            1, 
            GL_FALSE,
            glm::value_ptr(rightWallMvp)
        );

        glBindVertexArray(vertexArray);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // render bakcWall
        const glm::mat4 backWallMvp = 
            projectionMatrix * viewMatrix * backWallModelMatrix;

        glUniformMatrix4fv(
            mvpLocation,
            1,
            GL_FALSE,
            glm::value_ptr(backWallMvp)
        );

        glBindVertexArray(vertexArray);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // render particles.
        const glm::mat4 particleMvp = projectionMatrix * viewMatrix;

        glUniformMatrix4fv(
            mvpLocation,
            1,
            GL_FALSE,
            glm::value_ptr(particleMvp)
        );

        glPointSize(12.0f);
        glBindVertexArray(particleVertexArray);
        glDrawArrays(
            GL_POINTS,
            0,
            static_cast<GLsizei>(particles.size())
        );

        glfwSwapBuffers(window);
    }

    glDeleteProgram(shaderProgram);

    glDeleteBuffers(1, &particleVertexBuffer);
    glDeleteVertexArrays(1, &particleVertexArray);

    glDeleteBuffers(1, &vertexBuffer);
    glDeleteVertexArrays(1, &vertexArray);

    glfwDestroyWindow(window);
    glfwTerminate();

    return EXIT_SUCCESS;
}
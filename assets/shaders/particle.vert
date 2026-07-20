    #version 330 core

    layout (location = 0) in vec3 aPosition;
    layout (location = 1) in vec3 aNormal;
    layout (location = 2) in vec3 aInstancePosition;
    layout (location = 3) in vec3 aInstanceColor;

    uniform mat4 uMvp;
    uniform mat4 uModel;
    uniform bool uUseInstancePositions;
    uniform float uParticleRadius;

    out vec3 vertexNormal;
    out vec3 vertexColor;

    void main() 
    {
        vec3 position = aPosition;

        if (uUseInstancePositions)
        {
            position = aInstancePosition + aPosition * uParticleRadius;
        }

        gl_Position = uMvp * vec4(position, 1.0);

        mat3 normalMatrix = transpose(inverse(mat3(uModel)));

        vertexNormal = normalize(normalMatrix * aNormal);
        vertexColor = aInstanceColor;
    }

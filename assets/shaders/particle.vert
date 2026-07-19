    #version 330 core

    layout (location = 0) in vec3 aPosition;
    layout (location = 1) in vec3 aNormal;

    uniform mat4 uMvp;
    uniform mat4 uModel;

    out vec3 vertexNormal;

    void main() 
    {
        gl_Position = uMvp * vec4(aPosition, 1.0);

        mat3 normalMatrix = transpose(inverse(mat3(uModel)));

        vertexNormal = normalize(normalMatrix * aNormal);
    }
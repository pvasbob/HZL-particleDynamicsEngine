    #version 330 core

    layout (location = 0) in vec3 aPosition;
    uniform mat4 uMvp;
    out vec3 vertexColor;

    void main() 
    {
        gl_Position = uMvp * vec4(aPosition, 1.0);
        vertexColor = aPosition + vec3(0.5);
    }
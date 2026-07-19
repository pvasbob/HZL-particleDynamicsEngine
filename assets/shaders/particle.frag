    #version 330 core

    in vec3 vertexNormal;

    out vec4 fragmentColor;

    void main()
    {
        const vec3 materialColor = vec3(0.20, 0.75, 1.00);

        vec3 lightDirection = normalize(
            vec3(0.40, 1.0, 0.3)
        );

        const float ambientStrength = 0.25;

        float diffuseStrength = max(
            dot(normalize(vertexNormal), lightDirection),
            0.0
        );

        float lighting = 
            ambientStrength +
            (1.0 - ambientStrength) * diffuseStrength;

        fragmentColor = vec4(materialColor * lighting, 1.0);
    }
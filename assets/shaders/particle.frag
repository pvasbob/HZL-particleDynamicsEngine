    #version 330 core

    in vec3 vertexNormal;
    in vec3 vertexColor;

    uniform vec3 uMaterialColor;
    uniform bool uUseInstanceColors;

    out vec4 fragmentColor;

    void main()
    {


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

        vec3 materialColor = uUseInstanceColors
            ? vertexColor
            : uMaterialColor;

        fragmentColor = vec4(materialColor * lighting, 1.0);
    }

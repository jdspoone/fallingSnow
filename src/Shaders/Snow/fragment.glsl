#version 410 core

in vec4 position;
in vec4 middle;

out vec4 finalColor;

void main()
{
//    // Gaussian Transparency
//    float distance = pow(length(middle - position), 0.9);

    //White mode
    finalColor = vec4(1.0f, 1.0f, 1.0f, 1.0);
}
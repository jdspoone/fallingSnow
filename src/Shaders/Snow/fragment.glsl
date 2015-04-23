#version 410 core

in vec4 position;
in vec4 middle;

out vec4 finalColor;

void main()
{
    // Gaussian translucency
    float distance = pow(length(middle - position), 0.9);

	finalColor = vec4(0.95f, 0.95f, 0.95f, max(0.6f * (0.0016f - distance) / 0.001f, 0.00f));
}

#version 410 core

in vec4 position;
in vec4 middle;

out vec4 finalColor;

void main()
{
    //Colourful mode
    //finalColor = vec4(0.5/position.x, 0.5/position.y, 0.5/position.z, 0.2);
	float distance = pow(length(middle - position), 0.9);

    //White mode
    finalColor = vec4(1.0f, 1.0f, 1.0f, max(0.5f * (0.002f - distance) / 0.001f, 0.00f));
}

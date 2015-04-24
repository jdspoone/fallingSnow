#version 410 core

in vec3 position;
in vec3 velocity;
in float angle;

uniform mat4 MVP;

out float angleDegrees;

void main() {
    gl_Position = MVP * vec4(position, 1.0);
    angleDegrees = angle;
}


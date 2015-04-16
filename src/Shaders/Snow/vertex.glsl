#version 410 core

in vec3 position;
in vec3 velocity;

uniform mat4 MVP;

void main() {
    gl_Position = MVP * vec4(position, 1.0);
}


#version 410 core

in vec3 inVec;
out vec4 outVec;

uniform mat4 MVP;

void main() {
    gl_Position = MVP * vec4(inVec, 1.0);
    outVec = vec4(inVec, 1.0);
}


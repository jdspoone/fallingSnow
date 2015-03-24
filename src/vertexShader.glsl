#version 410 core

in vec4 inVec;
out vec4 outVec;

uniform mat4 MVP;

void main() {
    gl_Position = MVP * vec4(inVec.xyz, 1.0);
    outVec = inVec;
}


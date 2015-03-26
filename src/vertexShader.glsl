#version 410 core

in vec4 inVec;
out vec4 outVec;

uniform mat4 MVP;
uniform float wind;

void main() {
    vec3 p = vec3(1.0,0.0,0.0) * sin(wind);
    gl_Position = MVP * vec4(inVec.xyz + p, 1.0);
    outVec = inVec;
}


#version 410 core
in vec3 inVec;

uniform mat4 MVP;
out vec3 outVec;

void main() {
    gl_Position = MVP * vec4(inVec, 1.0);
    gl_PointSize = 5.0f; //will remove after geometry shader is implemented
	outVec = inVec + 0.5;
}

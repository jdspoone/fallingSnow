#version 410 core
in vec4 inVec;

uniform mat4 MVP;
out vec4 outVec;

void main() {
    gl_Position = MVP * inVec;
    gl_PointSize = 5.0f; //will remove after geometry shader is implemented
	outVec = inVec * 2;
}

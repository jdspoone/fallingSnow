#version 410 core
in vec3 inVec;

uniform mat4 MVP;
out vec3 outVec;

void main() {
    gl_Position = MVP * vec4(inVec, 1.0);
    gl_PointSize = 10.0f; //will remove after geometry shader is implemented
	outVec = vec3(inVec.x, inVec.y - 0.01, inVec.z);
}

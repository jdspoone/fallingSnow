#version 410 core
in vec4 inValue;

uniform mat4 MVP;
out vec4 outValue;

void main() {
    gl_Position = MVP * inValue;
    gl_PointSize = 2.0f; //will remove after geometry shader is implemented
	outValue = inValue;
}

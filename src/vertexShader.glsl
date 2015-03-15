#version 410 core
in vec4 inVec;

uniform mat4 MVP;
out vec4 outVec;

void main() {
    gl_Position = MVP * vec4(inVec.xyz, 1.0);
    gl_PointSize = 10.0f; //will remove after geometry shader is implemented
	outVec = vec4(inVec.x, inVec.y - inVec.w, inVec.z, inVec.w);
    if (outVec.y < -1){outVec.y = 1.00;}
}


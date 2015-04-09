#version 410 core

in vec3 inVec;

out vec3 outVec;

void main() {
    outVec = vec3(inVec.x, inVec.y - 0.0005, inVec.z);
    if (outVec.y < -1) {
      outVec.y = 1.00;
    }
}


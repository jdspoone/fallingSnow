#version 410 core

in vec3 previousPosition;

out vec3 nextPosition;

void main() {

  nextPosition = vec3(previousPosition.x, previousPosition.y - 0.0001, previousPosition.z);

  if (nextPosition.y < -1) {
    nextPosition.y = 1.00;
  }
}


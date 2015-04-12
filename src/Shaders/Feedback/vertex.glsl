#version 410 core

in vec3 previousPosition;
in vec3 previousVelocity;

out vec3 nextPosition;

void main() {

  nextPosition = previousPosition + previousVelocity;

  if (nextPosition.y < -1) {
    nextPosition.y = 1.00;
  }
}


#version 410 core

in vec3 previousPosition;
in vec3 previousVelocity;

out vec3 nextPosition;
out vec3 nextVelocity;

void main() {

  nextPosition = previousPosition + previousVelocity;

  if (nextPosition.y < 0) {
    nextPosition.y = 1.00;
  }
  
  nextVelocity = previousVelocity;
}


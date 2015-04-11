#version 410 core

in vec3 position;
in vec3 velocity;

out vec3 nextPosition;
out vec3 nextVelocity;

void main() {

  nextPosition = position + velocity;

  if (nextPosition.y < -1) {
    nextPosition.y = 1.00;
  }
}


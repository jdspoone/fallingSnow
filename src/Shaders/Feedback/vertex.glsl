#version 410 core

in vec3 previousPosition;
in vec3 previousVelocity;
in float previousAngle;

out vec3 nextPosition;
out vec3 nextVelocity;
out float nextAngle;

void main() {

  nextPosition = previousPosition + previousVelocity;

  if (nextPosition.y < 0) {
    nextPosition.y = 1.00;
  }
 
  nextVelocity = previousVelocity;
  
  nextAngle = previousAngle + 1;
}


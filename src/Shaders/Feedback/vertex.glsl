#version 410 core

in vec3 previousPosition;
in vec3 previousVelocity;

out vec3 nextPosition;
out vec3 nextVelocity;

uniform sampler2D windTex;

void main() {
  vec2 index = previousPosition.xz;
  index.x = (index.x + 1.0) / 2.0;
  index.y = (index.y + 1.0) / 2.0;
  nextVelocity = texture(windTex, index).xyz;
  nextVelocity = mix(nextVelocity, previousVelocity, 0.9);

  nextPosition = previousPosition - previousVelocity;

  if (nextPosition.y < 0) {
    nextPosition.y = 1.00;
  }
  if (nextPosition.x < -1.0) {
    nextPosition.x += 2.0;
  }
  if (nextPosition.x > 1.0) {
    nextPosition.x -= 2.0;
  }
  if (nextPosition.z < -1.0) {
    nextPosition.z += 2.0;
  }
  if (nextPosition.z > 1.0) {
    nextPosition.z -= 2.0;
  }
}


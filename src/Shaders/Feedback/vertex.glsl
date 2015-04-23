#version 410 core

in vec3 previousPosition;
in vec3 previousVelocity;
in float previousAngle;

out vec3 nextPosition;
out vec3 nextVelocity;
out float nextAngle;

uniform sampler3D windTex;
uniform float numParticles = 1;
uniform float windSpeed;

void main() {
  // Put the texture index into the range [0, 1]
  vec3 index = previousPosition.xzy;
  index.x = (index.x + 1.0) / 2.0;
  index.y = (index.y + 1.0) / 2.0;
  index.z = (index.z + 1.0) / 2.0;

  // Calculate the next velocity by interpolating between the previous velocity and the new one.
  nextVelocity = texture(windTex, index).xyz / 1000.0;
  nextVelocity = mix(nextVelocity, previousVelocity, min(0.9, max(0.1, gl_VertexID / numParticles)));

  // Apply the wind speed scalar to next velocity
  nextVelocity = windSpeed * nextVelocity;

  // Where the particle will be for the next draw call.
  nextPosition = previousPosition - nextVelocity;
  nextAngle = previousAngle + length(10000 * nextVelocity);
  nextVelocity = previousVelocity;

  // Loop particles if they're out of range.
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


#version 410 core
in vec4 previousPosition;

uniform mat4 MVP;

void main() {
  // Apply the Model View Projection transformation to the given point
  gl_Position = MVP * vec4(previousPosition.xyz, 1.0);
  gl_PointSize = 10.0f;
}


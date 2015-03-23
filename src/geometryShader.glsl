#version 410 core

layout(points) in;

uniform vec3 cameraPosition;

layout(triangle_strip, max_vertices = 4) out;

void main() {
  // Emit a square centered at the given point
  gl_Position = gl_in[0].gl_Position + vec4(-0.05, -0.05, 0.0, 0.0);
  EmitVertex();

  gl_Position = gl_in[0].gl_Position + vec4(-0.05, 0.05, 0.0, 0.0);
  EmitVertex();

  gl_Position = gl_in[0].gl_Position + vec4(0.05, -0.05, 0.0, 0.0);
  EmitVertex();

  gl_Position = gl_in[0].gl_Position + vec4(0.05, 0.05, 0.0, 0.0);
  EmitVertex();

  EndPrimitive();
}

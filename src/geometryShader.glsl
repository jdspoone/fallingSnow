#version 410 core

layout(points) in;

uniform vec3 cameraPosition;

in vec4 outVec[];
out vec4 v;

layout(triangle_strip, max_vertices = 4) out;

void main() {
  float r = 0.001f; //Radius of snowflake

  // Emit a square centered at the given point
  gl_Position = gl_in[0].gl_Position + vec4(-r, -r, 0.0, 0.0);
  v = outVec[0] + vec4(-r, -r, 0.0, 0.0);
  EmitVertex();
  
  gl_Position = gl_in[0].gl_Position + vec4(-r, r, 0.0, 0.0);
  v = outVec[0] + vec4(-r, r, 0.0, 0.0);
  EmitVertex();

  gl_Position = gl_in[0].gl_Position + vec4(r, -r, 0.0, 0.0);
  v = outVec[0] + vec4(r, -r, 0.0, 0.0);
  EmitVertex();

  gl_Position = gl_in[0].gl_Position + vec4(r, r, 0.0, 0.0);
  v = outVec[0] + vec4(r, r, 0.0, 0.0);
  EmitVertex();


  EndPrimitive();
}

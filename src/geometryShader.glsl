#version 410 core

layout(points) in;

uniform vec3 cameraPosition;

in vec4 outVec[];
out vec4 position;
out vec4 middle;

layout(triangle_strip, max_vertices = 4) out;

void main() {
  float r = 0.001f; //Radius of snowflake
  middle = outVec[0];

  // Emit a square centered at the given point
  gl_Position = gl_in[0].gl_Position + vec4(-r, -r, 0.0, 0.0);
  position = outVec[0] + vec4(-r, -r, 0.0, 0.0);
  EmitVertex();
  
  gl_Position = gl_in[0].gl_Position + vec4(-r, r, 0.0, 0.0);
  position = outVec[0] + vec4(-r, r, 0.0, 0.0);
  EmitVertex();

  gl_Position = gl_in[0].gl_Position + vec4(r, -r, 0.0, 0.0);
  position = outVec[0] + vec4(r, -r, 0.0, 0.0);
  EmitVertex();

  gl_Position = gl_in[0].gl_Position + vec4(r, r, 0.0, 0.0);
  position = outVec[0] + vec4(r, r, 0.0, 0.0);
  EmitVertex();


  EndPrimitive();
}

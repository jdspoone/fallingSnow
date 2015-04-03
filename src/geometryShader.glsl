#version 410 core

layout(points) in;

uniform vec3 cameraPosition;

in vec4 outVec[];
out vec4 position;
out vec4 middle;

/*
 *  Note: While the only valid output types for the geometry shader are points, 
 *        line_strip and triangle_strip, you can emit a series of independent 
 *        lines or triangles by calling EndPrimitive() after emitting 2 or 3 vertices
 */

layout(triangle_strip, max_vertices = 3) out;

void main() {
  float r = 0.001f; //Radius of snowflake
  middle = outVec[0];

  mat4 rotationMatrix = mat4(
    cos(0), -sin(0), 0, 0,
    sin(0), cos(0), 0, 0,
    0, 0, 1, 0,
    0, 0, 0, 1
  );

  // Emit an equilateral triangle centered at the given point
  gl_Position = gl_in[0].gl_Position + (rotationMatrix * vec4(-r / 2.0, -r / 2.0, 0.0, 0.0));
  position = outVec[0] + vec4(-r / 2.0, -r / 2.0, 0.0, 0.0);
  EmitVertex();
  
  gl_Position = gl_in[0].gl_Position + (rotationMatrix * vec4(r / 2.0, -r / 2.0, 0.0, 0.0));
  position = outVec[0] + vec4(r / 2.0, -r / 2.0, 0.0, 0.0);
  EmitVertex();
  
  gl_Position = gl_in[0].gl_Position + (rotationMatrix * vec4(0.0, r/2, 0.0, 0.0));
  position = outVec[0] + vec4(0.0, r/2, 0.0, 0.0);
  EmitVertex();

  EndPrimitive();
}

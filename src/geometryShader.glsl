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

#define M_PI 3.1415926535897932384626433832795


// This function emits an equilateral triangle centered at the given point
void emitEquilateralTriangle(vec4 pointMVP, vec4 pointM, float radius, float angle) {

  mat4 rotationMatrix = mat4(
    cos(angle), -sin(angle), 0, 0,
    sin(angle), cos(angle), 0, 0,
    0, 0, 1, 0,
    0, 0, 0, 1
  );

  gl_Position = pointMVP + (rotationMatrix * vec4(radius * cos(0), radius * sin(0), 0.0, 0.0));
  position = pointM + (rotationMatrix * vec4(radius * cos(0), radius * sin(0), 0.0, 0.0));
  EmitVertex();
  
  gl_Position = pointMVP + (rotationMatrix * vec4(radius * cos(120 * M_PI / 180), radius * sin(120 * M_PI / 180), 0.0, 0.0));
  position = pointM + (rotationMatrix * vec4(radius * cos(120 * M_PI / 180), radius * sin(120 * M_PI / 180), 0.0, 0.0));
  EmitVertex();
  
  gl_Position = pointMVP + (rotationMatrix * vec4(radius * cos(240 * M_PI / 180), radius * sin(240 * M_PI / 180), 0.0, 0.0));
  position = pointM + (rotationMatrix * vec4(radius * cos(240 * M_PI / 180), radius * sin(240 * M_PI / 180), 0.0, 0.0));
  EmitVertex();

  EndPrimitive();
}


void main() {
  float r = 0.001f; //Radius of snowflake
  middle = outVec[0];
  float angle = -M_PI / 2.0;

  emitEquilateralTriangle(gl_in[0].gl_Position, outVec[0], r, angle);
}
#version 410 core

layout(points) in;

uniform vec3 cameraPosition;

uniform vec4 firstTriangleVertex;
uniform vec4 secondTriangleVertex;
uniform vec4 thirdTriangleVertex;

out vec4 position;
out vec4 middle;

/*
 *  Note: While the only valid output types for the geometry shader are points, 
 *        line_strip and triangle_strip, you can emit a series of independent 
 *        lines or triangles by calling EndPrimitive() after emitting 2 or 3 vertices
 */

layout(triangle_strip, max_vertices = 6) out;

#define M_PI 3.1415926535897932384626433832795


// This function emits an equilateral triangle centered at the given point, where the circumscribed circle has the given radius
void emitEquilateralTriangle(float radius, float angle, vec4 point) {

  mat4 rotationMatrix = mat4(
    cos(angle), -sin(angle), 0, 0,
    sin(angle), cos(angle), 0, 0,
    0, 0, 1, 0,
    0, 0, 0, 1
  );

  gl_Position = point + (rotationMatrix * (radius * firstTriangleVertex));
  position = gl_Position;
  EmitVertex();
  
  gl_Position = point + (rotationMatrix * (radius * secondTriangleVertex));
  position = gl_Position;
  EmitVertex();
  
  gl_Position = point + (rotationMatrix * (radius * thirdTriangleVertex));
  position = gl_Position;
  EmitVertex();

  EndPrimitive();
}


void emitSnowflake(int level, float radius, float upAngle, vec4 point) {

  switch (level) {
    case 1:
      emitEquilateralTriangle(radius, upAngle, point);
      emitEquilateralTriangle(radius, -upAngle, point);
      break;

    default:
      emitEquilateralTriangle(radius, upAngle, point);
      break;
  }
}


void main() {
  float radius = 0.001f;
  middle = gl_in[0].gl_Position;

  emitSnowflake(1, radius, -M_PI / 2.0, gl_in[0].gl_Position);
}
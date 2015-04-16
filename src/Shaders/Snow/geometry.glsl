#version 410 core

layout(points) in;

uniform vec3 cameraPosition;

out vec4 position;
out vec4 middle;

/*
 *  Note: While the only valid output types for the geometry shader are points, 
 *        line_strip and triangle_strip, you can emit a series of independent 
 *        lines or triangles by calling EndPrimitive() after emitting 2 or 3 vertices
 */

layout(triangle_strip, max_vertices = 6) out;

#define M_PI 3.1415926535897932384626433832795


// This function emits an equilateral triangle centered at the given point
void emitEquilateralTriangle(float radius, float angle, vec4 point) {

  mat4 rotationMatrix = mat4(
    cos(angle), -sin(angle), 0, 0,
    sin(angle), cos(angle), 0, 0,
    0, 0, 1, 0,
    0, 0, 0, 1
  );

  gl_Position = point + (rotationMatrix * (radius * vec4(cos(0), sin(0), 0.0, 0.0)));
  position = gl_Position;
  EmitVertex();
  
  gl_Position = point + (rotationMatrix * (radius * vec4(cos(120 * M_PI / 180), sin(120 * M_PI / 180), 0.0, 0.0)));
  position = gl_Position;
  EmitVertex();
  
  gl_Position = point + (rotationMatrix * (radius * vec4(cos(240 * M_PI / 180), sin(240 * M_PI / 180), 0.0, 0.0)));
  position = gl_Position;
  EmitVertex();

  EndPrimitive();
}


void kochSnowflake(int level, float radius, float upAngle, vec4 point) {

  switch (level) {
    case 1:
      emitEquilateralTriangle(radius, -M_PI / 2.0, point);
      emitEquilateralTriangle(radius, M_PI / 2.0, point);
      break;

    default:
      emitEquilateralTriangle(radius, -M_PI / 2.0, point);
      break;
  }
}

void main() {
  float radius = 0.001f;
  middle = gl_in[0].gl_Position;

  kochSnowflake(1, radius, -M_PI / 2.0, gl_in[0].gl_Position);
}
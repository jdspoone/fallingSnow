#version 410 core

out vec4 finalColor;
in vec4 v;

void main()
{
  //Black, opaque
  finalColor = vec4(0.5/v.x, 0.5/v.y, 0.5/v.z, 0.2);
}

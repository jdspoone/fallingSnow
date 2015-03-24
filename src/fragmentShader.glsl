#version 410 core

out vec4 finalColor;
in vec4 v;

void main()
{
  //Colourful mode
  //finalColor = vec4(0.5/v.x, 0.5/v.y, 0.5/v.z, 0.2);

  //White mode
  finalColor = vec4(1.0f, 1.0f, 1.0f, 0.2);
}

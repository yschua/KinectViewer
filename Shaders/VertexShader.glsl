#version 440 core

in layout(location=0) vec3 position;
in layout(location=1) vec3 in_color;

uniform mat4 mvpMatrix;

out vec3 color;

void main()
{
  gl_Position = mvpMatrix * vec4(position, 1.0);
  color = in_color;
}
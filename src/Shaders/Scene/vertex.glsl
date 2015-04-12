#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 texture;

out vec3 v;
out vec3 tex;
out vec3 n = normal;
 
uniform mat4 MVP;
 
void main(){
 
    gl_Position =  MVP * vec4(position,1);
	v = position;
    n = normal; 
	tex = texture;
}


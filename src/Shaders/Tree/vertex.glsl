#version 330 core

layout(location = 0) in vec4 position;
layout(location = 1) in vec3 normal;

out vec4 v;
out vec3 n;
 
uniform mat4 MVP;
 
void main(){
 
    gl_Position =  MVP * position;
	v = position;
    n = normal; 
}


#version 330 core

layout(location = 0) in vec3 vert;
layout(location = 1) in vec2 uv;
 
out vec3 v;
out vec2 tex;
 
uniform mat4 MVP;
 
void main(){
 
    gl_Position =  MVP * vec4(vert,1);
	v = vert; 
	tex = uv;
}


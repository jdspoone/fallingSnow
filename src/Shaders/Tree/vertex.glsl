#version 330 core

layout(location = 0) in vec4 position;
layout(location = 1) in vec3 normal;

out vec3 n;
out vec3 l;
 
uniform mat4 MVP;
uniform mat4 VP;
uniform vec3 light_position;
 
void main(){
 
    gl_Position =  MVP * position;
    n =  mat3(VP) * normal;
    vec4 p = VP * position;
    l = light_position - p.xyz;
}


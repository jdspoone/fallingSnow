#version 330 core 
in vec3 v;
in vec2 tex;

out vec4 finalColor;

uniform sampler2D texUnit;

void main() {
    v; tex;
	finalColor = texture(texUnit, tex);
	//finalColor = vec4(0.0,0.0,0.0,1.0);
}


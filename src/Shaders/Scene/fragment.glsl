#version 330 core 
in vec3 v;
in vec3 tex;
in vec3 n;

out vec4 finalColor;

uniform sampler2D texUnit;
uniform vec3 cameraPosition;
uniform vec4 clearColor;

void main() {
    vec2 UV = tex.xy; 
	finalColor = texture(texUnit, UV);
	finalColor = mix(clearColor, finalColor, min(1.0, pow(1 / length(cameraPosition - v), 2)));
}


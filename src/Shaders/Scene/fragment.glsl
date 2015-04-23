#version 330 core 
in vec3 v;
in vec3 tex;
in vec3 n;

out vec4 finalColor;

uniform sampler2D texUnit;

void main() {
    v;
    vec2 UV = tex.xy;
    if (tex.z > 0)
        finalColor = texture(texUnit, UV);
    else
        finalColor = vec4(0.0,0.0,0.0,1.0);
}


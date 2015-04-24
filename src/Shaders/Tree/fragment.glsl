#version 330 core 
in vec3 v;
in vec3 n;
in vec3 l;

out vec4 finalColor;

// Material properties
uniform vec3 diffuse_albedo = vec3(1.0); //color of light
uniform vec3 ambient = vec3(0.1, 0.1, 0.1);        //color of ambient
uniform float power = 0.04;
uniform vec3 cameraPosition;
uniform vec4 clearColor;
uniform float numParticles;

void main() {
    vec3 N = normalize(n);
    vec3 L = normalize(l);

    vec3 diffuse = max(dot(N, L), 0.0) * diffuse_albedo * power;
    vec3 color = vec3(0.0f, 0.3f, 0.0f); //Green 
   
	finalColor = vec4(ambient + diffuse + color, 1.0);
	finalColor = mix(finalColor, clearColor, min(1.0, (numParticles / 5000000) * pow(v.z, 2)));
}


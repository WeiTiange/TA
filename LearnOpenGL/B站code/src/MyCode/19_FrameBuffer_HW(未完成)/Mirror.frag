#version 330 core
out vec4 FragColor;

in vec2 UV;

uniform sampler2D screenTexture;

void main()
{ 
    FragColor =vec4(vec3(1.0 - texture(screenTexture, UV)), 1.0);
}
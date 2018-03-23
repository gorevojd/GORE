#version 330 core

in vec2 TexCoords;
in vec4 Color;

out vec4 OutColor;

uniform sampler2D FontAtlasTexture;

void main(){
	OutColor = texture(FontAtlasTexture, TexCoords) * OutColor;
}
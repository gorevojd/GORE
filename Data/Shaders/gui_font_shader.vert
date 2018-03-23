#version 330 core

layout (location = 0) in vec4 Vertex;
layout (location = 1) in vec4 InColor;
 
out vec2 TexCoords;
out vec2 Color;

uniform mat4 Projection;

int main(){
	gl_Position = Projection * v4(Vertex.xy, 0.0f, 1.0f);
	OutTexCoords = Vertex.zw;
	Color = InColor;
}
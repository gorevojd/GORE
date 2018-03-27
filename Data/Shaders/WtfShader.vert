#version 330 core

layout (location = 0) in vec3 Position;
layout (location = 1) in vec2 UV;
layout (location = 2) in vec3 Normal;
layout (location = 3) in vec3 Color;

uniform mat4 Projection;
uniform mat4 View;
uniform mat4 Model;

out vec3 FragmentColor;
out vec2 FragmentUV;
out vec3 FragmentWorldP;
out vec3 FragmentWorldN;

void main(){
	gl_Position = Projection * View * Model * vec4(Position.xyz, 1.0f);
	FragmentColor = Color;

	FragmentWorldP = (Model * vec4(Position.xyz, 1.0f)).xyz;
	FragmentWorldN = Normal;
	FragmentUV = UV;
}
#version 330 core

layout (location = 0) in vec3 Position;
layout (location = 1) in vec2 UV;
layout (location = 2) in vec3 Normal;

uniform mat4 Projection;
uniform mat4 View;
uniform mat4 Model;

void main(){
	//gl_Position = Projection * View * Model * vec4(Position, 1.0f);
	gl_Position = Projection * View * Model * vec4(Position.xyz, 1.0f);
}
#version 330 core

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec2 inTexCoords;

uniform mat4 Projection;
uniform mat4 View;

out VS_OUT{
	vec3 FragPos;
	vec2 TexCoords;
}vs_out;

void main(){
	vs_out.FragPos = inPosition;
	vs_out.TexCoords = inTexCoords;

	gl_Position = Projection * View * vec4(vs_out.FragPos, 1.0f);
}

#version 330 core

layout (location = 0) in vec3 inPos;
layout (location = 2) in uint inColor;

uniform mat4 Projection;
uniform mat4 View;
uniform mat4 Model;
uniform samplerBuffer NormalsBuffer;

out VS_OUT{
	flat vec3 N;
	vec2 UV;
	vec3 FragPos;
	vec3 Color;
	float Visibility;
}vs_out;

void main(){
	vec4 WorldPosition = Model * vec4(inPos.xyz, 1.0);
	vec4 ViewPosition = View * WorldPosition;
	float CamToVertexLen = length(ViewPosition.xyz);

	vec3 FetchedNormal = texelFetch(NormalsBuffer, gl_VertexID / 3).xyz;

	vec4 InColorVector = vec4(
		(inColor >> 24u) & 255u,
		(inColor >> 16u) & 255u,
		(inColor >> 8u) & 255u, 
		inColor & 255u);

	InColorVector /= 255.0;

	vs_out.FragPos = WorldPosition.xyz;
	vs_out.UV = vec2(0.0, 0.0);
	vs_out.N = FetchedNormal;
	vs_out.Visibility = 0.0;
	vs_out.Color = InColorVector.xyz;

	//gl_Position = Projection * ViewPosition;
	gl_Position = Projection * View * Model * vec4(inPos.xyz, 1.0);
}
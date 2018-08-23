#version 330 core

layout (location = 0) in vec2 inPosition;
layout (location = 1) in ivec4 inOffsetsToOtherPoints;

uniform float WaterLevel;
uniform float GlobalTime;
uniform float PerVertexOffset;

uniform mat4 Projection;
uniform mat4 View;
uniform mat4 Model;

out VS_OUT{
	vec3 FragPos;
	flat vec3 N;
}vs_out;

float WaterWavesAmplitude = 0.3;
float Pi = 3.14159265359;

vec3 NoiseFunc(vec3 VertOffset){
	vec3 Result = vec3(0.0);

#if 1

	Result.x = (sin(VertOffset.x * Pi * 0.4 + GlobalTime) + cos(VertOffset.z * 0.4 + GlobalTime)) * 0.3;
	Result.y = (sin(VertOffset.x * Pi * 0.4 + GlobalTime) + cos(VertOffset.z * 0.5 + GlobalTime * 0.7)) * WaterWavesAmplitude;
	Result.z = (sin(VertOffset.x * Pi * 0.3 + GlobalTime) + cos(VertOffset.z * 0.3 + GlobalTime)) * 0.2;
#endif

	return(Result);
}

void main(){

	vec3 ChunkTransl = Model._m30_m31_m32;
	vec3 VertOffset = ChunkTransl + vec3(inPosition.x, 0.0, inPosition.y);

	vec3 VertNoiseOffset = NoiseFunc(VertOffset);

	vec4 WorldPosition = Model * vec4(vec3(inPosition.x, WaterLevel, inPosition.y) + VertNoiseOffset, 1.0);
	vec4 ViewPosition = View * WorldPosition;

	gl_Position = Projection * ViewPosition;

	vec3 OffsetVector1 = vec3(
		float(inOffsetsToOtherPoints.x) * PerVertexOffset,
		0.0,
		float(inOffsetsToOtherPoints.y) * PerVertexOffset);
	vec3 TriPoint1 = VertOffset + OffsetVector1;
	TriPoint1.y += WaterLevel;
	TriPoint1 += NoiseFunc(TriPoint1);
	OffsetVector1 = TriPoint1 - WorldPosition.xyz;

	vec3 OffsetVector2 = vec3(
		float(inOffsetsToOtherPoints.z) * PerVertexOffset,
		0.0,
		float(inOffsetsToOtherPoints.w) * PerVertexOffset);
	vec3 TriPoint2 = VertOffset + OffsetVector2;
	TriPoint2.y += WaterLevel;
	TriPoint2 += NoiseFunc(TriPoint2);
	OffsetVector2 = TriPoint2 - WorldPosition.xyz;

	vs_out.N = normalize(cross(OffsetVector1, OffsetVector2));
	vs_out.FragPos = WorldPosition.xyz;
}
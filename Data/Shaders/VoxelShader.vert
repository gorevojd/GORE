#version 330 core

#define TEXTURES_PER_WIDTH 16

layout (location = 0) in uint VertexData;

out VS_OUT{
	vec3 N;
	vec2 UV;
	vec3 FragPos;
	float Visibility;
}vs_out;

uniform mat4 Projection;
uniform mat4 View;
uniform mat4 Model;

float FogDensity = 0.002;
float FogGradient = 3.0f;

void main(){
#if 0
	uint VertexData1 = 
		(VertexData << 24u) |
		(VertexData >> 24u) |
		(((VertexData >> 8u) & 255u) << 16u)|
		(((VertexData >> 16u) & 255u) << 8u);
#else
	uint VertexData1 = VertexData;
#endif

	vec3 NormalTable[6];
	NormalTable[0] = vec3(0.0f, 1.0f, 0.0f);
	NormalTable[1] = vec3(0.0f, -1.0f, 0.0f);
	NormalTable[2] = vec3(-1.0f, 0.0f, 0.0f);
	NormalTable[3] = vec3(1.0f, 0.0f, 0.0f);
	NormalTable[4] = vec3(0.0f, 0.0f, -1.0f);
	NormalTable[5] = vec3(0.0f, 0.0f, 1.0f);

	vec3 Pos;
	Pos.x = float((VertexData1 >> 27u) & 31u);
	Pos.y = float((VertexData1 >> 18u) & 511u);
	Pos.z = float((VertexData1 >> 13u) & 31u);

	int ExtractedNormalIndex = int((VertexData1 >> 10u) & 7u);
	vec3 Norm = NormalTable[ExtractedNormalIndex];

	int ExtractedTextureIndex = int((VertexData1 >> 2u) & 255u);
	int ExtractedTexVertType = int(VertexData1 & 3u);

	float Epsilon = 0.000f;
	float TwoEpsilon = 2.0f * Epsilon;

	vec2 TexCoord;
	float OneTextureDelta = 1.0f / float(TEXTURES_PER_WIDTH);
	TexCoord.x = (ExtractedTextureIndex % TEXTURES_PER_WIDTH) * OneTextureDelta + Epsilon;
	TexCoord.y = (ExtractedTextureIndex / TEXTURES_PER_WIDTH) * OneTextureDelta + Epsilon;

	if(ExtractedTexVertType == 1){
		TexCoord.x += OneTextureDelta - TwoEpsilon;
	}
	else if(ExtractedTexVertType == 2){
		TexCoord.x += OneTextureDelta - TwoEpsilon;
		TexCoord.y += OneTextureDelta - TwoEpsilon;
	}
	else if(ExtractedTexVertType == 3){
		TexCoord.y += OneTextureDelta - TwoEpsilon;
	}

	vec4 WorldPosition = Model * vec4(Pos.xyz, 1.0f);
	vec4 ViewPosition = View * WorldPosition;
	float CamToVertexLen = length(ViewPosition.xyz);
	vs_out.Visibility = exp(-pow((FogDensity * CamToVertexLen), FogGradient));
	vs_out.Visibility = clamp(vs_out.Visibility, 0.0f, 1.0f);

	gl_Position = Projection * ViewPosition;

	mat3 NormalMatrix = mat3(transpose(inverse(Model)));
	vs_out.N = NormalMatrix * Norm;
	vs_out.FragPos = vec3(Model * vec4(Pos.xyz, 1.0f));
	vs_out.UV = vec2(TexCoord.x, TexCoord.y);
}
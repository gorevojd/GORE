#version 330 core

#define TEXTURES_PER_WIDTH 16

layout (location = 0) in uint VertexData;

out VS_OUT{
	vec3 N;
	vec2 UV;
	vec3 FragPos;
}vs_out;

uniform mat4 Projection;
uniform mat4 View;
uniform mat4 Model;

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

	vec2 TexCoord;
	float OneTextureDelta = 1.0f / float(TEXTURES_PER_WIDTH);
	TexCoord.x = ((TEXTURES_PER_WIDTH - 1) & ExtractedTextureIndex) * OneTextureDelta;
	TexCoord.y = (ExtractedTextureIndex / TEXTURES_PER_WIDTH) * OneTextureDelta;

	if(ExtractedTexVertType == 1){
		TexCoord.x += OneTextureDelta;
	}
	else if(ExtractedTexVertType == 2){
		TexCoord.x += OneTextureDelta;
		TexCoord.y += OneTextureDelta;
	}
	else if(ExtractedTexVertType == 3){
		TexCoord.y += OneTextureDelta;
	}

	gl_Position = Projection * View * Model * vec4(Pos.xyz, 1.0f);

	mat3 NormalMatrix = mat3(transpose(inverse(Model)));
	vs_out.N = NormalMatrix * Norm;
	vs_out.FragPos = vec3(Model * vec4(Pos.xyz, 1.0f));
	vs_out.UV = vec2(TexCoord.x, TexCoord.y);
}
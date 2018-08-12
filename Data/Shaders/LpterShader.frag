#version 330 core

in VS_OUT{
	flat vec3 N;
	vec2 UV;
	vec3 FragPos;
	vec3 Color;
	float Visibility;
}fs_in;

out vec4 OutColor;

struct dir_light{
	vec3 Direction;
	vec3 Diffuse;
	vec3 Ambient;
};

uniform vec3 CameraP;
uniform dir_light DirLight;
uniform vec3 FogColor;

vec3 CalcDirLight(dir_light Lit, vec3 Normal, vec3 ViewDir, vec3 SampledDiffColor){
	vec3 LightDir = normalize(Lit.Direction);
	vec3 ReflectDir = normalize(reflect(LightDir, Normal));
	vec3 HalfWayDir = normalize(-ViewDir + ReflectDir);

	float Diff = max(dot(Normal, -LightDir), 0.0f);

	vec3 Ambient = Lit.Ambient * SampledDiffColor.xyz;
	vec3 Diffuse = Lit.Diffuse * Diff * SampledDiffColor.xyz;

	return(Ambient + Diffuse);
}

void main(){
	vec3 Result = vec3(0.0f, 0.0f, 0.0f);

	vec3 ViewDir = normalize(fs_in.FragPos - CameraP);
	Result += CalcDirLight(DirLight, fs_in.N, ViewDir, fs_in.Color);

	//Result = (1.0f - fs_in.Visibility) * FogColor +  fs_in.Visibility * Result, 

	OutColor = vec4(Result.xyz, 1.0);
}
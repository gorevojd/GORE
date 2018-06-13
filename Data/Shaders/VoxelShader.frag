#version 330 core

in VS_OUT{
	vec3 N;
	vec2 UV;
	vec3 FragPos;
}fs_in;

out vec4 OutColor;

struct dir_light{
	vec3 Direction;
	vec3 Diffuse;
	vec3 Ambient;
};

uniform vec3 CameraP;
uniform sampler2D DiffuseMap;
uniform dir_light DirLight;

vec3 CalcDirLight(dir_light Lit, vec3 Normal, vec3 ViewDir){
	vec3 LightDir = normalize(Lit.Direction);
	vec3 ReflectDir = normalize(reflect(LightDir, Normal));
	vec3 HalfWayDir = normalize(-ViewDir + ReflectDir);

	vec4 SampledColor = texture(DiffuseMap, fs_in.UV);

	float Diff = max(dot(Normal, -LightDir), 0.0f);

	vec3 Ambient = Lit.Ambient * SampledColor.xyz;
	vec3 Diffuse = Lit.Diffuse * Diff * SampledColor.xyz;

	return(Ambient + Diffuse);
}

void main(){
	vec3 Result = vec3(0.0f, 0.0f, 0.0f);

	vec3 ViewDir = normalize(fs_in.FragPos - CameraP);
	Result += CalcDirLight(DirLight, fs_in.N, ViewDir);

	OutColor = vec4(Result, 1.0f);
	//OutColor = texture(DiffuseMap, fs_in.UV);
}
#version 330 core

in VS_OUT{
	vec3 FragPos;
	flat vec3 N;
}fs_in;

out vec4 FragColor;

struct dir_light{
	vec3 Direction;
	vec3 Diffuse;
};

uniform vec3 CameraP;
uniform dir_light DirLight;

vec3 CalcDirLight(dir_light Lit, vec3 Normal, vec3 ViewDir, vec3 DiffColor){
	vec3 LightDir = normalize(Lit.Direction);
	vec3 ReflectDir = normalize(reflect(LightDir, Normal));

	float Diff = max(dot(Normal, -LightDir), 0.0);

	vec3 Diffuse = Lit.Diffuse * Diff * DiffColor;

	return(Diffuse);
}

void main(){
	vec3 Result = vec3(0.0, 0.0, 0.0);
	vec3 ViewDir = normalize(fs_in.FragPos - CameraP);
	vec3 WaterColor = vec3(0.0, 0.0, 1.0);

	Result += CalcDirLight(DirLight, fs_in.N, ViewDir, WaterColor);

	FragColor = vec4(Result, 0.5);
}
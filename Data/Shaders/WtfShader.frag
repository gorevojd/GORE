#version 330 core

in vec3 VertColor;
in vec3 FragmentWorldP;
in vec3 FragmentWorldN;

out vec4 FragmentColor;

uniform vec3 CameraP;

struct point_light{
	vec3 P;
	vec3 Color;
	float Radius;
} Light1;

vec3 CalculatePointLight(point_light Lit){
	vec3 Result = vec3(0.0f, 0.0f, 0.0f);

	vec3 ToLight = Lit.P - FragmentWorldP;
	float ToLightLen = length(ToLight);
	ToLight = normalize(ToLight);

	float OneOverRad = 1.0f / Lit.Radius;
	float Attenuation = 1.0f / (1.0f + 2.0f * ToLightLen * OneOverRad + (ToLightLen * ToLightLen) * OneOverRad * OneOverRad);

	//NOTE(dima): Diffuse lighting
	float CosLightNorm = dot(ToLight, FragmentWorldN);
	CosLightNorm = clamp(CosLightNorm, 0.0f, 1.0f);
	Result += Attenuation * CosLightNorm * VertColor.xyz * Lit.Color;

	//NOTE(dima): Specular lighting



	return(Result);
}

void main(){

	Light1.P = vec3(0.0f, 4.0f, 0.0f);
	Light1.Color = vec3(1.0f, 1.0f, 1.0f);
	Light1.Radius = 50.0f;

	vec3 DirLightDir = vec3(0.2f, 0.2f, -1.0f);
	DirLightDir = normalize(DirLightDir);
	
	vec3 TotalColor = vec3(0.0f, 0.0f, 0.0f);

	TotalColor += CalculatePointLight(Light1);

	FragmentColor = vec4(TotalColor, 1.0f);
}
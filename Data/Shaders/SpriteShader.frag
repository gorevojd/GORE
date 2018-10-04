#version 330 core

in VS_OUT{
	vec3 FragPos;
	vec2 TexCoords;
}fs_in;

uniform bool Bitmap1IsSet;
uniform sampler2D Bitmap1;
uniform vec4 ModulationColor;
uniform vec3 CameraP;

out vec4 ResultColor;

void main(){
	vec4 Sampled1Color = ModulationColor;
	if(Bitmap1IsSet){
		Sampled1Color = texture(Bitmap1, fs_in.TexCoords) * ModulationColor;
	}

	ResultColor = Sampled1Color;
}
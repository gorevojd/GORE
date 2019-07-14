#version 330 core

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec2 inTexCoords;

uniform mat4 Projection;
uniform mat4 View;

uniform bool MirrorUVsHorizontally;

out VS_OUT{
	vec3 FragPos;
	vec2 TexCoords;
}vs_out;

void main(){
	vs_out.FragPos = inPosition;

	if(MirrorUVsHorizontally){
		vs_out.TexCoords = vec2(inTexCoords.x, inTexCoords.y);
	}
	else{
		vs_out.TexCoords = vec2(1.0f - inTexCoords.x, inTexCoords.y);
	}

	gl_Position = Projection * View * vec4(vs_out.FragPos, 1.0f);
	//gl_Position = vec4(vs_out.FragPos, 1.0f) * View * Projection;
}

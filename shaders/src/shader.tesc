#version 450 core
#extension GL_ARB_separate_shader_objects : enable

struct vert_t{
	vec2 UV;
	vec3 Normal;
};

layout(location = 0) in vert_t inVert[];

layout(vertices = 3) out;
layout(location = 0) out vert_t result[];


float lod(in vec4 pos){
	return clamp(pow(2.0, int(6.0 * (pos.z + 1.0))), 1.0, 64.0);
	// return 64.0;
	// return max(64.0 * (1.0 + pos.z +0.5), 1.0);
}

void main() {
	result[gl_InvocationID].UV = inVert[gl_InvocationID].UV;
	result[gl_InvocationID].Normal = inVert[gl_InvocationID].Normal;
	if(gl_InvocationID==0){
		gl_TessLevelInner[0]=lod((gl_in[0].gl_Position+gl_in[1].gl_Position+gl_in[2].gl_Position)/3.0);
		gl_TessLevelOuter[0]=lod((gl_in[1].gl_Position+gl_in[2].gl_Position)/2.0);
		gl_TessLevelOuter[1]=lod((gl_in[0].gl_Position+gl_in[2].gl_Position)/2.0);
		gl_TessLevelOuter[2]=lod((gl_in[0].gl_Position+gl_in[1].gl_Position)/2.0);
	}
	gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
}

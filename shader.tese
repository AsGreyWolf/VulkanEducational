#version 450 core
#extension GL_ARB_separate_shader_objects : enable

struct vert_t{
	vec2 UV;
	vec3 Normal;
};
layout(set = 0, binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;
layout(set = 1, binding = 1) uniform sampler2D bumpMap;

layout(triangles, equal_spacing, ccw) in;
layout(location = 0) in vert_t inVert[];

layout(location = 0) out vert_t result;

void main(){
	result.UV = gl_TessCoord.x*inVert[0].UV+gl_TessCoord.y*inVert[1].UV+gl_TessCoord.z*inVert[2].UV;
	result.Normal = gl_TessCoord.x*inVert[0].Normal+gl_TessCoord.y*inVert[1].Normal+gl_TessCoord.z*inVert[2].Normal;

	gl_Position = gl_TessCoord.x*gl_in[0].gl_Position+gl_TessCoord.y*gl_in[1].gl_Position+gl_TessCoord.z*gl_in[2].gl_Position;
	gl_Position.xyz += normalize(result.Normal) * 0.05 * texture(bumpMap, result.UV).z;//, (-gl_Position.z - 0.5) * 9.0
	gl_Position = ubo.proj*gl_Position;
}

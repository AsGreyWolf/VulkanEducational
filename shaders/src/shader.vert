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

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec2 inUV;
layout(location = 2) in vec3 inNormal;

layout(location = 0) out vert_t result;

void main() {
	mat4 matrix = ubo.view*ubo.model;
	gl_Position = matrix*vec4(inPos, 1.0);
	result.UV = inUV;
	result.Normal = (inverse(transpose(matrix)) * vec4(inNormal, 0.0)).xyz;
}

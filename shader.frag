#version 450 core
#extension GL_ARB_separate_shader_objects : enable

struct vert_t{
	vec2 UV;
	vec3 Normal;
};

layout(set = 1, binding = 0) uniform sampler2D tex;

layout(location = 0) in vert_t inVert;

layout(location = 0) out vec4 outColor;

void main() {
	outColor = texture(tex, inVert.UV);
}

#version 450

layout(location = 0) in vec3 inPos;

layout(set = 0, binding = 0) uniform UBO {
	mat4 view;
	mat4 proj;
} ubo;

layout(push_constant) uniform Push {
	mat4 model;
} push;

void main()
{
	gl_Position = ubo.proj * ubo.view * push.model * vec4(inPos, 1.0);
}

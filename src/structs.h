#pragma once
#include <glm/glm.hpp>
#include <vulkan/vulkan.h>
#include <array>

struct Vertex
{
	glm::vec3 xyz;
	glm::vec3 rgb;
	glm::vec2 uv;

	static VkVertexInputBindingDescription getBindingDescription()
	{
		VkVertexInputBindingDescription binding{};
		binding.binding = 0;
		binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		binding.stride = sizeof(Vertex);

		return binding;
	}

	static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions()
	{
		std::array<VkVertexInputAttributeDescription, 3> attributes{};
		attributes[0].binding = 0;
		attributes[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributes[0].location = 0;
		attributes[0].offset = offsetof(Vertex, xyz);

		attributes[1].binding = 0;
		attributes[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributes[1].location = 1;
		attributes[1].offset = offsetof(Vertex, rgb);

		attributes[2].binding = 0;
		attributes[2].format = VK_FORMAT_R32G32_SFLOAT;
		attributes[2].location = 2;
		attributes[2].offset = offsetof(Vertex, uv);
		return attributes;
	}
};

struct MVP
{
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 proj;
};
#pragma once
#define NOMINMAX
#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include <optional>
#include <vector>
#include <iostream>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <array>
#include "World.h"
#include "structs.h"
#include "Camera.h"

constexpr uint8_t MAX_FRAMES_IN_FLIGHT = 3;
const std::string texturePath = "src/txt/atlas.png";
struct QueueFamilyIndices
{
	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> presentFamily;

	bool isComplete()
	{
		return graphicsFamily.has_value() && presentFamily.has_value();
	}
};

struct SwapchainSupportDetails
{
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
	VkSurfaceCapabilitiesKHR capabilities;
};





class GraphicsEngine
{
public:
	static GraphicsEngine& getInstance()
	{
		static GraphicsEngine instance;
		return instance;
	}
	void operator=(GraphicsEngine&) = delete;
	void IntializeGraphicsEngine();
	void setFramebufferResized(bool resized);

	
	static void createIndexBuffer(const std::vector<uint16_t>& indices, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
	static void createVertexBuffer(const std::vector<Vertex>& vertices, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
	static VkDevice getDevice();
private:
	GraphicsEngine() = default;
	
	void initChunk();
	void createColorResources();
	VkSampleCountFlagBits getMaxSampleCount();
	bool hasStencilComponent(VkFormat format);
	VkFormat findDepthFormat();
	VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
	void createDepthResources();
	void createTextureSampler();
	VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);
	void createTextureImageView();
	void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
	void transitionImageLayout(VkImage, VkFormat format, VkImageLayout srcLayout, VkImageLayout dstLayout);
	static void endSingleTimeCommands(VkCommandBuffer buffer);
	static VkCommandBuffer beginSingleTimeCommands();
	void createImage(uint32_t height, uint32_t width, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkSampleCountFlagBits samples, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& memory);
	void createTextureImage();
	void createDescriptorSets();
	void createDescriptorPool();
	void updateUniformBuffer(uint32_t currentImage);
	void createUniformBuffers();
	void createDescriptorSetLayout();
	static void copyBuffer(VkBuffer src, VkBuffer dst, VkDeviceSize size);
	static void createBuffer(VkDeviceSize size, VkBufferUsageFlags flags, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& memory);
	static uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags flags);
	void cleanupSwapchain();
	void recreateSwapchain();
	void createSyncObjects();
	void drawFrame();
	void recordCommandBuffer(VkCommandBuffer buffer, uint32_t imageIndex);
	void createCommandBuffer();
	void createCommandPool();
	void createFramebuffers();
	void createRenderPass();
	VkShaderModule createShaderModule(const std::vector<char>& code);
	static std::vector<char> readFile(const std::string& filename);
	void createGraphicsPipeline();
	void createImageViews();
	SwapchainSupportDetails querySwapchainSupport(VkPhysicalDevice device);
	QueueFamilyIndices findQueueIndices(VkPhysicalDevice device);
	VkExtent2D chooseSwapExtent(const SwapchainSupportDetails& supportDetails);
	VkPresentModeKHR choosePresentMode(const SwapchainSupportDetails& supportDetails);
	VkSurfaceFormatKHR chooseSurfaceFormat(const SwapchainSupportDetails& supportDetails);
	void createSwapchain();
	void createWindowSurface();
	bool checkDeviceExtensionSupport(VkPhysicalDevice device);
	void createLogicalDevice();
	void pickPhysicalDevice();
	void createDebugMessenger();
	void createVulkanInstance();
	void createWindow();
	void mainLoop();
	void terminate();
private:
	GLFWwindow* m_Window;
	GLFWmonitor* m_Monitor;
	
	VkInstance m_Instance;
	VkDebugUtilsMessengerEXT m_DebugMessenger;
	static VkPhysicalDevice m_PhysicalDevice;
	static VkDevice m_Device;
	VkSurfaceKHR m_Surface;
	
	static VkQueue m_GraphicsQueue;
	VkQueue m_PresentQueue;

	VkSwapchainKHR m_Swapchain;
	std::vector<VkImage> m_SwapchainImages;
	std::vector<VkImageView> m_SwapchainImageViews;

	VkFormat m_SwapchainFormat;
	VkExtent2D m_SwapchainExtent;

	VkDescriptorSetLayout m_DescLayout;
	VkPipelineLayout m_PipelineLayout;
	VkRenderPass m_RenderPass;
	VkPipeline m_Pipeline;
	int m_Width, m_Height;

	static VkCommandPool m_CPool;
	std::vector<VkCommandBuffer> m_CommandBuffers;

	std::vector<VkFramebuffer> m_Framebuffers;
	std::vector<VkSemaphore> imageReadySemaphores, renderFinishedSemaphores;
	std::vector<VkFence> inFlightFences;

	uint32_t currentFrame = 0;
	bool framebufferResized = false;

	std::vector<VkBuffer> m_UniformBuffers;
	std::vector<VkDeviceMemory> m_UniformBuffersMemory;
	std::vector<void*> m_UniformBuffersMapped;

	VkDescriptorPool m_DPool;
	std::vector<VkDescriptorSet> m_DescriptorSets;
	float m_aspectRatio = 1.0f;

	VkImage textureImage;
	VkDeviceMemory textureImageMemory;
	VkImageView textureImageView;
	VkSampler textureSampler;

	VkImage depthImage;
	VkDeviceMemory depthImageMemory;
	VkImageView depthImageView;

	VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT;
	VkImage colorImage;
	VkDeviceMemory colorImageMemory;
	VkImageView colorImageView;

	Chunk mChunk = Chunk({ 0,0 });

	Camera mCamera;
};

static void framebufferResizeCallback(GLFWwindow* window, int width, int height)
{
	auto gEngine = reinterpret_cast<GraphicsEngine*>(glfwGetWindowUserPointer(window));
	gEngine->setFramebufferResized(true);


}
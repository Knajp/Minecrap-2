#include "GraphicsEngine.h"
#include <assert.h>
#include <set>
#include <array>
#include <algorithm> 
#include <fstream>
#include <glm/gtc/matrix_transform.hpp>
#include <chrono>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

std::vector<const char*> g_EnabledLayers = {
	"VK_LAYER_KHRONOS_validation"
};
std::vector<const char*> g_EnabledExtensions = {
	VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
	VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
	VK_KHR_SURFACE_EXTENSION_NAME
};
std::vector<const char*> g_EnabledDeviceExtensions = {  
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};


VkPhysicalDevice GraphicsEngine::m_PhysicalDevice = VK_NULL_HANDLE;
VkDevice GraphicsEngine::m_Device = VK_NULL_HANDLE;
VkQueue GraphicsEngine::m_GraphicsQueue = VK_NULL_HANDLE;
VkCommandPool GraphicsEngine::m_CPool = VK_NULL_HANDLE;


static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* userData)
{
	std::cerr << "Validation layer: " << pCallbackData->pMessage << std::endl;

	return VK_FALSE;
}
void GraphicsEngine::IntializeGraphicsEngine()
{
	createWindow();
	createVulkanInstance();
	createDebugMessenger();
	createWindowSurface();
	pickPhysicalDevice();
	createLogicalDevice();
	createSwapchain();
	createImageViews();
	createRenderPass();
	createDescriptorSetLayout();
	createGraphicsPipeline();
	createColorResources();
	createDepthResources();
	createFramebuffers();
	createCommandPool();
	createTextureImage();
	createTextureImageView();
	createTextureSampler();
	createUniformBuffers();
	createDescriptorPool();
	createDescriptorSets();
	initChunk();
	createCommandBuffer();
	createSyncObjects();
	mainLoop();
	terminate();
}

VkSurfaceFormatKHR GraphicsEngine::chooseSurfaceFormat(const SwapchainSupportDetails& supportDetails)
{
	for (const auto& format : supportDetails.formats)
	{
		if (format.format == VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace == VK_COLORSPACE_SRGB_NONLINEAR_KHR)
			return format;
	}

	return VkSurfaceFormatKHR();
}

QueueFamilyIndices GraphicsEngine::findQueueIndices(VkPhysicalDevice device)
{
	QueueFamilyIndices indices{};


	uint32_t queueCount;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueCount, nullptr);
	if (queueCount == 0) return indices;
	std::vector<VkQueueFamilyProperties> properties(queueCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueCount, properties.data());

	int i = 0;
	for (const auto& prop : properties)
	{
		if (prop.queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			indices.graphicsFamily = i;
		}
		VkBool32 presentSupport;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_Surface, &presentSupport);
		if (presentSupport) indices.presentFamily = i;
		if (indices.isComplete()) break;
		i++;
	}

	return indices;

}

void GraphicsEngine::createSwapchain()
{
	SwapchainSupportDetails supportDetails = querySwapchainSupport(m_PhysicalDevice);

	VkSwapchainCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = m_Surface;
	createInfo.presentMode = choosePresentMode(supportDetails);
	createInfo.imageExtent = chooseSwapExtent(supportDetails);
	createInfo.minImageCount = supportDetails.capabilities.minImageCount + 1;
	if (supportDetails.capabilities.maxImageCount > 0 && createInfo.minImageCount > supportDetails.capabilities.maxImageCount)
		createInfo.minImageCount = supportDetails.capabilities.maxImageCount;
	createInfo.imageFormat = chooseSurfaceFormat(supportDetails).format;
	createInfo.imageColorSpace = chooseSurfaceFormat(supportDetails).colorSpace;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	
	QueueFamilyIndices indices = findQueueIndices(m_PhysicalDevice);
	uint32_t queueIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

	if (indices.graphicsFamily == indices.presentFamily)
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	}
	else
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueIndices;
	}

	createInfo.preTransform = supportDetails.capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.clipped = VK_TRUE;
	createInfo.oldSwapchain = VK_NULL_HANDLE;

	if (vkCreateSwapchainKHR(m_Device, &createInfo, nullptr, &m_Swapchain) != VK_SUCCESS)
		throw std::runtime_error("Failed to create swapchain!");

	uint32_t imageCount;
	vkGetSwapchainImagesKHR(m_Device, m_Swapchain, &imageCount, nullptr);
	m_SwapchainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(m_Device, m_Swapchain, &imageCount, m_SwapchainImages.data());
	
	m_SwapchainFormat = chooseSurfaceFormat(supportDetails).format;
	m_SwapchainExtent = chooseSwapExtent(supportDetails);
}

void GraphicsEngine::createWindowSurface()
{
	VkWin32SurfaceCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	createInfo.hwnd = glfwGetWin32Window(m_Window);
	createInfo.hinstance = GetModuleHandle(nullptr);

	if (vkCreateWin32SurfaceKHR(m_Instance, &createInfo, nullptr, &m_Surface) != VK_SUCCESS)
		throw std::runtime_error("Failed to create window surface!");
}

bool GraphicsEngine::checkDeviceExtensionSupport(VkPhysicalDevice device)
{
	uint32_t supportedCount;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &supportedCount, nullptr);
	std::vector<VkExtensionProperties> availableProperties(supportedCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &supportedCount, availableProperties.data());

	std::set<std::string> required(g_EnabledDeviceExtensions.begin(), g_EnabledDeviceExtensions.end());

	for (const auto& ext : availableProperties)
	{
		required.erase(ext.extensionName);
	}
	return required.empty();
}

void GraphicsEngine::createLogicalDevice()
{
	QueueFamilyIndices queueIndices = findQueueIndices(m_PhysicalDevice);

	std::vector<VkDeviceQueueCreateInfo> queueInfos;
	std::set<uint32_t> uniqueQueueIndices =
	{
		queueIndices.graphicsFamily.value(),
		queueIndices.presentFamily.value()
	};

	float queuePriorities = 1.0f;
	for (const auto& ind : uniqueQueueIndices)
	{
		VkDeviceQueueCreateInfo queueCreateInfo{};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.queueFamilyIndex = ind;
		queueCreateInfo.pQueuePriorities = &queuePriorities;
		queueInfos.push_back(queueCreateInfo);
	}

	VkPhysicalDeviceFeatures enabledFeatures{};
	enabledFeatures.geometryShader = true;
	enabledFeatures.samplerAnisotropy = true;

	VkDeviceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.enabledExtensionCount = static_cast<uint32_t>(g_EnabledDeviceExtensions.size());
	createInfo.ppEnabledExtensionNames = g_EnabledDeviceExtensions.data();
	createInfo.enabledLayerCount = static_cast<uint32_t>(g_EnabledLayers.size());
	createInfo.ppEnabledLayerNames = g_EnabledLayers.data();
	createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueInfos.size());
	createInfo.pQueueCreateInfos = queueInfos.data();
	createInfo.pEnabledFeatures = &enabledFeatures;
	
	if (vkCreateDevice(m_PhysicalDevice, &createInfo, nullptr, &m_Device) != VK_SUCCESS)
		throw std::runtime_error("Failed to create logical device!");

	vkGetDeviceQueue(m_Device, queueIndices.graphicsFamily.value(), 0, &m_GraphicsQueue);
	vkGetDeviceQueue(m_Device, queueIndices.presentFamily.value(), 0, &m_PresentQueue);
}

void GraphicsEngine::pickPhysicalDevice()
{
	// enumerate physical devices
	uint32_t deviceCount;
	vkEnumeratePhysicalDevices(m_Instance, &deviceCount, nullptr);
	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(m_Instance, &deviceCount, devices.data());

	for (const VkPhysicalDevice& device : devices)
	{
		VkPhysicalDeviceProperties properties{};
		vkGetPhysicalDeviceProperties(device, &properties);

		VkPhysicalDeviceFeatures supportedFeatures{};
		vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

		bool extensionsSupported = checkDeviceExtensionSupport(device);

		bool swapchainAdequate = false;
		if (extensionsSupported)
		{
			SwapchainSupportDetails supportDetails = querySwapchainSupport(device);
			if (!supportDetails.formats.empty() && !supportDetails.presentModes.empty())
				swapchainAdequate = true;
		}
		else continue;

		if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && supportedFeatures.geometryShader && checkDeviceExtensionSupport(device) && supportedFeatures.samplerAnisotropy)
		{
			m_PhysicalDevice = device;
			msaaSamples = getMaxSampleCount();
			return;
		}
		
	}
	throw std::runtime_error("Failed to find a suitable GPU!");
}

void GraphicsEngine::createDebugMessenger()
{
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(m_Instance, "vkCreateDebugUtilsMessengerEXT");
	
	VkDebugUtilsMessengerCreateInfoEXT createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createInfo.pUserData = nullptr;
	createInfo.pfnUserCallback = debugCallback;

	if (func(m_Instance, &createInfo, nullptr, &m_DebugMessenger) != VK_SUCCESS)
		throw std::runtime_error("Failed to create debug utils messenger!");
}

void GraphicsEngine::createVulkanInstance()
{
	VkApplicationInfo appInfo{};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Minecrap 2";
	appInfo.applicationVersion = 1;
	appInfo.pEngineName = nullptr;
	appInfo.engineVersion = 0;
	appInfo.apiVersion = VK_API_VERSION_1_0;

	VkInstanceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;
	createInfo.enabledLayerCount = static_cast<uint32_t>(g_EnabledLayers.size());
	createInfo.ppEnabledLayerNames = g_EnabledLayers.data();
	createInfo.enabledExtensionCount = static_cast<uint32_t>(g_EnabledExtensions.size());
	createInfo.ppEnabledExtensionNames = g_EnabledExtensions.data();

	if (vkCreateInstance(&createInfo, nullptr, &m_Instance) != VK_SUCCESS)
		throw std::runtime_error("Failed to create instance!");
}

void GraphicsEngine::createWindow()
{
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	
	m_Monitor = glfwGetPrimaryMonitor();
	const GLFWvidmode* videoMode = glfwGetVideoMode(m_Monitor);

	assert(videoMode != nullptr);

	m_Width = videoMode->width;
	m_Height = videoMode->height;

	m_aspectRatio = (float)m_Width / (float)m_Height;

	mCamera.modifyAspectRatio(m_aspectRatio);

	m_Window = glfwCreateWindow(m_Width, m_Height, "Minecrap 2", nullptr, nullptr);

	glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetWindowUserPointer(m_Window, this);
	glfwSetFramebufferSizeCallback(m_Window, framebufferResizeCallback);
}

void GraphicsEngine::mainLoop()
{
	while (!glfwWindowShouldClose(m_Window))
	{

		glfwPollEvents();
		drawFrame();
	}
	vkDeviceWaitIdle(m_Device);
}

void GraphicsEngine::terminate()
{
	mChunk.destroyChunk();
	vkFreeMemory(m_Device, colorImageMemory, nullptr);
	vkDestroyImageView(m_Device, colorImageView, nullptr);
	vkDestroyImage(m_Device, colorImage, nullptr);
	vkFreeMemory(m_Device, depthImageMemory, nullptr);
	vkDestroyImageView(m_Device, depthImageView, nullptr);
	vkDestroyImage(m_Device, depthImage, nullptr);
	vkDestroySampler(m_Device, textureSampler, nullptr);
	vkFreeMemory(m_Device, textureImageMemory, nullptr);
	vkDestroyImageView(m_Device, textureImageView, nullptr);
	vkDestroyImage(m_Device, textureImage, nullptr);
	vkFreeDescriptorSets(m_Device, m_DPool, static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT), m_DescriptorSets.data());
	vkDestroyDescriptorPool(m_Device, m_DPool, nullptr);
	vkDestroyDescriptorSetLayout(m_Device, m_DescLayout, nullptr);
	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		vkUnmapMemory(m_Device, m_UniformBuffersMemory[i]);
		vkFreeMemory(m_Device, m_UniformBuffersMemory[i], nullptr);
		vkDestroyBuffer(m_Device, m_UniformBuffers[i], nullptr);
		vkDestroySemaphore(m_Device, imageReadySemaphores[i], nullptr);
		vkDestroySemaphore(m_Device, renderFinishedSemaphores[i], nullptr);
		vkDestroyFence(m_Device, inFlightFences[i], nullptr);
	}
	vkFreeCommandBuffers(m_Device, m_CPool, 1, m_CommandBuffers.data());
	vkDestroyCommandPool(m_Device, m_CPool, nullptr);
	for (auto framebuffer : m_Framebuffers)
		vkDestroyFramebuffer(m_Device, framebuffer, nullptr);
	vkDestroyPipeline(m_Device, m_Pipeline, nullptr);
	vkDestroyPipelineLayout(m_Device, m_PipelineLayout, nullptr);
	vkDestroyRenderPass(m_Device, m_RenderPass, nullptr);
	for (auto imageView : m_SwapchainImageViews)
		vkDestroyImageView(m_Device, imageView, nullptr);
	vkDestroySwapchainKHR(m_Device, m_Swapchain, nullptr);
	vkDestroySurfaceKHR(m_Instance, m_Surface, nullptr);
	vkDestroyDevice(m_Device, nullptr);
	auto destroyDebugMessenger = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(m_Instance, "vkDestroyDebugUtilsMessengerEXT");
	destroyDebugMessenger(m_Instance, m_DebugMessenger, nullptr);
	vkDestroyInstance(m_Instance, nullptr);
	glfwDestroyWindow(m_Window);
	glfwTerminate();
}

SwapchainSupportDetails GraphicsEngine::querySwapchainSupport(VkPhysicalDevice device)
{
	SwapchainSupportDetails supportDetails;

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_Surface, &supportDetails.capabilities);
	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_Surface, &formatCount, nullptr);

	if (formatCount != 0)
	{
		supportDetails.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_Surface, &formatCount, supportDetails.formats.data());
	}

	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_Surface, &presentModeCount, nullptr);

	if (presentModeCount != 0)
	{
		supportDetails.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_Surface, &presentModeCount, supportDetails.presentModes.data());
	}

	return supportDetails;
}

void GraphicsEngine::createImageViews()
{
	m_SwapchainImageViews.resize(m_SwapchainImages.size());

	for (size_t i = 0; i < m_SwapchainImageViews.size(); i++)
	{
		VkImageViewCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = m_SwapchainImages[i];
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = m_SwapchainFormat;
		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.layerCount = 1;
		createInfo.subresourceRange.levelCount = 1;

		if (vkCreateImageView(m_Device, &createInfo, nullptr, &m_SwapchainImageViews[i]) != VK_SUCCESS)
			throw std::runtime_error("Failed to create image view!");
	}
}

std::vector<char> GraphicsEngine::readFile(const std::string& filename)
{
	std::ifstream file(filename, std::ios::binary | std::ios::ate);

	if (!file.is_open())
		throw std::runtime_error("Failed to open file!");

	size_t fileSize = file.tellg();
	std::vector<char> buffer(fileSize);

	file.seekg(0);
	file.read(buffer.data(), fileSize);

	file.close();
	return buffer;
}

void GraphicsEngine::createGraphicsPipeline()
{
	auto vertexCode = readFile("src/bin/vert.spv");
	auto fragCode = readFile("src/bin/frag.spv");

	VkShaderModule vertexModule = createShaderModule(vertexCode);
	VkShaderModule fragModule = createShaderModule(fragCode);

	VkPipelineShaderStageCreateInfo vertexStageCreateInfo{};
	vertexStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertexStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertexStageCreateInfo.module = vertexModule;
	vertexStageCreateInfo.pName = "main";

	VkPipelineShaderStageCreateInfo fragStageCreateInfo{};
	fragStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragStageCreateInfo.pName = "main";
	fragStageCreateInfo.module = fragModule;

	VkPipelineShaderStageCreateInfo shaderStages[] = { vertexStageCreateInfo, fragStageCreateInfo };

	std::vector<VkDynamicState> dynamicStates = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

	VkPipelineDynamicStateCreateInfo dynamicCreateInfo{};
	dynamicCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicCreateInfo.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
	dynamicCreateInfo.pDynamicStates = dynamicStates.data();

	auto bindingDesc = Vertex::getBindingDescription();
	auto attDesc = Vertex::getAttributeDescriptions();

	VkPipelineVertexInputStateCreateInfo vertexInput{};
	vertexInput.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInput.vertexAttributeDescriptionCount = static_cast<uint32_t>(attDesc.size());
	vertexInput.vertexBindingDescriptionCount = 1;
	vertexInput.pVertexBindingDescriptions = &bindingDesc;
	vertexInput.pVertexAttributeDescriptions = attDesc.data();

	VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.primitiveRestartEnable = VK_FALSE;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

	VkPipelineDepthStencilStateCreateInfo depthInfo{};
	depthInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthInfo.depthTestEnable = VK_TRUE;
	depthInfo.depthWriteEnable = VK_TRUE;
	depthInfo.depthCompareOp = VK_COMPARE_OP_LESS;
	depthInfo.depthBoundsTestEnable = VK_FALSE;
	depthInfo.stencilTestEnable = VK_FALSE;

	VkViewport viewport{};
	viewport.height = (float)m_SwapchainExtent.height;
	viewport.width = (float)m_SwapchainExtent.width;
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor{};
	scissor.extent = m_SwapchainExtent;
	scissor.offset = { 0, 0 };

	VkPipelineViewportStateCreateInfo viewportState{};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;

	VkPipelineRasterizationStateCreateInfo rasterizer{};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.cullMode = VK_CULL_MODE_NONE;
	rasterizer.depthBiasEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
	rasterizer.lineWidth = 1.0f;

	VkPipelineMultisampleStateCreateInfo multisample{};
	multisample.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisample.rasterizationSamples = msaaSamples;
	multisample.sampleShadingEnable = VK_FALSE;

	VkPipelineColorBlendAttachmentState colorBlend{};
	colorBlend.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlend.blendEnable = VK_TRUE;
	colorBlend.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	colorBlend.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	colorBlend.colorBlendOp = VK_BLEND_OP_ADD;
	colorBlend.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlend.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlend.alphaBlendOp = VK_BLEND_OP_ADD;

	VkPipelineColorBlendStateCreateInfo colorState{};
	colorState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorState.attachmentCount = 1;
	colorState.pAttachments = &colorBlend;
	colorState.logicOpEnable = VK_FALSE;

	VkPipelineLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	layoutInfo.setLayoutCount = 1;
	layoutInfo.pSetLayouts = &m_DescLayout;

	if (vkCreatePipelineLayout(m_Device, &layoutInfo, nullptr, &m_PipelineLayout) != VK_SUCCESS)
		throw std::runtime_error("Failed to create pipeline layout");

	VkGraphicsPipelineCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	createInfo.stageCount = 2;
	createInfo.pStages = shaderStages;
	createInfo.pVertexInputState = &vertexInput;
	createInfo.pInputAssemblyState = &inputAssembly;
	createInfo.pRasterizationState = &rasterizer;
	createInfo.pMultisampleState = &multisample;
	createInfo.pDynamicState = &dynamicCreateInfo;
	createInfo.pViewportState = &viewportState;
	createInfo.pColorBlendState = &colorState;
	createInfo.pDepthStencilState = &depthInfo;
	createInfo.layout = m_PipelineLayout;
	createInfo.renderPass = m_RenderPass;
	createInfo.subpass = 0;

	if (vkCreateGraphicsPipelines(m_Device, VK_NULL_HANDLE, 1, &createInfo, nullptr, &m_Pipeline) != VK_SUCCESS)
		throw std::runtime_error("Failed to create graphics pipeline!");


	vkDestroyShaderModule(m_Device, vertexModule, nullptr);
	vkDestroyShaderModule(m_Device, fragModule, nullptr);
}

void GraphicsEngine::setFramebufferResized(bool resized)
{
	framebufferResized = resized;
}

void GraphicsEngine::initChunk()
{
	mChunk.generateMesh();
}

void GraphicsEngine::createColorResources()
{
	VkFormat colorFormat = m_SwapchainFormat;

	createImage(m_SwapchainExtent.height, m_SwapchainExtent.width, colorFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, msaaSamples, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, colorImage, colorImageMemory);
	colorImageView = createImageView(colorImage, colorFormat, VK_IMAGE_ASPECT_COLOR_BIT);
}

VkSampleCountFlagBits GraphicsEngine::getMaxSampleCount()
{
	VkPhysicalDeviceProperties props{};
	vkGetPhysicalDeviceProperties(m_PhysicalDevice, &props);
	
	
	VkSampleCountFlags counts = props.limits.framebufferColorSampleCounts & props.limits.framebufferDepthSampleCounts;
	if (counts & VK_SAMPLE_COUNT_64_BIT) return VK_SAMPLE_COUNT_64_BIT;
	if (counts & VK_SAMPLE_COUNT_32_BIT) return VK_SAMPLE_COUNT_32_BIT;
	if (counts & VK_SAMPLE_COUNT_16_BIT) return VK_SAMPLE_COUNT_16_BIT;
	if (counts & VK_SAMPLE_COUNT_8_BIT) return VK_SAMPLE_COUNT_8_BIT;
	if (counts & VK_SAMPLE_COUNT_4_BIT) return VK_SAMPLE_COUNT_4_BIT;
	if (counts & VK_SAMPLE_COUNT_2_BIT) return VK_SAMPLE_COUNT_2_BIT;
	
	return VK_SAMPLE_COUNT_1_BIT;


}

bool GraphicsEngine::hasStencilComponent(VkFormat format)
{
	return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}

VkFormat GraphicsEngine::findDepthFormat()
{
	return findSupportedFormat({ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT }, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

VkFormat GraphicsEngine::findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
{
	for (VkFormat format : candidates)
	{
		VkFormatProperties props;
		vkGetPhysicalDeviceFormatProperties(m_PhysicalDevice, format, &props);
		if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
			return format;
		else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
			return format;
	}
	throw std::runtime_error("Failed to find supported format!");
}

void GraphicsEngine::createDepthResources()
{
	VkFormat depthFormat = findDepthFormat();
	createImage(m_SwapchainExtent.height, m_SwapchainExtent.width, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, msaaSamples, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depthImage, depthImageMemory);
	depthImageView = createImageView(depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);

}

void GraphicsEngine::createTextureSampler()
{
	VkSamplerCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	createInfo.magFilter = VK_FILTER_NEAREST;
	createInfo.minFilter = VK_FILTER_NEAREST;
	createInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	createInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	createInfo.addressModeW= VK_SAMPLER_ADDRESS_MODE_REPEAT;
	createInfo.anisotropyEnable = VK_TRUE;
	VkPhysicalDeviceProperties props{};
	vkGetPhysicalDeviceProperties(m_PhysicalDevice, &props);

	createInfo.maxAnisotropy = props.limits.maxSamplerAnisotropy;
	createInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	createInfo.unnormalizedCoordinates = VK_FALSE;
	createInfo.compareEnable = VK_FALSE;
	createInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	createInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	createInfo.mipLodBias = 0.0f;
	createInfo.minLod = 0.0f;
	createInfo.maxLod = 0.0f;

	if (vkCreateSampler(m_Device, &createInfo, nullptr, &textureSampler) != VK_SUCCESS)
		throw std::runtime_error("Failed to create texture sampler!");
}

VkImageView GraphicsEngine::createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags)
{
	VkImageViewCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	createInfo.image = image;
	createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	createInfo.format = format;
	createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	createInfo.subresourceRange.baseArrayLayer = 0;
	createInfo.subresourceRange.layerCount = 1;
	createInfo.subresourceRange.baseMipLevel = 0;
	createInfo.subresourceRange.levelCount = 1;
	createInfo.subresourceRange.aspectMask = aspectFlags;
	VkImageView imageView;
	if (vkCreateImageView(m_Device, &createInfo, nullptr, &imageView) != VK_SUCCESS)
		throw std::runtime_error("Failed to create texture image view!");

	return imageView;
}

void GraphicsEngine::createTextureImageView()
{
	textureImageView = createImageView(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);
}

void GraphicsEngine::copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
{
	VkCommandBuffer commandBuffer = beginSingleTimeCommands();

	VkBufferImageCopy region{};
	region.bufferImageHeight = 0;
	region.bufferOffset = 0;
	region.bufferRowLength = 0;

	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;
	region.imageSubresource.mipLevel = 0;

	region.imageExtent = { width, height, 1 };
	region.imageOffset = { 0,0,0 };

	vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

	endSingleTimeCommands(commandBuffer);
}

void GraphicsEngine::transitionImageLayout(VkImage image, VkFormat format, VkImageLayout srcLayout, VkImageLayout dstLayout)
{
	VkCommandBuffer commandBuffer = beginSingleTimeCommands();

	VkPipelineStageFlags srcStage;
	VkPipelineStageFlags dstStage;

	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.image = image;
	barrier.newLayout = dstLayout;
	barrier.oldLayout = srcLayout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.levelCount = 1;
	
	if (srcLayout == VK_IMAGE_LAYOUT_UNDEFINED && dstLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
	{
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if (srcLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && dstLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	{
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	else throw std::invalid_argument("Unsupported image transition!");

	vkCmdPipelineBarrier(commandBuffer, srcStage, dstStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);

	endSingleTimeCommands(commandBuffer);
}

void GraphicsEngine::endSingleTimeCommands(VkCommandBuffer buffer)
{
	vkEndCommandBuffer(buffer);

	VkSubmitInfo submitInfo{};
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &buffer;
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	vkQueueSubmit(m_GraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(m_GraphicsQueue);

	vkFreeCommandBuffers(m_Device, m_CPool, 1, &buffer);
}

VkCommandBuffer GraphicsEngine::beginSingleTimeCommands()
{
	VkCommandBuffer commandBuffer{};

	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandBufferCount = 1;
	allocInfo.commandPool = m_CPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

	vkAllocateCommandBuffers(m_Device, &allocInfo, &commandBuffer);

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	return commandBuffer;
}

void GraphicsEngine::createImage(uint32_t height, uint32_t width, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkSampleCountFlagBits samples, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& memory)
{
	VkImageCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	createInfo.extent.height = height;
	createInfo.extent.width = width;
	createInfo.extent.depth = 1;
	createInfo.mipLevels = 1;
	createInfo.arrayLayers = 1;
	createInfo.format = format;
	createInfo.tiling = tiling;
	createInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	createInfo.usage = usage;
	createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	createInfo.samples = samples;
	createInfo.imageType = VK_IMAGE_TYPE_2D;

	if (vkCreateImage(m_Device, &createInfo, nullptr, &image) != VK_SUCCESS)
		throw std::runtime_error("Failed to create texture image!");

	VkMemoryRequirements memReqs{};
	vkGetImageMemoryRequirements(m_Device, image, &memReqs);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.allocationSize = memReqs.size;
	allocInfo.memoryTypeIndex = findMemoryType(memReqs.memoryTypeBits, properties);
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;

	if (vkAllocateMemory(m_Device, &allocInfo, nullptr, &memory) != VK_SUCCESS)
		throw std::runtime_error("Failed to allocate texture image memory!");

	vkBindImageMemory(m_Device, image, memory, 0);
}

void GraphicsEngine::createTextureImage()
{
	int texWidth, texHeight, numColCh;
	stbi_uc* pixels = stbi_load(texturePath.c_str(), &texWidth, &texHeight, &numColCh, STBI_rgb_alpha);

	VkDeviceSize bufferSize = texWidth * texHeight * 4;

	if (!pixels)
		throw std::runtime_error("Failed to load texture!");

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;

	createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, stagingBuffer, stagingBufferMemory);

	void* data;
	vkMapMemory(m_Device, stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, pixels, static_cast<size_t>(bufferSize));
	vkUnmapMemory(m_Device, stagingBufferMemory);

	stbi_image_free(pixels);

	createImage(static_cast<uint32_t>(texHeight), static_cast<uint32_t>(texWidth), VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, VK_SAMPLE_COUNT_1_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImage, textureImageMemory);

	transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	copyBufferToImage(stagingBuffer, textureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
	transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	vkFreeMemory(m_Device, stagingBufferMemory, nullptr);
	vkDestroyBuffer(m_Device, stagingBuffer, nullptr);

}

void GraphicsEngine::createDescriptorSets()
{
	std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, m_DescLayout);

	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.descriptorPool = m_DPool;
	allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
	allocInfo.pSetLayouts = layouts.data();
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;

	m_DescriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
	if (vkAllocateDescriptorSets(m_Device, &allocInfo, m_DescriptorSets.data()) != VK_SUCCESS)
		throw std::runtime_error("Failed to allocate desc sets!");

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		VkDescriptorBufferInfo bufferInfo{};
		bufferInfo.buffer = m_UniformBuffers[i];
		bufferInfo.offset = 0;
		bufferInfo.range = sizeof(MVP);

		VkDescriptorImageInfo imageInfo{};
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo.imageView = textureImageView;
		imageInfo.sampler = textureSampler;

		std::array<VkWriteDescriptorSet, 2> writeInfos{};

		writeInfos[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeInfos[0].dstSet = m_DescriptorSets[i];
		writeInfos[0].dstBinding = 0;
		writeInfos[0].dstArrayElement = 0;
		writeInfos[0].descriptorCount = 1;
		writeInfos[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		writeInfos[0].pBufferInfo = &bufferInfo;

		writeInfos[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeInfos[1].dstSet = m_DescriptorSets[i];
		writeInfos[1].dstBinding = 1;
		writeInfos[1].dstArrayElement = 0;
		writeInfos[1].descriptorCount = 1;
		writeInfos[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		writeInfos[1].pImageInfo = &imageInfo;

		vkUpdateDescriptorSets(m_Device, static_cast<uint32_t>(writeInfos.size()), writeInfos.data(), 0, nullptr);
	}
}

void GraphicsEngine::createDescriptorPool()
{
	std::array<VkDescriptorPoolSize, 2> poolSizes{};
	
	poolSizes[0].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
	poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSizes[1].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
	poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	
	VkDescriptorPoolCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	createInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	createInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
	createInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	createInfo.pPoolSizes = poolSizes.data();
	
	if (vkCreateDescriptorPool(m_Device, &createInfo, nullptr, &m_DPool) != VK_SUCCESS)
		throw std::runtime_error("Failed to create descriptor pool!");
}

void GraphicsEngine::updateUniformBuffer(uint32_t currentImage)
{	
	MVP ubo = mCamera.getMatrices();

	memcpy(m_UniformBuffersMapped[currentImage], &ubo, sizeof(MVP));
}

void GraphicsEngine::createUniformBuffers()
{
	VkDeviceSize bufferSize = sizeof(MVP);

	m_UniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
	m_UniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);
	m_UniformBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_UniformBuffers[i], m_UniformBuffersMemory[i]);
		vkMapMemory(m_Device, m_UniformBuffersMemory[i], 0, bufferSize, 0, &m_UniformBuffersMapped[i]);
	}
}

void GraphicsEngine::createDescriptorSetLayout()
{
	VkDescriptorSetLayoutBinding bindInfo{};
	bindInfo.binding = 0;
	bindInfo.descriptorCount = 1;
	bindInfo.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	bindInfo.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	VkDescriptorSetLayoutBinding samplerBind{};
	samplerBind.binding = 1;
	samplerBind.descriptorCount = 1;
	samplerBind.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	samplerBind.pImmutableSamplers = nullptr;
	samplerBind.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	std::array<VkDescriptorSetLayoutBinding, 2> bindings = { bindInfo, samplerBind };
	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
	layoutInfo.pBindings = bindings.data();
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	
	if (vkCreateDescriptorSetLayout(m_Device, &layoutInfo, nullptr, &m_DescLayout) != VK_SUCCESS)
		throw std::runtime_error("Failed to create descriptor set layout");

}

void GraphicsEngine::copyBuffer(VkBuffer src, VkBuffer dst, VkDeviceSize size)
{
	VkCommandBuffer commandBuffer = beginSingleTimeCommands();

	VkBufferCopy copyRegion{ };
	copyRegion.size = size;
	vkCmdCopyBuffer(commandBuffer, src, dst, 1, &copyRegion);
	
	endSingleTimeCommands(commandBuffer);
}

void GraphicsEngine::createBuffer(VkDeviceSize size, VkBufferUsageFlags flags, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& memory)
{
	VkBufferCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	createInfo.size = size;
	createInfo.usage = flags;

	if (vkCreateBuffer(m_Device, &createInfo, nullptr, &buffer) != VK_SUCCESS)
		throw std::runtime_error("Failed to create buffer!");

	VkMemoryRequirements memReqs{};
	vkGetBufferMemoryRequirements(m_Device, buffer, &memReqs);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memReqs.size;
	allocInfo.memoryTypeIndex = findMemoryType(memReqs.memoryTypeBits, properties);

	if (vkAllocateMemory(m_Device, &allocInfo, nullptr, &memory) != VK_SUCCESS)
		throw std::runtime_error("Failed to allocate buffer memory!");

	vkBindBufferMemory(m_Device, buffer, memory, 0);

}

uint32_t GraphicsEngine::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags flags)
{
	VkPhysicalDeviceMemoryProperties properties{};
	vkGetPhysicalDeviceMemoryProperties(m_PhysicalDevice, &properties);

	for (uint32_t i = 0; properties.memoryTypeCount; i++)
	{
		if ((typeFilter & (1 << i)) && (properties.memoryTypes[i].propertyFlags & flags) == flags )
			return i;
	}
	throw std::runtime_error("Failed to find a suitable memory type!");
}

void GraphicsEngine::createIndexBuffer(const std::vector<uint16_t>& indices, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
{
	VkDeviceSize bufferSize = sizeof(uint16_t) * indices.size();

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

	void* data;
	vkMapMemory(m_Device, stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, indices.data(), bufferSize);
	vkUnmapMemory(m_Device, stagingBufferMemory);

	createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, buffer, bufferMemory);
	copyBuffer(stagingBuffer, buffer, bufferSize);

	vkFreeMemory(m_Device, stagingBufferMemory, nullptr);
	vkDestroyBuffer(m_Device, stagingBuffer, nullptr);
}

void GraphicsEngine::createVertexBuffer(const std::vector<Vertex>& vertices, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
{
	VkDeviceSize bufferSize = sizeof(Vertex) * vertices.size();

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

	void* data;
	vkMapMemory(m_Device, stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, vertices.data(), bufferSize);
	vkUnmapMemory(m_Device, stagingBufferMemory);

	createBuffer(bufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, buffer, bufferMemory);
	copyBuffer(stagingBuffer, buffer, bufferSize);

	vkFreeMemory(m_Device, stagingBufferMemory, nullptr);
	vkDestroyBuffer(m_Device, stagingBuffer, nullptr);
}

VkDevice GraphicsEngine::getDevice()
{
	return m_Device;
}

void GraphicsEngine::cleanupSwapchain()
{
	for (auto framebuffer : m_Framebuffers)
	{
		vkDestroyFramebuffer(m_Device, framebuffer, nullptr);
	}
	for (auto imageView : m_SwapchainImageViews)
	{
		vkDestroyImageView(m_Device, imageView, nullptr);
	}
	vkDestroySwapchainKHR(m_Device, m_Swapchain, nullptr);
}

void GraphicsEngine::recreateSwapchain()
{
	int width = 0, height = 0;
	glfwGetFramebufferSize(m_Window, &width, &height);
	while (width == 0 || height == 0)
	{
		glfwGetFramebufferSize(m_Window, &width, &height);
		glfwWaitEvents();
	}

	vkDeviceWaitIdle(m_Device);

	m_Width = width;
	m_Height = height;

	m_aspectRatio = (float)m_Width / (float)m_Height;

	mCamera.modifyAspectRatio(m_aspectRatio);

	cleanupSwapchain();

	createSwapchain();
	createImageViews();
	createColorResources();
	createDepthResources();
	createFramebuffers();
}

void GraphicsEngine::createSyncObjects()
{
	VkSemaphoreCreateInfo semaphoreInfo{};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	VkFenceCreateInfo fenceInfo{};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
	imageReadySemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		if (vkCreateSemaphore(m_Device, &semaphoreInfo, nullptr, &imageReadySemaphores[i]) != VK_SUCCESS ||
			vkCreateSemaphore(m_Device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
			vkCreateFence(m_Device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS)
			throw std::runtime_error("Failed to create sync objects!");
	}
	
}

void GraphicsEngine::drawFrame()
{
	vkWaitForFences(m_Device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

	uint32_t imageIndex;
	VkResult result = vkAcquireNextImageKHR(m_Device, m_Swapchain, UINT64_MAX, imageReadySemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

	if (result == VK_ERROR_OUT_OF_DATE_KHR)
	{
		recreateSwapchain();
		return;
	}
	else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
		throw std::runtime_error("Failed to acquire next image index!");

	vkResetFences(m_Device, 1, &inFlightFences[currentFrame]);

	vkResetCommandBuffer(m_CommandBuffers[currentFrame], 0);

	mCamera.processInput(m_Window);
	updateUniformBuffer(currentFrame);

	recordCommandBuffer(m_CommandBuffers[currentFrame], imageIndex);

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &m_CommandBuffers[currentFrame];

	VkSemaphore waitSemaphores[] = { imageReadySemaphores[currentFrame] };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[currentFrame] };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	if (vkQueueSubmit(m_GraphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS)
		throw std::runtime_error("Failed to submit draw command buffer");

	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;
	VkSwapchainKHR swapchains[] = { m_Swapchain };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapchains;
	presentInfo.pImageIndices = &imageIndex;

	result = vkQueuePresentKHR(m_PresentQueue, &presentInfo);
	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized)
	{
		framebufferResized = false;
		recreateSwapchain();
		return;
	}
	else if (result != VK_SUCCESS)
		throw std::runtime_error("Failed to present!");

	currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void GraphicsEngine::recordCommandBuffer(VkCommandBuffer buffer, uint32_t imageIndex)
{
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	if (vkBeginCommandBuffer(buffer, &beginInfo) != VK_SUCCESS)
		throw std::runtime_error("Failed to begin command buffer!");

	std::array<VkClearValue, 2> clearValues{};
	clearValues[0].color = { {0.537f, 0.906f, 1.0f, 1.0f} };
	clearValues[1].depthStencil = { 1.0f, 0 };

	VkRenderPassBeginInfo renderInfo{};
	renderInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderInfo.framebuffer = m_Framebuffers[imageIndex];
	renderInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
	renderInfo.renderArea.extent = m_SwapchainExtent;
	renderInfo.renderArea.offset = { 0,0 };
	renderInfo.renderPass = m_RenderPass;
	renderInfo.pClearValues = clearValues.data();
	
	vkCmdBeginRenderPass(buffer, &renderInfo, VK_SUBPASS_CONTENTS_INLINE);

	vkCmdBindPipeline(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline);

	

	VkViewport viewport{};
	viewport.height = static_cast<float>(m_SwapchainExtent.height);
	viewport.width = static_cast<float>(m_SwapchainExtent.width);
	viewport.maxDepth = 1.0f;
	viewport.minDepth = 0.0f;
	viewport.x = 0.0f;
	viewport.y = 0.0f;

	VkRect2D scissor{};
	scissor.extent = m_SwapchainExtent;
	scissor.offset = { 0, 0 };

	vkCmdSetViewport(buffer, 0, 1, &viewport);
	vkCmdSetScissor(buffer, 0, 1, &scissor);

	vkCmdBindDescriptorSets(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_PipelineLayout, 0, 1, &m_DescriptorSets[currentFrame], 0, nullptr);

	mChunk.Render(buffer);
	
	vkCmdEndRenderPass(buffer);

	if (vkEndCommandBuffer(buffer) != VK_SUCCESS)
		throw std::runtime_error("Failed to record command buffer!");
}

void GraphicsEngine::createCommandBuffer()
{
	m_CommandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = m_CPool;
	allocInfo.commandBufferCount = static_cast<uint32_t>(m_CommandBuffers.size());
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

	if (vkAllocateCommandBuffers(m_Device, &allocInfo, m_CommandBuffers.data()) != VK_SUCCESS)
		throw std::runtime_error("Failed to allocate command buffer!");
}

void GraphicsEngine::createCommandPool()
{
	QueueFamilyIndices indices = findQueueIndices(m_PhysicalDevice);

	VkCommandPoolCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	createInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	createInfo.queueFamilyIndex = indices.graphicsFamily.value();

	if (vkCreateCommandPool(m_Device, &createInfo, nullptr, &m_CPool) != VK_SUCCESS)
		throw std::runtime_error("Failed to create command pool!");
}

void GraphicsEngine::createFramebuffers()
{
	m_Framebuffers.resize(m_SwapchainImageViews.size());

	for (size_t i = 0; i < m_SwapchainImageViews.size(); i++)
	{
		std::array<VkImageView, 3> attachments = { colorImageView, depthImageView, m_SwapchainImageViews[i] };

		VkFramebufferCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		createInfo.height = m_SwapchainExtent.height;
		createInfo.width = m_SwapchainExtent.width;
		createInfo.layers = 1;
		createInfo.renderPass = m_RenderPass;
		createInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		createInfo.pAttachments = attachments.data();

		if (vkCreateFramebuffer(m_Device, &createInfo, nullptr, &m_Framebuffers[i]) != VK_SUCCESS)
			throw std::runtime_error("Failed to create framebuffer!");
	}
	
}

void GraphicsEngine::createRenderPass()
{
	VkAttachmentDescription colorAtt{};
	colorAtt.format = m_SwapchainFormat;
	colorAtt.samples = msaaSamples;
	colorAtt.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAtt.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAtt.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAtt.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAtt.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAtt.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference colorRef{};
	colorRef.attachment = 0;
	colorRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentDescription depthAtt{};
	depthAtt.format = findDepthFormat();
	depthAtt.samples = msaaSamples;
	depthAtt.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAtt.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAtt.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthAtt.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAtt.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depthAtt.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depthRef{};
	depthRef.attachment = 1;
	depthRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentDescription colorResolve{};
	colorResolve.format = m_SwapchainFormat;
	colorResolve.samples = VK_SAMPLE_COUNT_1_BIT;
	colorResolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorResolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorResolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorResolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorResolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorResolve.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference resolveRef{};
	resolveRef.attachment = 2;
	resolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorRef;
	subpass.pDepthStencilAttachment = &depthRef;
	subpass.pResolveAttachments = &resolveRef;

	VkSubpassDependency dep{};
	dep.srcSubpass = VK_SUBPASS_EXTERNAL;
	dep.dstSubpass = 0;
	dep.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	dep.srcAccessMask = 0;
	dep.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	dep.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;


	std::array<VkAttachmentDescription, 3> attachments = { colorAtt, depthAtt, colorResolve };
	VkRenderPassCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	createInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	createInfo.pAttachments = attachments.data();
	createInfo.subpassCount = 1;
	createInfo.pSubpasses = &subpass;
	createInfo.dependencyCount = 1;
	createInfo.pDependencies = &dep;

	if (vkCreateRenderPass(m_Device, &createInfo, nullptr, &m_RenderPass) != VK_SUCCESS)
		throw std::runtime_error("Failed to create render pass!");
}

VkShaderModule GraphicsEngine::createShaderModule(const std::vector<char>& code)
{
	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

	VkShaderModule smodule;
	if (vkCreateShaderModule(m_Device, &createInfo, nullptr, &smodule) != VK_SUCCESS)
		throw std::runtime_error("Failed to create shader module");

	return smodule;
}

VkPresentModeKHR GraphicsEngine::choosePresentMode(const SwapchainSupportDetails& supportDetails)
{
	for (const auto& presentMode : supportDetails.presentModes)
	{
		if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR)
			return presentMode;
	}
	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D GraphicsEngine::chooseSwapExtent(const SwapchainSupportDetails& supportDetails)
{
	if (supportDetails.capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
		return supportDetails.capabilities.currentExtent;
	else
	{
		int width, height;
		glfwGetFramebufferSize(m_Window, &width, &height);

		VkExtent2D actualExtent = {
			static_cast<uint32_t>(width),
			static_cast<uint32_t>(height)
		};

		actualExtent.width = std::clamp(actualExtent.width, supportDetails.capabilities.minImageExtent.width, supportDetails.capabilities.maxImageExtent.width);
		actualExtent.height = std::clamp(actualExtent.height, supportDetails.capabilities.minImageExtent.height, supportDetails.capabilities.maxImageExtent.height);

		return actualExtent;
	}
}

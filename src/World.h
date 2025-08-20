#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include <glm/glm.hpp>
#include "structs.h"
#include <unordered_map>
#include <functional>
#include <iostream>
constexpr unsigned short int CHUNKSIZE = 16;
constexpr unsigned short int CHUNKHEIGHT = 64;

constexpr uint8_t MAX_FRAMES_IN_FLIGHT = 3;

enum BLOCKTYPE {
	AIR, GRASS, DIRT, STONE
};
enum BLOCKFACE {
	FRONT, BACK, RIGHT, LEFT, TOP, BOTTOM
};

namespace std {
	template <>
	struct hash<glm::ivec2> {
		std::size_t operator()(const glm::ivec2& v) const{
			std::size_t h1 = std::hash<int>{}(v.x);
			std::size_t h2 = std::hash<int>{}(v.y);
			return h1 ^ (h2 + 0x9e3779b9 + (h1 << 6) + (h1 >> 2));
		}
	};
}


static uint8_t getBlockTextureIndex(BLOCKTYPE bType, BLOCKFACE bFace)
{
	switch (bType)
	{
	case GRASS:
		switch (bFace)
		{
		case TOP:
			return 0;
		case BOTTOM:
			return 2;
		default:
			return 1;
		}
		break;
	case DIRT:
		return 2;
		break;
	case STONE:
		return 3;
	default:
		return 99;
	}
}
class ChunkData
{
public:
	ChunkData() = default;
	ChunkData(glm::ivec2 chunkCoords);
	~ChunkData();

	ChunkData(const ChunkData&) = delete;
	ChunkData& operator=(const ChunkData&) = delete;

	bool allocateChunkData(glm::ivec2 chunkCoords);
	uint8_t* getData();
	bool isFaceVisible(glm::ivec3 blockPos, BLOCKFACE face);
	int getBlockIndex(glm::ivec3 blockCoords);
private:
	uint8_t* pData;
};

class Chunk
{
public:
	Chunk(glm::ivec2 aWorldPos);
	Chunk() = default;

	~Chunk();

	Chunk(const Chunk&) = delete;
	Chunk& operator=(const Chunk&) = delete;
	
	

	void generateMesh();
	void Render(VkCommandBuffer commandBuffer) const;
	glm::ivec2 getPosition() const;
	void destroyChunk() const;

	bool isPendingDeletion() const;
	void setPendingDeletionStatus(bool val);
private:
	std::vector<Vertex> mMeshVertices;
	std::vector<uint16_t> mMeshIndices;
	glm::ivec2 mWorldPosition;
	ChunkData mData;

	VkBuffer mVertexBuffer;
	VkDeviceMemory mVertexBufferMemory;
	
	VkBuffer mIndexBuffer;
	VkDeviceMemory mIndexBufferMemory;

	bool pendingDeletion = false;
};

struct ChunkFreeList
{
	ChunkFreeList() = default;
	void Init(VkDevice& pDevice)
	{
		VkFenceCreateInfo fenceCreateInfo{};
		fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

		if (vkCreateFence(pDevice, &fenceCreateInfo, nullptr, &fence) != VK_SUCCESS)
			throw std::runtime_error("Failed to create ChunkFreeList fence!");
	}
	VkFence fence;
	std::vector<Chunk*> chunkList;
};

class Planet
{
public:
	Planet();
	Planet(Planet&) = delete;
	Planet& operator=(Planet&) = delete;

	void InitDestructionList(VkDevice& pDevice);
	void Update(glm::vec2 playerPosition, uint32_t currentFrame);
	void onPlayerCrossedChunk(glm::ivec2 plrChunk, uint32_t currentFrame);
	void Render(VkCommandBuffer commandBuffer, VkPipelineLayout& layout);
	void Cleanup(uint32_t currentFrame, VkDevice& device);
private:
	std::unordered_map<glm::ivec2, Chunk> mChunks; 
	std::array<ChunkFreeList, MAX_FRAMES_IN_FLIGHT> mAwaitngDestruction;
	const uint8_t renderDistance = 3;
};

/*
	TODO: add a pendingDestruction flag for chunks to cut them out of renders for other FramesInFlight. Never render a chunk with that flag signalled.
*/
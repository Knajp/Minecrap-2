#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include <glm/glm.hpp>
#include "structs.h"
#include <unordered_map>
#include <functional>
#include <iostream>
#include <unordered_set>
#include "Camera.h"

constexpr unsigned short int CHUNKSIZE = 16;
constexpr unsigned short int CHUNKHEIGHT = 64;

constexpr uint8_t MAX_FRAMES_IN_FLIGHT = 3;

enum BLOCKTYPE {
	AIR, GRASS, DIRT, STONE, WOOD, LEAVES, TALLGRASS, REDFLOWER, PURPLEFLOWER, DEBUGBLOCK
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
	
	template<>
	struct hash<glm::ivec3> {
		std::size_t operator()(const glm::ivec3& v) const {
			std::size_t h1 = std::hash<int>()(v.x);
			std::size_t h2 = std::hash<int>()(v.y);
			std::size_t h3 = std::hash<int>()(v.z);

			std::size_t seed = h1;
			seed ^= h2 + 0x9e3779b9 + (seed << 6) + (seed >> 2);
			seed ^= h3 + 0x9e3779b9 + (seed << 6) + (seed >> 2);

			return seed;
		}
	};
}



constexpr static uint8_t getBlockTextureIndex(BLOCKTYPE bType, BLOCKFACE bFace)
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
	case WOOD:
		switch (bFace)
		{
		case TOP:
			return 4;
		case BOTTOM:
			return 4;
		default:
			return 5;
		}
	case LEAVES:
		return 6;
	case TALLGRASS:
		return 7;
	case REDFLOWER:
		return 8;
	case PURPLEFLOWER:
		return 9;
	case DEBUGBLOCK:
		return 10;
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

	ChunkData(ChunkData&&) noexcept = default;
	ChunkData& operator=(ChunkData&&) noexcept = default;

	bool allocateChunkData(glm::ivec2 chunkCoords);
	uint8_t* getData();
	bool isFaceVisible(glm::ivec3 blockPos, BLOCKFACE face);
	bool isBlockTransparent(BLOCKTYPE type);
	int getBlockIndex(glm::ivec3 blockCoords);
	void generateTrees(glm::ivec2 chunkCoords);
	void placeTree(glm::ivec3 baseCoords);
	int getTopBlock(glm::ivec2 coords);
	bool canPlaceTree(const std::vector<glm::ivec2>& treeVector, glm::ivec2 coords);
	void generateGrass(glm::ivec2 chunkCoords);
	unsigned int airBlockCount;
	unsigned int transparentBlockCount;
	std::unique_ptr<std::unordered_map<glm::ivec3, BLOCKTYPE>> mMissingBlocks;

private:
	std::unique_ptr<uint8_t[]> pData;
};

class Chunk
{
public:
	Chunk(glm::ivec2 aWorldPos);
	Chunk() = default;

	~Chunk();

	Chunk(Chunk&& other) noexcept;
	Chunk& operator=(Chunk&&) noexcept = default;
	
	

	void generateMesh();
	void createGPUBuffers();
	void Render(VkCommandBuffer commandBuffer) const;
	glm::ivec2 getPosition() const;
	void destroyChunk() const;

	bool isPendingDeletion() const;
	void setPendingDeletionStatus(bool val);
	bool hasGPUbuffers() const;
	inline static std::unordered_set<BLOCKTYPE> billboards = { TALLGRASS, REDFLOWER, PURPLEFLOWER };
	inline static std::unordered_set<BLOCKTYPE> transparentBlocks = { TALLGRASS, REDFLOWER, PURPLEFLOWER, LEAVES };
	std::unordered_map<glm::ivec3, BLOCKTYPE>* getMissingBlocks();
	void updateBlock(glm::ivec3 blockCoords, BLOCKTYPE bType);

	bool getNeedRemeshStatus() const;
	void setNeedRemeshStatus(bool nr);

	const AABB* getBoundingBox() const;
private:
	std::vector<Vertex> mMeshVertices;
	std::vector<uint16_t> mMeshIndices;
	std::vector<Vertex> mTransparentMeshVertices;
	std::vector<uint16_t> mTransparentMeshIndices;

	glm::ivec2 mWorldPosition;
	ChunkData mData;

	VkBuffer mVertexBuffer;
	VkDeviceMemory mVertexBufferMemory;
	
	VkBuffer mIndexBuffer;
	VkDeviceMemory mIndexBufferMemory;

	VkBuffer mTransparentVertexBuffer;
	VkDeviceMemory mTransparentVertexBuffferMemory;

	VkBuffer mTransparentIndexBuffer;
	VkDeviceMemory mTransparentIndexBufferMemory;

	bool needRemesh = false;
	bool pendingDeletion = false;

	std::unique_ptr<AABB> boundingbox;
};

struct ChunkFreeList
{
	ChunkFreeList() = default;
	std::vector<Chunk*> chunkList;
};

class Planet
{
public:
	Planet();
	Planet(Planet&) = delete;
	Planet& operator=(Planet&) = delete;

	void prepareForDestruction(); // very funny name
	void Update(glm::vec2 playerPosition, uint32_t currentFrame);
	void onPlayerCrossedChunk(glm::ivec2 plrChunk, uint32_t currentFrame);
	void Render(VkCommandBuffer commandBuffer, VkPipelineLayout& layout, Camera& cam);
	void Cleanup(uint32_t currentFrame, VkDevice& device);
	void fillMissingBlocks();
private:
	std::unordered_map<glm::ivec2, Chunk> mChunks; 
	std::array<ChunkFreeList, MAX_FRAMES_IN_FLIGHT> mAwaitngDestruction;
	const uint8_t renderDistance = 3;
	
};

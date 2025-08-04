#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include <glm/glm.hpp>
#include "structs.h"

constexpr unsigned short int CHUNKSIZE = 16;
constexpr unsigned short int CHUNKHEIGHT = 64;

enum BLOCKTYPE {
	AIR, GRASS, DIRT, STONE
};
enum BLOCKFACE {
	FRONT, BACK, RIGHT, LEFT, TOP, BOTTOM
};


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
	ChunkData();
	~ChunkData();

	ChunkData(const ChunkData&) = delete;
	ChunkData& operator=(const ChunkData&) = delete;

	bool allocateChunkData();
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
	void Render(VkCommandBuffer commandBuffer);
	glm::ivec2 getPosition() const;
	void destroyChunk();
private:
	std::vector<Vertex> mMeshVertices;
	std::vector<uint16_t> mMeshIndices;
	glm::ivec2 mWorldPosition;
	ChunkData mData;

	VkBuffer mVertexBuffer;
	VkDeviceMemory mVertexBufferMemory;
	
	VkBuffer mIndexBuffer;
	VkDeviceMemory mIndexBufferMemory;
};
#include "World.h"
#include "GraphicsEngine.h"
#include <array>
#include <glm/glm.hpp>

Chunk::Chunk(glm::ivec2 aWorldPos)
    :mWorldPosition(aWorldPos)
{
    
}


Chunk::~Chunk()
{
    
}

void Chunk::generateMesh()
{
    uint8_t* data = mData.getData();

    std::vector<uint16_t> Indices = {
        0,1,2, 2,3,0
    };

    int forwardIndices = 0;

    for(int x = 0; x < CHUNKSIZE; x++)
        for (int y = 0; y < CHUNKHEIGHT; y++)
        for (int z = 0; z < CHUNKSIZE; z++)
        {
            
            BLOCKTYPE bType = (BLOCKTYPE)data[static_cast<int>(x * CHUNKHEIGHT * CHUNKSIZE + y * CHUNKSIZE + z)];
            if (bType == AIR) continue;
            // add entire cubes for now
            mMeshVertices.reserve(mMeshVertices.size() + 24);

            uint8_t frontTexture = getBlockTextureIndex(bType, BLOCKFACE::FRONT);
            uint8_t backTexture = getBlockTextureIndex(bType, BLOCKFACE::BACK);
            uint8_t leftTexture = getBlockTextureIndex(bType, BLOCKFACE::LEFT);
            uint8_t rightTexture = getBlockTextureIndex(bType, BLOCKFACE::RIGHT);
            uint8_t topTexture = getBlockTextureIndex(bType, BLOCKFACE::TOP);
            uint8_t bottomTexture = getBlockTextureIndex(bType, BLOCKFACE::BOTTOM);

            

            if (mData.isFaceVisible({ x, y, z }, FRONT))
            {
                mMeshVertices.push_back(Vertex{ glm::vec3(x, y + 1.0f, z), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2((frontTexture % 10) * 0.1f, (frontTexture / 10) * 0.1f + 0.1f) });
                mMeshVertices.push_back(Vertex{ glm::vec3(x, y, z), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2((frontTexture % 10) * 0.1f, (frontTexture / 10) * 0.1f) });
                mMeshVertices.push_back(Vertex{ glm::vec3(x + 1.0f,  y, z), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2((frontTexture % 10) * 0.1f + 0.1f, (frontTexture / 10) * 0.1f) });
                mMeshVertices.push_back(Vertex{ glm::vec3(x + 1.0f,  y + 1.0f, z), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2((frontTexture % 10) * 0.1f + 0.1f, (frontTexture / 10) * 0.1f + 0.1f) });

                for (const uint16_t& index : Indices)
                    mMeshIndices.push_back(index + forwardIndices);
                forwardIndices += 4;
            }

            if (mData.isFaceVisible({ x,y,z }, BACK))
            {
                mMeshVertices.push_back(Vertex{ glm::vec3(x + 1.0f,  y + 1.0f, z + 1.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2((backTexture % 10) * 0.1f,        (backTexture / 10) * 0.1f + 0.1f) });
                mMeshVertices.push_back(Vertex{ glm::vec3(x + 1.0f,  y,        z + 1.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2((backTexture % 10) * 0.1f,        (backTexture / 10) * 0.1f) });
                mMeshVertices.push_back(Vertex{ glm::vec3(x,         y,        z + 1.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2((backTexture % 10) * 0.1f + 0.1f, (backTexture / 10) * 0.1f) });
                mMeshVertices.push_back(Vertex{ glm::vec3(x,         y + 1.0f, z + 1.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2((backTexture % 10) * 0.1f + 0.1f,        (backTexture / 10) * 0.1f + 0.1f) });

                for (const uint16_t& index : Indices)
                    mMeshIndices.push_back(index + forwardIndices);
                forwardIndices += 4;
            }

            if (mData.isFaceVisible({ x,y,z }, LEFT))
            {
                mMeshVertices.push_back(Vertex{ glm::vec3(x,  y,        z + 1.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2((leftTexture % 10) * 0.1f,        (leftTexture / 10) * 0.1f) });
                mMeshVertices.push_back(Vertex{ glm::vec3(x,  y,        z),        glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2((leftTexture % 10) * 0.1f + 0.1f, (leftTexture / 10) * 0.1f) });
                mMeshVertices.push_back(Vertex{ glm::vec3(x,  y + 1.0f, z),        glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2((leftTexture % 10) * 0.1f + 0.1f, (leftTexture / 10) * 0.1f + 0.1f) });
                mMeshVertices.push_back(Vertex{ glm::vec3(x,  y + 1.0f, z + 1.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2((leftTexture % 10) * 0.1f,        (leftTexture / 10) * 0.1f + 0.1f) });

                for (const uint16_t& index : Indices)
                    mMeshIndices.push_back(index + forwardIndices);
                forwardIndices += 4;
            }

            if (mData.isFaceVisible({ x,y,z }, RIGHT))
            {
                mMeshVertices.push_back(Vertex{ glm::vec3(x + 1.0f,  y,        z + 1.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2((rightTexture % 10) * 0.1f,        (rightTexture / 10) * 0.1f) });
                mMeshVertices.push_back(Vertex{ glm::vec3(x + 1.0f,  y,        z),        glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2((rightTexture % 10) * 0.1f + 0.1f,        (rightTexture / 10) * 0.1f) });
                mMeshVertices.push_back(Vertex{ glm::vec3(x + 1.0f,  y + 1.0f, z),        glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2((rightTexture % 10) * 0.1f + 0.1f, (rightTexture / 10) * 0.1f + 0.1f) });
                mMeshVertices.push_back(Vertex{ glm::vec3(x + 1.0f,  y + 1.0f, z + 1.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2((rightTexture % 10) * 0.1f, (rightTexture / 10) * 0.1f + 0.1f) });

                for (const uint16_t& index : Indices)
                    mMeshIndices.push_back(index + forwardIndices);
                forwardIndices += 4;
            }

            if (mData.isFaceVisible({ x,y,z }, TOP))
            {
                mMeshVertices.push_back(Vertex{ glm::vec3(x,        y, z),        glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2((topTexture % 10) * 0.1f,        (topTexture / 10) * 0.1f + 0.1f) });
                mMeshVertices.push_back(Vertex{ glm::vec3(x,        y, z + 1.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2((topTexture % 10) * 0.1f,        (topTexture / 10) * 0.1f) });
                mMeshVertices.push_back(Vertex{ glm::vec3(x + 1.0f, y, z + 1.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2((topTexture % 10) * 0.1f + 0.1f, (topTexture / 10) * 0.1f) });
                mMeshVertices.push_back(Vertex{ glm::vec3(x + 1.0f, y, z),        glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2((topTexture % 10) * 0.1f + 0.1f,        (topTexture / 10) * 0.1f + 0.1f) });

                for (const uint16_t& index : Indices)
                    mMeshIndices.push_back(index + forwardIndices);
                forwardIndices += 4;
            }
            if (mData.isFaceVisible({ x,y,z }, BOTTOM))
            {
                mMeshVertices.push_back(Vertex{ glm::vec3(x,        y + 1.0f, z),         glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2((bottomTexture % 10) * 0.1f,        (bottomTexture / 10) * 0.1f + 0.1f) });
                mMeshVertices.push_back(Vertex{ glm::vec3(x,        y + 1.0f, z + 1.0f),  glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2((bottomTexture % 10) * 0.1f,        (bottomTexture / 10) * 0.1f) });
                mMeshVertices.push_back(Vertex{ glm::vec3(x + 1.0f, y + 1.0f, z + 1.0f),  glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2((bottomTexture % 10) * 0.1f + 0.1f, (bottomTexture / 10) * 0.1f) });
                mMeshVertices.push_back(Vertex{ glm::vec3(x + 1.0f, y + 1.0f, z),         glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2((bottomTexture % 10) * 0.1f + 0.1f,        (bottomTexture / 10) * 0.1f + 0.1f) });

                for (const uint16_t& index : Indices)
                    mMeshIndices.push_back(index + forwardIndices);
                forwardIndices += 4;
            }
        }
    GraphicsEngine::createVertexBuffer(mMeshVertices, mVertexBuffer, mVertexBufferMemory);
    GraphicsEngine::createIndexBuffer(mMeshIndices, mIndexBuffer, mIndexBufferMemory);

    
}

void Chunk::Render(VkCommandBuffer commandBuffer)
{
    VkBuffer vertexBuffers[] = { mVertexBuffer };
    VkDeviceSize offsets[] = { 0 };
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
    vkCmdBindIndexBuffer(commandBuffer, mIndexBuffer, 0, VK_INDEX_TYPE_UINT16);

    vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(mMeshIndices.size()), 1, 0, 0, 0);
}

glm::ivec2 Chunk::getPosition() const
{
    return mWorldPosition;
}

void Chunk::destroyChunk()
{
    VkDevice device = GraphicsEngine::getDevice();

    vkFreeMemory(device, mIndexBufferMemory, nullptr);
    vkDestroyBuffer(device, mIndexBuffer, nullptr);
    vkFreeMemory(device, mVertexBufferMemory, nullptr);
    vkDestroyBuffer(device, mVertexBuffer, nullptr);
}

ChunkData::ChunkData()
{
    pData = new uint8_t[CHUNKSIZE * CHUNKHEIGHT * CHUNKSIZE];
    allocateChunkData();
}

ChunkData::~ChunkData()
{
    delete[] pData;
}

bool ChunkData::allocateChunkData()
{
    for (size_t x = 0; x < CHUNKSIZE; x++)
        for (size_t y = 0; y < CHUNKHEIGHT; y++)
            for (size_t z = 0; z < CHUNKSIZE; z++)
            {
                if(y == 0)
                    pData[x * CHUNKHEIGHT * CHUNKSIZE + y * CHUNKSIZE + z] = static_cast<uint8_t>(BLOCKTYPE::GRASS);
                else if(y > 0 && y < 5)
                    pData[x * CHUNKHEIGHT * CHUNKSIZE + y * CHUNKSIZE + z] = static_cast<uint8_t>(BLOCKTYPE::DIRT);
                else
                    pData[x * CHUNKHEIGHT * CHUNKSIZE + y * CHUNKSIZE + z] = static_cast<uint8_t>(BLOCKTYPE::STONE);
            }
            

    return true;
}

uint8_t* ChunkData::getData()
{
    return pData;
}

bool ChunkData::isFaceVisible(glm::ivec3 blockPos, BLOCKFACE face)
{

    // FRONT = Z-
    // BACK = Z+
    // LEFT = X-
    // RIGHT = X+
    // BOTTOM = Y+
    // TOP = Y-
    int idx = 0;
    switch (face)
    {
    case FRONT:
        idx = getBlockIndex({ blockPos.x, blockPos.y, blockPos.z - 1 });
        break;
    case BACK:
        idx = getBlockIndex({ blockPos.x, blockPos.y, blockPos.z + 1 });
        break;
    case LEFT:
        idx = getBlockIndex({ blockPos.x - 1, blockPos.y, blockPos.z });
        break;
    case RIGHT:
        idx = getBlockIndex({ blockPos.x + 1, blockPos.y, blockPos.z });
        break;
    case BOTTOM:
        idx = getBlockIndex({ blockPos.x, blockPos.y + 1, blockPos.z });
        break;
    case TOP:
        idx = getBlockIndex({ blockPos.x, blockPos.y - 1, blockPos.z });
        break;
    }
    if (idx < 0 || idx >= CHUNKHEIGHT * CHUNKSIZE * CHUNKSIZE) return true; // if out of array bounds
    
    if (pData[idx] == AIR) return true;

    return false;
}

int ChunkData::getBlockIndex(glm::ivec3 blockCoords)
{
    if (blockCoords.x >= CHUNKSIZE || blockCoords.x < 0 || blockCoords.y >= CHUNKHEIGHT || blockCoords.y < 0 || blockCoords.z >= CHUNKSIZE || blockCoords.z < 0)
        return -1;
    return blockCoords.x * CHUNKHEIGHT * CHUNKSIZE + blockCoords.y * CHUNKSIZE + blockCoords.z;
}

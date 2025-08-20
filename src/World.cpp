#include "World.h"
#include "GraphicsEngine.h"
#include <array>
#include <glm/glm.hpp>
#include "PerlinNoise.hpp"


Chunk::Chunk(glm::ivec2 aWorldPos)
    :mWorldPosition(aWorldPos), mData(aWorldPos)
{
    generateMesh();
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

    //mMeshVertices.clear();
    //mMeshIndices.clear();
    
}

void Chunk::Render(VkCommandBuffer commandBuffer) const
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

void Chunk::destroyChunk() const
{
    VkDevice device = GraphicsEngine::getDevice();

    vkDestroyBuffer(device, mIndexBuffer, nullptr);
    vkFreeMemory(device, mIndexBufferMemory, nullptr);
    vkDestroyBuffer(device, mVertexBuffer, nullptr);
    vkFreeMemory(device, mVertexBufferMemory, nullptr);
    

}

bool Chunk::isPendingDeletion() const
{
    return pendingDeletion;
}

void Chunk::setPendingDeletionStatus(bool val)
{
    pendingDeletion = val;
}

ChunkData::ChunkData(glm::ivec2 chunkCoords)
{
    pData = new uint8_t[CHUNKSIZE * CHUNKHEIGHT * CHUNKSIZE];
    allocateChunkData(chunkCoords);
}

ChunkData::~ChunkData()
{
    delete[] pData;
}

bool ChunkData::allocateChunkData(glm::ivec2 chunkCoords)
{
    static double scale = 0.01;
    static int octaves = 6;
    static double persistance = 0.5;

    
    static siv::BasicPerlinNoise<float> noise(static_cast<uint32_t>(glfwGetTime() * 1000));
    for (size_t x = 0; x < CHUNKSIZE; x++)
        for (size_t y = 0; y < CHUNKHEIGHT; y++)
            for (size_t z = 0; z < CHUNKSIZE; z++)
            {
                int64_t globalX = (uint64_t)chunkCoords.x * CHUNKSIZE + x;
                int64_t globalZ = (uint64_t)chunkCoords.y * CHUNKSIZE + z;

                double nx = globalX * scale;
                double nz = globalZ * scale;

                double n = noise.normalizedOctave2D_01(nx, nz, octaves, persistance);

                uint16_t height = CHUNKHEIGHT * n;
                
                if (y < height)
                    pData[x * CHUNKHEIGHT * CHUNKSIZE + y * CHUNKSIZE + z] = BLOCKTYPE::AIR;          // above terrain
                else if (y == height)
                    pData[x * CHUNKHEIGHT * CHUNKSIZE + y * CHUNKSIZE + z] = BLOCKTYPE::GRASS;        // surface
                else if (y <= height + 5)
                    pData[x * CHUNKHEIGHT * CHUNKSIZE + y * CHUNKSIZE + z] = BLOCKTYPE::DIRT;         // top layers below surface
                else
                    pData[x * CHUNKHEIGHT * CHUNKSIZE + y * CHUNKSIZE + z] = BLOCKTYPE::STONE;
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

    

    if (idx < 0 || static_cast<size_t>(idx) >= CHUNKHEIGHT * CHUNKSIZE * CHUNKSIZE) return true; // if out of array bounds

    assert(pData);

    if (pData[idx] == AIR) return true;


    return false;
}

int ChunkData::getBlockIndex(glm::ivec3 blockCoords)
{
    if (blockCoords.x >= CHUNKSIZE || blockCoords.x < 0 || blockCoords.y >= CHUNKHEIGHT || blockCoords.y < 0 || blockCoords.z >= CHUNKSIZE || blockCoords.z < 0)
        return -1;
    return blockCoords.x * CHUNKHEIGHT * CHUNKSIZE + blockCoords.y * CHUNKSIZE + blockCoords.z;
}

Planet::Planet()
{

}

void Planet::InitDestructionList(VkDevice& pDevice)
{
    for (auto& list : mAwaitngDestruction)
    {
        list.Init(pDevice);
    }
}

void Planet::Update(glm::vec2 playerPosition, uint32_t currentFrame)
{
    static glm::ivec2 lastChunk = { 1,1 };

    int chunkX, chunkY;
    chunkX = static_cast<int>(playerPosition.x) / CHUNKSIZE;
    chunkY = static_cast<int>(playerPosition.y) / CHUNKSIZE;

    if (lastChunk != glm::ivec2(chunkX, chunkY))
        onPlayerCrossedChunk({chunkX, chunkY}, currentFrame);

    lastChunk = { chunkX, chunkY };


}



void Planet::onPlayerCrossedChunk(glm::ivec2 plrChunk, uint32_t currentFrame)
{
    double maxDistanceFromPlayer = renderDistance * sqrt(2.0);

    for (auto it = mChunks.begin(); it != mChunks.end();)
    {
        glm::ivec2 delta = it->first - plrChunk;
        double distanceFromPlayer = static_cast<double>(delta.x * delta.x + delta.y * delta.y);

        if (distanceFromPlayer > maxDistanceFromPlayer * maxDistanceFromPlayer)
        {
            mAwaitngDestruction[currentFrame].chunkList.push_back(&it->second);
            it->second.setPendingDeletionStatus(true);
            
        }
        
        ++it;
    }
    for(int i = -renderDistance; i <= renderDistance; i++) 
        for (int j = -renderDistance; j <= renderDistance; j++)
        {
            glm::ivec2 newChunkCoords = { plrChunk.x + i, plrChunk.y + j };
            if (mChunks.count(newChunkCoords) == 0)
            {
                mChunks.try_emplace(newChunkCoords, newChunkCoords);
            }
        }
}

void Planet::Render(VkCommandBuffer commandBuffer, VkPipelineLayout& layout)
{
    for (const auto& chunk : mChunks)
    {
        if (chunk.second.isPendingDeletion())
            continue;
        
        PushConstants pConsts{};
        pConsts.model = glm::translate(glm::mat4(1.0f), glm::vec3(chunk.first.x * CHUNKSIZE, 0.0f, chunk.first.y * CHUNKSIZE));
        vkCmdPushConstants(commandBuffer, layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PushConstants), &pConsts);
        chunk.second.Render(commandBuffer);
    }
}

void Planet::Cleanup(uint32_t currentFrame, VkDevice& device)
{
    //vkWaitForFences(device, 1, &mAwaitngDestruction[currentFrame].fence, VK_TRUE, INT64_MAX);
    for (auto chunk : mAwaitngDestruction[currentFrame].chunkList)
    {
        chunk->destroyChunk();
        mChunks.erase(chunk->getPosition());
    }
    mAwaitngDestruction[currentFrame].chunkList.clear();
}

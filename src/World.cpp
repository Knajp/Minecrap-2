#include "World.h"
#include "GraphicsEngine.h"
#include <array>
#include <glm/glm.hpp>
#include "PerlinNoise.hpp"
#include <random>


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


    int forwardIndicesFull = 0, forwardIndicesTransparent = 0;
    std::vector<Vertex>* targetVertexVector;
    std::vector<uint16_t>* targetIndexVector;

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

            int& forwardIndices = (bType == LEAVES || bType == TALLGRASS) ? forwardIndicesTransparent : forwardIndicesFull;

            targetVertexVector = &mMeshVertices;
            targetIndexVector = &mMeshIndices;
            
            if (bType == LEAVES || bType == TALLGRASS)
            {
                targetVertexVector = &mTransparentMeshVertices;
                targetIndexVector = &mTransparentMeshIndices;
            }

            if (mData.isFaceVisible({ x, y, z }, FRONT))
            {
                targetVertexVector->push_back(Vertex{ glm::vec3(x, y + 1.0f, z), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2((frontTexture % 10) * 0.1f, (frontTexture / 10) * 0.1f + 0.1f) });
                targetVertexVector->push_back(Vertex{ glm::vec3(x, y, z), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2((frontTexture % 10) * 0.1f, (frontTexture / 10) * 0.1f) });
                targetVertexVector->push_back(Vertex{ glm::vec3(x + 1.0f,  y, z), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2((frontTexture % 10) * 0.1f + 0.1f, (frontTexture / 10) * 0.1f) });
                targetVertexVector->push_back(Vertex{ glm::vec3(x + 1.0f,  y + 1.0f, z), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2((frontTexture % 10) * 0.1f + 0.1f, (frontTexture / 10) * 0.1f + 0.1f) });

                for (const uint16_t& index : Indices)
                    targetIndexVector->push_back(index + forwardIndices);
                forwardIndices += 4;
            }

            if (mData.isFaceVisible({ x,y,z }, BACK))
            {
                targetVertexVector->push_back(Vertex{ glm::vec3(x + 1.0f,  y + 1.0f, z + 1.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2((backTexture % 10) * 0.1f,        (backTexture / 10) * 0.1f + 0.1f) });
                targetVertexVector->push_back(Vertex{ glm::vec3(x + 1.0f,  y,        z + 1.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2((backTexture % 10) * 0.1f,        (backTexture / 10) * 0.1f) });
                targetVertexVector->push_back(Vertex{ glm::vec3(x,         y,        z + 1.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2((backTexture % 10) * 0.1f + 0.1f, (backTexture / 10) * 0.1f) });
                targetVertexVector->push_back(Vertex{ glm::vec3(x,         y + 1.0f, z + 1.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2((backTexture % 10) * 0.1f + 0.1f,        (backTexture / 10) * 0.1f + 0.1f) });

                for (const uint16_t& index : Indices)
                    targetIndexVector->push_back(index + forwardIndices);
                forwardIndices += 4;
            }

            if (mData.isFaceVisible({ x,y,z }, LEFT))
            {
                targetVertexVector->push_back(Vertex{ glm::vec3(x,  y,        z + 1.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2((leftTexture % 10) * 0.1f,        (leftTexture / 10) * 0.1f) });
                targetVertexVector->push_back(Vertex{ glm::vec3(x,  y,        z),        glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2((leftTexture % 10) * 0.1f + 0.1f, (leftTexture / 10) * 0.1f) });
                targetVertexVector->push_back(Vertex{ glm::vec3(x,  y + 1.0f, z),        glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2((leftTexture % 10) * 0.1f + 0.1f, (leftTexture / 10) * 0.1f + 0.1f) });
                targetVertexVector->push_back(Vertex{ glm::vec3(x,  y + 1.0f, z + 1.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2((leftTexture % 10) * 0.1f,        (leftTexture / 10) * 0.1f + 0.1f) });

                for (const uint16_t& index : Indices)
                    targetIndexVector->push_back(index + forwardIndices);
                forwardIndices += 4;
            }

            if (mData.isFaceVisible({ x,y,z }, RIGHT))
            {
                targetVertexVector->push_back(Vertex{ glm::vec3(x + 1.0f,  y,        z + 1.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2((rightTexture % 10) * 0.1f,        (rightTexture / 10) * 0.1f) });
                targetVertexVector->push_back(Vertex{ glm::vec3(x + 1.0f,  y,        z),        glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2((rightTexture % 10) * 0.1f + 0.1f,        (rightTexture / 10) * 0.1f) });
                targetVertexVector->push_back(Vertex{ glm::vec3(x + 1.0f,  y + 1.0f, z),        glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2((rightTexture % 10) * 0.1f + 0.1f, (rightTexture / 10) * 0.1f + 0.1f) });
                targetVertexVector->push_back(Vertex{ glm::vec3(x + 1.0f,  y + 1.0f, z + 1.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2((rightTexture % 10) * 0.1f, (rightTexture / 10) * 0.1f + 0.1f) });

                for (const uint16_t& index : Indices)
                    targetIndexVector->push_back(index + forwardIndices);
                forwardIndices += 4;
            }

            if (mData.isFaceVisible({ x,y,z }, TOP))
            {
                targetVertexVector->push_back(Vertex{ glm::vec3(x,        y, z),        glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2((topTexture % 10) * 0.1f,        (topTexture / 10) * 0.1f + 0.1f) });
                targetVertexVector->push_back(Vertex{ glm::vec3(x,        y, z + 1.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2((topTexture % 10) * 0.1f,        (topTexture / 10) * 0.1f) });
                targetVertexVector->push_back(Vertex{ glm::vec3(x + 1.0f, y, z + 1.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2((topTexture % 10) * 0.1f + 0.1f, (topTexture / 10) * 0.1f) });
                targetVertexVector->push_back(Vertex{ glm::vec3(x + 1.0f, y, z),        glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2((topTexture % 10) * 0.1f + 0.1f,        (topTexture / 10) * 0.1f + 0.1f) });

                for (const uint16_t& index : Indices)
                    targetIndexVector->push_back(index + forwardIndices);
                forwardIndices += 4;
            }
            if (mData.isFaceVisible({ x,y,z }, BOTTOM))
            {
                targetVertexVector->push_back(Vertex{ glm::vec3(x,        y + 1.0f, z),         glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2((bottomTexture % 10) * 0.1f,        (bottomTexture / 10) * 0.1f + 0.1f) });
                targetVertexVector->push_back(Vertex{ glm::vec3(x,        y + 1.0f, z + 1.0f),  glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2((bottomTexture % 10) * 0.1f,        (bottomTexture / 10) * 0.1f) });
                targetVertexVector->push_back(Vertex{ glm::vec3(x + 1.0f, y + 1.0f, z + 1.0f),  glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2((bottomTexture % 10) * 0.1f + 0.1f, (bottomTexture / 10) * 0.1f) });
                targetVertexVector->push_back(Vertex{ glm::vec3(x + 1.0f, y + 1.0f, z),         glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2((bottomTexture % 10) * 0.1f + 0.1f,        (bottomTexture / 10) * 0.1f + 0.1f) });

                for (const uint16_t& index : Indices)
                    targetIndexVector->push_back(index + forwardIndices);
                forwardIndices += 4;
            }
        }
    GraphicsEngine::createVertexBuffer(mMeshVertices, mVertexBuffer, mVertexBufferMemory);
    GraphicsEngine::createIndexBuffer(mMeshIndices, mIndexBuffer, mIndexBufferMemory);

    if (!mTransparentMeshVertices.empty())
    {
        GraphicsEngine::createVertexBuffer(mTransparentMeshVertices, mTransparentVertexBuffer, mTransparentVertexBuffferMemory);
        GraphicsEngine::createIndexBuffer(mTransparentMeshIndices, mTransparentIndexBuffer, mTransparentIndexBufferMemory);
    }
    

    //mMeshVertices.clear();
    //mMeshIndices.clear();
    
}

void Chunk::Render(VkCommandBuffer commandBuffer) const
{
    VkBuffer vertexBuffers[] = { mVertexBuffer };
    VkBuffer transparentVertexBuffers[] = { mTransparentVertexBuffer };
    VkDeviceSize offsets[] = { 0 };


    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
    vkCmdBindIndexBuffer(commandBuffer, mIndexBuffer, 0, VK_INDEX_TYPE_UINT16);

    vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(mMeshIndices.size()), 1, 0, 0, 0);

    if (!mTransparentMeshVertices.empty())
    {
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, transparentVertexBuffers, offsets);
        vkCmdBindIndexBuffer(commandBuffer, mTransparentIndexBuffer, 0, VK_INDEX_TYPE_UINT16);

        vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(mTransparentMeshIndices.size()), 1, 0, 0, 0);
    }
    
}

glm::ivec2 Chunk::getPosition() const
{
    return mWorldPosition;
}

void Chunk::destroyChunk() const
{
    VkDevice device = GraphicsEngine::getDevice();
    if (!mTransparentMeshVertices.empty())
    {
        vkDestroyBuffer(device, mTransparentVertexBuffer, nullptr);
        vkFreeMemory(device, mTransparentVertexBuffferMemory, nullptr);
        vkDestroyBuffer(device, mTransparentIndexBuffer, nullptr);
        vkFreeMemory(device, mTransparentIndexBufferMemory, nullptr);
    }
    
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
    static float scale = 0.01f;
    static int octaves = 6;
    static float persistance = 0.48f;

    
    static siv::BasicPerlinNoise<float> noise(static_cast<uint32_t>(glfwGetTime() * 1000));
    for (size_t x = 0; x < CHUNKSIZE; x++)
        for (size_t y = 0; y < CHUNKHEIGHT; y++)
            for (size_t z = 0; z < CHUNKSIZE; z++)
            {
                int64_t globalX = (uint64_t)chunkCoords.x * CHUNKSIZE + x;
                int64_t globalZ = (uint64_t)chunkCoords.y * CHUNKSIZE + z;

                float nx = globalX * scale;
                float nz = globalZ * scale;

                double n = noise.normalizedOctave2D_01(nx, nz, octaves, persistance);

                uint16_t height = static_cast<uint16_t>(CHUNKHEIGHT * n);
                
                if (y < height)
                    pData[x * CHUNKHEIGHT * CHUNKSIZE + y * CHUNKSIZE + z] = BLOCKTYPE::AIR;          // above terrain
                else if (y == height)
                    pData[x * CHUNKHEIGHT * CHUNKSIZE + y * CHUNKSIZE + z] = BLOCKTYPE::GRASS;        // surface
                else if (y <= (size_t)height + 5)
                    pData[x * CHUNKHEIGHT * CHUNKSIZE + y * CHUNKSIZE + z] = BLOCKTYPE::DIRT;         // top layers below surface
                else
                    pData[x * CHUNKHEIGHT * CHUNKSIZE + y * CHUNKSIZE + z] = BLOCKTYPE::STONE;
            }
    generateGrass(chunkCoords);
    generateTrees(chunkCoords);
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

    

    if (idx < 0 || static_cast<size_t>(idx) >= (size_t)CHUNKHEIGHT * (size_t)CHUNKSIZE * (size_t)CHUNKSIZE) return true; // if out of array bounds

    assert(pData);

    if (pData[idx] == AIR || pData[idx] == TALLGRASS|| (pData[idx] == LEAVES && pData[getBlockIndex(blockPos)] != LEAVES)) return true;


    return false;
}

int ChunkData::getBlockIndex(glm::ivec3 blockCoords)
{
    if (blockCoords.x >= CHUNKSIZE || blockCoords.x < 0 || blockCoords.y >= CHUNKHEIGHT || blockCoords.y < 0 || blockCoords.z >= CHUNKSIZE || blockCoords.z < 0)
        return -1;
    return blockCoords.x * CHUNKHEIGHT * CHUNKSIZE + blockCoords.y * CHUNKSIZE + blockCoords.z;
}

void ChunkData::generateTrees(glm::ivec2 chunkCoords)
{
    static uint8_t minDistance = 4;

    static float scale = 0.3f;
    static int octaves = 6;
    static float persistance = 0.6f;

    static siv::BasicPerlinNoise<float> treeNoise(rand());

    std::vector<glm::ivec2> placedTrees = {};

    float globalX, globalZ;
    for (int x = 0; x < CHUNKSIZE; x++)
    {
        globalX = static_cast<float>(chunkCoords.x * CHUNKSIZE + x);
        globalX *= scale;
        for (int z = 0; z < CHUNKSIZE; z++)
        {
            globalZ = static_cast<float>(chunkCoords.y * CHUNKSIZE + z);
            globalZ *= scale;

            float noise = treeNoise.normalizedOctave2D_01(globalX, globalZ, octaves, persistance);

            if (noise > 0.65f && canPlaceTree(placedTrees, { x,z }))
            { 
                placedTrees.push_back({ x,z });
                placeTree({ x, getTopBlock({x,z}) , z });
            }
        }
    }
}

void ChunkData::placeTree(glm::ivec3 baseCoords)
{
    int idx = 0;
    for (int y = 3; y <= 6; y++)
        for(int x = -2; x <= 2; x++)
            for (int z = -2; z <= 2; z++)
            {
                if (y == 6 && abs(x) + abs(z) == 1)
                {
                    idx = getBlockIndex({ baseCoords.x + x, baseCoords.y - y, baseCoords.z + z });
                    if(idx != -1)
                        pData[idx] = LEAVES;
                }
                else if (y == 5 && abs(x) < 2 && abs(z) < 2)
                {
                    idx = getBlockIndex({ baseCoords.x + x, baseCoords.y - y, baseCoords.z + z });
                    if (idx != -1)
                        pData[idx] = LEAVES;
                }
                else if (y == 4 && abs(x) + abs(z) != 4)
                {
                    idx = getBlockIndex({ baseCoords.x + x, baseCoords.y - y, baseCoords.z + z });
                    if (idx != -1)
                        pData[idx] = LEAVES;
                }
                else if (y == 3)
                {
                    idx = getBlockIndex({ baseCoords.x + x, baseCoords.y - y, baseCoords.z + z });
                    if (idx != -1)
                        pData[idx] = LEAVES;
                }

            }
                
    pData[getBlockIndex({ baseCoords.x, baseCoords.y - 7, baseCoords.z })] = LEAVES;
    for (int y = 1; y <= 6; y++)
        pData[getBlockIndex({ baseCoords.x, baseCoords.y - y, baseCoords.z })] = WOOD;


}

int ChunkData::getTopBlock(glm::ivec2 coords)
{
    for (int i = 0; i < 256; i++)
    {
        if (pData[getBlockIndex({ coords.x, i, coords.y })] != AIR) return i;
    }
    return 255;
}

bool ChunkData::canPlaceTree(const std::vector<glm::ivec2>& treeVector, glm::ivec2 coords)
{
    for (const auto& tree : treeVector)
    {
        glm::ivec2 deltaVector = coords - tree;

        if (deltaVector.x + deltaVector.y < 4) return false;
    }
    return true;
}

void ChunkData::generateGrass(glm::ivec2 chunkCoords)
{
    static float scale = 0.3f;
    static int octaves = 6;
    static float persistance = 0.6f;

    siv::BasicPerlinNoise<float> grassNoise;

    float globalX, globalZ;
    for (int x = 0; x <= CHUNKSIZE; x++)
    {
        globalX = chunkCoords.x * CHUNKSIZE + x;
        globalX *= scale;
        for (int z = 0; z <= CHUNKSIZE; z++)
        {
            globalZ = chunkCoords.y * CHUNKSIZE + z;
            globalZ *= scale;
            float noise = grassNoise.normalizedOctave2D_01(globalX, globalZ, octaves, persistance);

            if (noise > 0.7f)
                pData[getBlockIndex({ x, getTopBlock({x,z}) - 1, z })] = TALLGRASS;

        }
    }
}

Planet::Planet()
{
    srand(static_cast<uint32_t>(glfwGetTime()));
}

void Planet::prepareForDestruction()
{
    for (auto it = mChunks.begin(); it != mChunks.end();)
    {

        mAwaitngDestruction[2].chunkList.push_back(&it->second);
        it->second.setPendingDeletionStatus(true);

        ++it;
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
    double maxDistanceFromPlayer = renderDistance * 1.42;

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

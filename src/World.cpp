#include "World.h"
#include "GraphicsEngine.h"
#include <array>
#include <glm/glm.hpp>
#include "PerlinNoise.hpp"
#include <random>
#include <algorithm>
#include <mutex>
#include <future>
#include <chrono>

Chunk::Chunk(glm::ivec2 aWorldPos)
    :mWorldPosition(aWorldPos), mData(aWorldPos)
{
    boundingbox = std::make_unique<AABB>(aWorldPos);
    generateMesh();
}


Chunk::~Chunk()
{

}

Chunk::Chunk(Chunk&& other) noexcept
    : mMeshVertices(std::move(other.mMeshVertices)),
    mMeshIndices(std::move(other.mMeshIndices)),
    mTransparentMeshVertices(std::move(other.mTransparentMeshVertices)),
    mTransparentMeshIndices(std::move(other.mTransparentMeshIndices)),
    mWorldPosition(other.mWorldPosition),
    mData(std::move(other.mData)),
    mVertexBuffer(other.mVertexBuffer),
    mVertexBufferMemory(other.mVertexBufferMemory),
    mIndexBuffer(other.mIndexBuffer),
    mIndexBufferMemory(other.mIndexBufferMemory),
    mTransparentVertexBuffer(other.mTransparentVertexBuffer),
    mTransparentVertexBuffferMemory(other.mTransparentVertexBuffferMemory),
    mTransparentIndexBuffer(other.mTransparentIndexBuffer),
    mTransparentIndexBufferMemory(other.mTransparentIndexBufferMemory),
    pendingDeletion(other.pendingDeletion),
    boundingbox(std::move(other.boundingbox))
{
    // Invalidate the moved-from chunk's handles so they don't get destroyed twice
    other.mVertexBuffer = VK_NULL_HANDLE;
    other.mVertexBufferMemory = VK_NULL_HANDLE;
    other.mIndexBuffer = VK_NULL_HANDLE;
    other.mIndexBufferMemory = VK_NULL_HANDLE;

    other.mTransparentVertexBuffer = VK_NULL_HANDLE;
    other.mTransparentVertexBuffferMemory = VK_NULL_HANDLE;
    other.mTransparentIndexBuffer = VK_NULL_HANDLE;
    other.mTransparentIndexBufferMemory = VK_NULL_HANDLE;
}

void Chunk::generateMesh()
{
    uint8_t* data = mData.getData();

    std::vector<uint16_t> Indices = {
        0,1,2, 2,3,0
    };
    std::vector<uint16_t> billboardIndices =
    {
        0,1,2, 2,3,1,
        4,5,6, 6,7,5
    };


    int forwardIndicesFull = 0, forwardIndicesTransparent = 0;
    std::vector<Vertex>* targetVertexVector;
    std::vector<uint16_t>* targetIndexVector;

    uint32_t solidBlockCount = 16328 - mData.airBlockCount - mData.transparentBlockCount;


    mMeshVertices.clear();
    mMeshIndices.clear();
    mTransparentMeshVertices.clear();
    mTransparentMeshIndices.clear();
    
    mMeshVertices.reserve(solidBlockCount / 2 * 32);
    mMeshIndices.reserve(solidBlockCount * 36);

    mTransparentMeshVertices.reserve(mData.transparentBlockCount * 32);
    mTransparentMeshIndices.reserve(mData.transparentBlockCount * 36);

    for(int x = 0; x < CHUNKSIZE; x++)
        for (int z = 0; z < CHUNKSIZE; z++)
        for (int y = 0; y < CHUNKHEIGHT; y++)
        {
            
            BLOCKTYPE bType = (BLOCKTYPE)data[static_cast<int>(x * CHUNKHEIGHT * CHUNKSIZE + z * CHUNKHEIGHT + y)];
            if (bType == AIR) continue;
            uint8_t frontTexture = getBlockTextureIndex(bType, BLOCKFACE::FRONT);
            uint8_t backTexture = getBlockTextureIndex(bType, BLOCKFACE::BACK);
            uint8_t leftTexture = getBlockTextureIndex(bType, BLOCKFACE::LEFT);
            uint8_t rightTexture = getBlockTextureIndex(bType, BLOCKFACE::RIGHT);
            uint8_t topTexture = getBlockTextureIndex(bType, BLOCKFACE::TOP);
            uint8_t bottomTexture = getBlockTextureIndex(bType, BLOCKFACE::BOTTOM);

            int& forwardIndices = transparentBlocks.count(bType) != 0 ? forwardIndicesTransparent : forwardIndicesFull;

            targetVertexVector = &mMeshVertices;
            targetIndexVector = &mMeshIndices;
            
            if (bType == LEAVES || bType == TALLGRASS)
            {
                targetVertexVector = &mTransparentMeshVertices;
                targetIndexVector = &mTransparentMeshIndices;
            }

            if (billboards.count(bType) != 0)
            {
                targetVertexVector->push_back(Vertex{ glm::vec3(x + 0.1f, y + 1.0f, z + 0.1f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2((frontTexture % 10) * 0.1f, (frontTexture / 10) * 0.1f + 0.1f) });
                targetVertexVector->push_back(Vertex{ glm::vec3(x + 0.1f, y + 0.2f, z + 0.1f), glm::vec3(0.0f, 1.0f, 0.0f),        glm::vec2((frontTexture % 10) * 0.1f, (frontTexture / 10) * 0.1f) });
                targetVertexVector->push_back(Vertex{ glm::vec3(x + 0.9f, y + 1.0f, z + 0.9f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2((frontTexture % 10) * 0.1f + 0.1f, (frontTexture / 10) * 0.1f + 0.1f) });
                targetVertexVector->push_back(Vertex{ glm::vec3(x + 0.9f, y + 0.2f, z + 0.9f), glm::vec3(0.0f, 1.0f, 0.0f),       glm::vec2((frontTexture % 10) * 0.1f + 0.1f, (frontTexture / 10) * 0.1f) });

                targetVertexVector->push_back(Vertex{ glm::vec3(x + 0.1f, y + 1.0f, z + 0.9f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2((frontTexture % 10) * 0.1f, (frontTexture / 10) * 0.1f + 0.1f) });
                targetVertexVector->push_back(Vertex{ glm::vec3(x + 0.1f, y + 0.2f, z + 0.9f), glm::vec3(0.0f, 1.0f, 0.0f),        glm::vec2((frontTexture % 10) * 0.1f, (frontTexture / 10) * 0.1f) });
                targetVertexVector->push_back(Vertex{ glm::vec3(x + 0.9f, y + 1.0f, z + 0.1f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2((frontTexture % 10) * 0.1f + 0.1f, (frontTexture / 10) * 0.1f + 0.1f) });
                targetVertexVector->push_back(Vertex{ glm::vec3(x + 0.9f, y + 0.2f, z + 0.1f), glm::vec3(0.0f, 1.0f, 0.0f),       glm::vec2((frontTexture % 10) * 0.1f + 0.1f, (frontTexture / 10) * 0.1f) });

                const size_t count = billboardIndices.size();
                const size_t offset = targetIndexVector->size();

                targetIndexVector->resize(offset + count);

                memcpy(targetIndexVector->data() + offset, billboardIndices.data(), count * sizeof(uint16_t));

                uint16_t* dst = targetIndexVector->data() + offset;
                for (size_t i = 0; i < count; ++i)
                    dst[i] += forwardIndices;

                forwardIndices += 8;

            }
            else
            {
                if (mData.isFaceVisible({ x, y, z }, FRONT))
                {
                    targetVertexVector->push_back(Vertex{ glm::vec3(x, y + 1.0f, z), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2((frontTexture % 10) * 0.1f, (frontTexture / 10) * 0.1f + 0.1f) });
                    targetVertexVector->push_back(Vertex{ glm::vec3(x, y, z), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2((frontTexture % 10) * 0.1f, (frontTexture / 10) * 0.1f) });
                    targetVertexVector->push_back(Vertex{ glm::vec3(x + 1.0f,  y, z), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2((frontTexture % 10) * 0.1f + 0.1f, (frontTexture / 10) * 0.1f) });
                    targetVertexVector->push_back(Vertex{ glm::vec3(x + 1.0f,  y + 1.0f, z), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2((frontTexture % 10) * 0.1f + 0.1f, (frontTexture / 10) * 0.1f + 0.1f) });


                    const size_t count = Indices.size();
                    const size_t offset = targetIndexVector->size();

                    targetIndexVector->resize(offset + count);

                    memcpy(targetIndexVector->data() + offset, Indices.data(), count * sizeof(uint16_t));

                    uint16_t* dst = targetIndexVector->data() + offset;
                    for (size_t i = 0; i < count; ++i)
                        dst[i] += forwardIndices;

                    forwardIndices += 4;
                }

                if (mData.isFaceVisible({ x,y,z }, BACK))
                {
                    targetVertexVector->push_back(Vertex{ glm::vec3(x + 1.0f,  y + 1.0f, z + 1.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2((backTexture % 10) * 0.1f,        (backTexture / 10) * 0.1f + 0.1f) });
                    targetVertexVector->push_back(Vertex{ glm::vec3(x + 1.0f,  y,        z + 1.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2((backTexture % 10) * 0.1f,        (backTexture / 10) * 0.1f) });
                    targetVertexVector->push_back(Vertex{ glm::vec3(x,         y,        z + 1.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2((backTexture % 10) * 0.1f + 0.1f, (backTexture / 10) * 0.1f) });
                    targetVertexVector->push_back(Vertex{ glm::vec3(x,         y + 1.0f, z + 1.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2((backTexture % 10) * 0.1f + 0.1f,        (backTexture / 10) * 0.1f + 0.1f) });

                    const size_t count = Indices.size();
                    const size_t offset = targetIndexVector->size();

                    targetIndexVector->resize(offset + count);

                    memcpy(targetIndexVector->data() + offset, Indices.data(), count * sizeof(uint16_t));

                    uint16_t* dst = targetIndexVector->data() + offset;
                    for (size_t i = 0; i < count; ++i)
                        dst[i] += forwardIndices;

                    forwardIndices += 4;
                }

                if (mData.isFaceVisible({ x,y,z }, LEFT))
                {
                    targetVertexVector->push_back(Vertex{ glm::vec3(x,  y,        z + 1.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2((leftTexture % 10) * 0.1f,        (leftTexture / 10) * 0.1f) });
                    targetVertexVector->push_back(Vertex{ glm::vec3(x,  y,        z),        glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2((leftTexture % 10) * 0.1f + 0.1f, (leftTexture / 10) * 0.1f) });
                    targetVertexVector->push_back(Vertex{ glm::vec3(x,  y + 1.0f, z),        glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2((leftTexture % 10) * 0.1f + 0.1f, (leftTexture / 10) * 0.1f + 0.1f) });
                    targetVertexVector->push_back(Vertex{ glm::vec3(x,  y + 1.0f, z + 1.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2((leftTexture % 10) * 0.1f,        (leftTexture / 10) * 0.1f + 0.1f) });

                    const size_t count = Indices.size();
                    const size_t offset = targetIndexVector->size();

                    targetIndexVector->resize(offset + count);

                    memcpy(targetIndexVector->data() + offset, Indices.data(), count * sizeof(uint16_t));

                    uint16_t* dst = targetIndexVector->data() + offset;
                    for (size_t i = 0; i < count; ++i)
                        dst[i] += forwardIndices;

                    forwardIndices += 4;
                }

                if (mData.isFaceVisible({ x,y,z }, RIGHT))
                {
                    targetVertexVector->push_back(Vertex{ glm::vec3(x + 1.0f,  y,        z + 1.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2((rightTexture % 10) * 0.1f,        (rightTexture / 10) * 0.1f) });
                    targetVertexVector->push_back(Vertex{ glm::vec3(x + 1.0f,  y,        z),        glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2((rightTexture % 10) * 0.1f + 0.1f,        (rightTexture / 10) * 0.1f) });
                    targetVertexVector->push_back(Vertex{ glm::vec3(x + 1.0f,  y + 1.0f, z),        glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2((rightTexture % 10) * 0.1f + 0.1f, (rightTexture / 10) * 0.1f + 0.1f) });
                    targetVertexVector->push_back(Vertex{ glm::vec3(x + 1.0f,  y + 1.0f, z + 1.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2((rightTexture % 10) * 0.1f, (rightTexture / 10) * 0.1f + 0.1f) });

                    const size_t count = Indices.size();
                    const size_t offset = targetIndexVector->size();

                    targetIndexVector->resize(offset + count);

                    memcpy(targetIndexVector->data() + offset, Indices.data(), count * sizeof(uint16_t));

                    uint16_t* dst = targetIndexVector->data() + offset;
                    for (size_t i = 0; i < count; ++i)
                        dst[i] += forwardIndices;

                    forwardIndices += 4;
                }

                if (mData.isFaceVisible({ x,y,z }, TOP))
                {
                    targetVertexVector->push_back(Vertex{ glm::vec3(x,        y, z),        glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2((topTexture % 10) * 0.1f,        (topTexture / 10) * 0.1f + 0.1f) });
                    targetVertexVector->push_back(Vertex{ glm::vec3(x,        y, z + 1.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2((topTexture % 10) * 0.1f,        (topTexture / 10) * 0.1f) });
                    targetVertexVector->push_back(Vertex{ glm::vec3(x + 1.0f, y, z + 1.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2((topTexture % 10) * 0.1f + 0.1f, (topTexture / 10) * 0.1f) });
                    targetVertexVector->push_back(Vertex{ glm::vec3(x + 1.0f, y, z),        glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2((topTexture % 10) * 0.1f + 0.1f,        (topTexture / 10) * 0.1f + 0.1f) });

                    const size_t count = Indices.size();
                    const size_t offset = targetIndexVector->size();

                    targetIndexVector->resize(offset + count);

                    memcpy(targetIndexVector->data() + offset, Indices.data(), count * sizeof(uint16_t));

                    uint16_t* dst = targetIndexVector->data() + offset;
                    for (size_t i = 0; i < count; ++i)
                        dst[i] += forwardIndices;

                    forwardIndices += 4;
                }
                if (mData.isFaceVisible({ x,y,z }, BOTTOM))
                {
                    targetVertexVector->push_back(Vertex{ glm::vec3(x,        y + 1.0f, z),         glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2((bottomTexture % 10) * 0.1f,        (bottomTexture / 10) * 0.1f + 0.1f) });
                    targetVertexVector->push_back(Vertex{ glm::vec3(x,        y + 1.0f, z + 1.0f),  glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2((bottomTexture % 10) * 0.1f,        (bottomTexture / 10) * 0.1f) });
                    targetVertexVector->push_back(Vertex{ glm::vec3(x + 1.0f, y + 1.0f, z + 1.0f),  glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2((bottomTexture % 10) * 0.1f + 0.1f, (bottomTexture / 10) * 0.1f) });
                    targetVertexVector->push_back(Vertex{ glm::vec3(x + 1.0f, y + 1.0f, z),         glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2((bottomTexture % 10) * 0.1f + 0.1f,        (bottomTexture / 10) * 0.1f + 0.1f) });

                    const size_t count = Indices.size();
                    const size_t offset = targetIndexVector->size();

                    targetIndexVector->resize(offset + count);

                    memcpy(targetIndexVector->data() + offset, Indices.data(), count * sizeof(uint16_t));

                    uint16_t* dst = targetIndexVector->data() + offset;
                    for (size_t i = 0; i < count; ++i)
                        dst[i] += forwardIndices;

                    forwardIndices += 4;
                }
            }
        }
    
}

void Chunk::createGPUBuffers()
{
    // Destroy old opaque buffers if they exist
    if (mVertexBuffer != VK_NULL_HANDLE) {
        vkDestroyBuffer(GraphicsEngine::getDevice(), mVertexBuffer, nullptr);
        vkFreeMemory(GraphicsEngine::getDevice(), mVertexBufferMemory, nullptr);
        mVertexBuffer = VK_NULL_HANDLE;
        mVertexBufferMemory = VK_NULL_HANDLE;
    }

    if (mIndexBuffer != VK_NULL_HANDLE) {
        vkDestroyBuffer(GraphicsEngine::getDevice(), mIndexBuffer, nullptr);
        vkFreeMemory(GraphicsEngine::getDevice(), mIndexBufferMemory, nullptr);
        mIndexBuffer = VK_NULL_HANDLE;
        mIndexBufferMemory = VK_NULL_HANDLE;
    }

    // Create new opaque buffers
    GraphicsEngine::createVertexBuffer<Vertex>(mMeshVertices, mVertexBuffer, mVertexBufferMemory);
    GraphicsEngine::createIndexBuffer(mMeshIndices, mIndexBuffer, mIndexBufferMemory);

    // Destroy old transparent buffers if they exist
    if (!mTransparentMeshVertices.empty())
    {
        if (mTransparentVertexBuffer != VK_NULL_HANDLE) {
            vkDestroyBuffer(GraphicsEngine::getDevice(), mTransparentVertexBuffer, nullptr);
            vkFreeMemory(GraphicsEngine::getDevice(), mTransparentVertexBuffferMemory, nullptr);
            mTransparentVertexBuffer = VK_NULL_HANDLE;
            mTransparentVertexBuffferMemory = VK_NULL_HANDLE;
        }

        if (mTransparentIndexBuffer != VK_NULL_HANDLE) {
            vkDestroyBuffer(GraphicsEngine::getDevice(), mTransparentIndexBuffer, nullptr);
            vkFreeMemory(GraphicsEngine::getDevice(), mTransparentIndexBufferMemory, nullptr);
            mTransparentIndexBuffer = VK_NULL_HANDLE;
            mTransparentIndexBufferMemory = VK_NULL_HANDLE;
        }

        // Create new transparent buffers
        GraphicsEngine::createVertexBuffer<Vertex>(mTransparentMeshVertices, mTransparentVertexBuffer, mTransparentVertexBuffferMemory);
        GraphicsEngine::createIndexBuffer(mTransparentMeshIndices, mTransparentIndexBuffer, mTransparentIndexBufferMemory);
    }
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

bool Chunk::hasGPUbuffers() const 
{
    return mVertexBuffer != VK_NULL_HANDLE;
}

ChunkData::ChunkData(glm::ivec2 chunkCoords)
{
    pData = std::make_unique<uint8_t[]>(CHUNKSIZE * CHUNKHEIGHT * CHUNKSIZE);
    mMissingBlocks = std::make_unique<std::unordered_map<glm::ivec3, BLOCKTYPE>>();

    allocateChunkData(chunkCoords);
}

ChunkData::~ChunkData()
{
}

bool ChunkData::allocateChunkData(glm::ivec2 chunkCoords)
{
    static float scale = 0.01f;
    static int octaves = 6;
    static float persistance = 0.48f;

    static uint32_t seed = static_cast<uint32_t>(glfwGetTime() * 1000);

    std::mutex airBlockMutex;
    unsigned maxThreads = std::max(1u, std::thread::hardware_concurrency());
    std::vector<std::future<void>> futures;

    for (size_t x = 0; x < CHUNKSIZE; x++)
        for (size_t z = 0; z < CHUNKSIZE; z++)
        {
            futures.emplace_back(std::async(std::launch::async, [this, &chunkCoords, x, z, &airBlockMutex] {

                int64_t globalX = (uint64_t)chunkCoords.x * CHUNKSIZE + x;
                int64_t globalZ = (uint64_t)chunkCoords.y * CHUNKSIZE + z;

                float nx = globalX * scale;
                float nz = globalZ * scale;

                siv::BasicPerlinNoise<float> noise(seed);

                double n = noise.normalizedOctave2D_01(nx, nz, octaves, persistance);

                uint16_t height = static_cast<uint16_t>(CHUNKHEIGHT * n);

                for (size_t y = 0; y < CHUNKHEIGHT; y++)
                {
                    if (y < height)
                    {
                        pData[x * CHUNKHEIGHT * CHUNKSIZE + z * CHUNKHEIGHT + y] = BLOCKTYPE::AIR;

                        {
                            std::lock_guard<std::mutex> lock(airBlockMutex);
                            airBlockCount++;
                        }
                        
                    }
                    else if (y == height)
                        pData[x * CHUNKHEIGHT * CHUNKSIZE + z * CHUNKHEIGHT + y] = BLOCKTYPE::GRASS;        // surface
                    else if (y <= (size_t)height + 5)
                        pData[x * CHUNKHEIGHT * CHUNKSIZE + z * CHUNKHEIGHT + y] = BLOCKTYPE::DIRT;         // top layers below surface
                    else
                        pData[x * CHUNKHEIGHT * CHUNKSIZE + z * CHUNKHEIGHT + y] = BLOCKTYPE::STONE;
                }
                }));

            if (futures.size() >= maxThreads)
            {
                for (auto& f : futures)
                    f.get();
                futures.clear();
            }
            
        }
    
    for (auto& f : futures)
        f.get();

    generateGrass(chunkCoords);
    generateTrees(chunkCoords);

    return true;
}

uint8_t* ChunkData::getData()
{
    return pData.get();
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

    if (pData[idx] == AIR || Chunk::transparentBlocks.count((BLOCKTYPE)pData[idx]) != 0)  return true;

    return false;
}

bool ChunkData::isBlockTransparent(BLOCKTYPE type)
{
    return false;
}

int ChunkData::getBlockIndex(glm::ivec3 blockCoords)
{
    if (blockCoords.x >= CHUNKSIZE || blockCoords.x < 0 || blockCoords.y >= CHUNKHEIGHT || blockCoords.y < 0 || blockCoords.z >= CHUNKSIZE || blockCoords.z < 0)
        return -1;
    return blockCoords.x * CHUNKHEIGHT * CHUNKSIZE + blockCoords.z * CHUNKHEIGHT + blockCoords.y;
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
                idx = getBlockIndex({ baseCoords.x + x, baseCoords.y - y, baseCoords.z + z });

                if (y == 6 && abs(x) + abs(z) == 1)
                {
                    if (idx != -1) pData[idx] = LEAVES;
                    else mMissingBlocks->emplace(std::make_pair(glm::ivec3(baseCoords.x + x, baseCoords.y - y, baseCoords.z + z), LEAVES));
                }
                else if (y == 5 && abs(x) < 2 && abs(z) < 2)
                {
                    if (idx != -1) pData[idx] = LEAVES;
                    else mMissingBlocks->emplace(std::make_pair(glm::ivec3(baseCoords.x + x, baseCoords.y - y, baseCoords.z + z), LEAVES));
                }
                else if (y == 4 && abs(x) + abs(z) != 4)
                {
                    if (idx != -1) pData[idx] = LEAVES;
                    else mMissingBlocks->emplace(std::make_pair(glm::ivec3(baseCoords.x + x, baseCoords.y - y, baseCoords.z + z), LEAVES));
                }
                else if (y == 3)
                {
                    if (idx != -1) pData[idx] = LEAVES;
                    else mMissingBlocks->emplace(std::make_pair(glm::ivec3(baseCoords.x + x, baseCoords.y - y, baseCoords.z + z), LEAVES));
                }


            }
                
    pData[getBlockIndex({ baseCoords.x, baseCoords.y - 7, baseCoords.z })] = LEAVES;
    for (int y = 1; y <= 6; y++)
        pData[getBlockIndex({ baseCoords.x, baseCoords.y - y, baseCoords.z })] = WOOD;

    transparentBlockCount += 57;


}

int ChunkData::getTopBlock(glm::ivec2 coords)
{
    for (int i = 0; i < 64; i++)
    {
        int idx = getBlockIndex({ coords.x, i, coords.y });
        if (idx == -1) return 63;
        if (pData[idx] != AIR && Chunk::billboards.count((BLOCKTYPE)pData[idx]) == 0) return i;

    }
    return 63;
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
        globalX = (float)chunkCoords.x * (float)CHUNKSIZE + (float)x;
        globalX *= scale;
        for (int z = 0; z <= CHUNKSIZE; z++)
        {
            globalZ = (float)chunkCoords.y * (float)CHUNKSIZE + (float)z;
            globalZ *= scale;
            float noise = grassNoise.normalizedOctave2D_01(globalX, globalZ, octaves, persistance);

            int idx = getBlockIndex({ x, getTopBlock({x,z}) - 1, z });

            if (idx == -1) continue;

            if (noise > 0.7f)
                pData[idx] = PURPLEFLOWER;
            else if (noise > 0.65f)
                pData[idx] = REDFLOWER;
            else if (noise > 0.55f)
                pData[idx] = TALLGRASS;

            if (noise > 0.55f) transparentBlockCount++;

        }
    }
}


std::unordered_map<glm::ivec3, BLOCKTYPE>* Chunk::getMissingBlocks()
{
    return mData.mMissingBlocks.get();
}

void Chunk::updateBlock(glm::ivec3 blockCoords, BLOCKTYPE bType)
{
    int idx = mData.getBlockIndex(blockCoords);
    if (idx != -1)
    {
        mData.getData()[idx] = bType;
        if (transparentBlocks.count(bType) != 0) mData.transparentBlockCount++;
    }
    
}

bool Chunk::getNeedRemeshStatus() const
{
    return needRemesh;
}

void Chunk::setNeedRemeshStatus(bool nr)
{
    needRemesh = nr;
}

const AABB* Chunk::getBoundingBox() const
{
    return boundingbox.get();
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

    unsigned maxThreads = std::max(1u, std::thread::hardware_concurrency());
    std::vector<std::future<void>> futures;

    for (auto& chunk : mChunks)
    {
        if (!chunk.second.getNeedRemeshStatus()) continue;

        futures.emplace_back(std::async(std::launch::async, [this, &chunk] {
            chunk.second.generateMesh();
            }));

        if (futures.size() >= maxThreads)
        {
            for (auto& f : futures)
                f.get();
            futures.clear();
        }
    }

    for (auto& f : futures)
        f.get();

    for (auto& chunk : mChunks)
    {
        if (!chunk.second.getNeedRemeshStatus()) continue;
        chunk.second.createGPUBuffers();
        chunk.second.setNeedRemeshStatus(false);
    }

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

    std::mutex chunkListMutex;
    unsigned maxThreads = std::max(1u, std::thread::hardware_concurrency());
    std::vector<std::future<void>> futures;

    for(int i = -renderDistance; i <= renderDistance; i++) 
        for (int j = -renderDistance; j <= renderDistance; j++)
        {
            glm::ivec2 newChunkCoords = { plrChunk.x + i, plrChunk.y + j };

            futures.emplace_back(std::async(std::launch::async, [this, newChunkCoords, &chunkListMutex]()
                {
                    {
                        std::lock_guard<std::mutex> lock(chunkListMutex);
                        if (mChunks.count(newChunkCoords) != 0)
                            return; 
                    }

                    Chunk chunk(newChunkCoords);

                    {
                        std::lock_guard<std::mutex> lock(chunkListMutex);
                        if (mChunks.count(newChunkCoords) == 0)
                            mChunks.try_emplace(newChunkCoords, std::move(chunk));
                    }


                }));

            if (futures.size() >= maxThreads)
            {
                for (auto& f : futures) 
                    f.get();
                futures.clear();
            }
                
        }

    for (auto& f : futures) {
        f.get();
    }

    fillMissingBlocks();

    for (auto& pair : mChunks) 
        if (!pair.second.hasGPUbuffers())
            pair.second.createGPUBuffers();

}

void Planet::Render(VkCommandBuffer commandBuffer, VkPipelineLayout& layout, Camera& cam)
{
    for (const auto& chunk : mChunks)
    {
        if (chunk.second.isPendingDeletion() || !cam.AABBIntersectsFrustum(chunk.second.getBoundingBox()))
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

void Planet::fillMissingBlocks()
{
    auto floorDiv = [](int a, int b) {
        int q = a / b;
        int r = a % b;
        if ((r != 0) && ((r > 0) != (b > 0))) --q;
        return q;
        };

    for (auto& chunkPair : mChunks)
    {
        std::unordered_map<glm::ivec3, BLOCKTYPE>* blockMap = chunkPair.second.getMissingBlocks();
        if (blockMap->empty()) continue;

        bool needsRemesh = false;
        for (auto& blockPair : *blockMap)
        {
            glm::ivec3 globalBlockPos = glm::ivec3(chunkPair.first.x * CHUNKSIZE, 0, chunkPair.first.y * CHUNKSIZE) + blockPair.first;

            glm::ivec2 targetChunkCoords = {
                floorDiv(globalBlockPos.x, CHUNKSIZE),
                floorDiv(globalBlockPos.z, CHUNKSIZE)
            };

            int localX = globalBlockPos.x - targetChunkCoords.x * CHUNKSIZE;
            int localZ = globalBlockPos.z - targetChunkCoords.y * CHUNKSIZE;

            glm::ivec3 localBlockPos = { localX, globalBlockPos.y, localZ };

            auto targetChunkIt = mChunks.find(targetChunkCoords);
            if (targetChunkIt != mChunks.end()) {
                targetChunkIt->second.updateBlock(localBlockPos, blockPair.second);
                needsRemesh = true;
            }
        }
        if (needsRemesh) chunkPair.second.setNeedRemeshStatus(true);

        blockMap->clear();
    }

   
}


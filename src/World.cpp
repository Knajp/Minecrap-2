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
        0,1,2, 2,3,0,
        4,5,6, 6,7,4,
        8,9,10, 10,11,8,
        12,13,14,14,15,12,
        16,17,18,18,19,16,
        20,21,22,22,23,20
    };

    int forwardIndices = 0;

    for(float x = 0; x < CHUNKSIZE; x++)
        for (float y = 0; y < CHUNKSIZE; y++)
        {
            BLOCKTYPE bType = (BLOCKTYPE)data[static_cast<int>(y * CHUNKSIZE + x)];
            // add entire cubes for now
            mMeshVertices.reserve(mMeshVertices.size() + 24);

            uint8_t frontTexture = getBlockTextureIndex(bType, BLOCKFACE::FRONT);
            uint8_t backTexture = getBlockTextureIndex(bType, BLOCKFACE::BACK);
            uint8_t leftTexture = getBlockTextureIndex(bType, BLOCKFACE::LEFT);
            uint8_t rightTexture = getBlockTextureIndex(bType, BLOCKFACE::RIGHT);
            uint8_t topTexture = getBlockTextureIndex(bType, BLOCKFACE::TOP);
            uint8_t bottomTexture = getBlockTextureIndex(bType, BLOCKFACE::BOTTOM);
            std::array<Vertex, 24> newVertices =
            {
                // FRONT FACE
                Vertex{glm::vec3(x,         0.0f, y),        glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2((frontTexture % 10) * 0.1f,        (frontTexture / 10) * 0.1f + 0.1f)},
                Vertex{glm::vec3(x,        -1.0f, y),        glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2((frontTexture % 10) * 0.1f,        (frontTexture / 10) * 0.1f)},
                Vertex{glm::vec3(x + 1.0f, -1.0f, y),        glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2((frontTexture % 10) * 0.1f + 0.1f, (frontTexture / 10) * 0.1f)},
                Vertex{glm::vec3(x + 1.0f,  0.0f, y),        glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2((frontTexture % 10) * 0.1f,        (frontTexture / 10) * 0.1f + 0.1f)},

                // BACK FACE
                Vertex{glm::vec3(x + 1.0f,  0.0f, y + 1.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2((backTexture % 10) * 0.1f,        (backTexture / 10) * 0.1f + 0.1f)},
                Vertex{glm::vec3(x + 1.0f, -1.0f, y + 1.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2((backTexture % 10) * 0.1f,        (backTexture / 10) * 0.1f)},
                Vertex{glm::vec3(x,        -1.0f, y + 1.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2((backTexture % 10) * 0.1f + 0.1f, (backTexture / 10) * 0.1f)},
                Vertex{glm::vec3(x,         0.0f, y + 1.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2((backTexture % 10) * 0.1f + 0.1f,        (backTexture / 10) * 0.1f + 0.1f)},

                // LEFT FACE
                Vertex{glm::vec3(x, -1.0f, y + 1.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2((leftTexture % 10) * 0.1f,        (leftTexture / 10) * 0.1f)},          
                Vertex{glm::vec3(x, -1.0f, y),        glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2((leftTexture % 10) * 0.1f + 0.1f, (leftTexture / 10) * 0.1f)},          
                Vertex{glm::vec3(x,  0.0f, y),        glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2((leftTexture % 10) * 0.1f + 0.1f, (leftTexture / 10) * 0.1f + 0.1f)},   
                Vertex{glm::vec3(x,  0.0f, y + 1.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2((leftTexture % 10) * 0.1f,        (leftTexture / 10) * 0.1f + 0.1f)},   


                // RIGHT FACE
                Vertex{glm::vec3(x + 1.0f,  0.0f, y),        glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2((rightTexture % 10) * 0.1f,        (rightTexture / 10) * 0.1f + 0.1f)},
                Vertex{glm::vec3(x + 1.0f, -1.0f, y),        glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2((rightTexture % 10) * 0.1f,        (rightTexture / 10) * 0.1f)},
                Vertex{glm::vec3(x + 1.0f, -1.0f, y + 1.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2((rightTexture % 10) * 0.1f + 0.1f, (rightTexture / 10) * 0.1f)},
                Vertex{glm::vec3(x + 1.0f,  0.0f, y + 1.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2((rightTexture % 10) * 0.1f + 0.1f,        (rightTexture / 10) * 0.1f + 0.1f)},

                // TOP FACE
                Vertex{glm::vec3(x,        -1.0f, y),        glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2((topTexture % 10) * 0.1f,        (topTexture / 10) * 0.1f + 0.1f)},
                Vertex{glm::vec3(x,        -1.0f, y + 1.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2((topTexture % 10) * 0.1f,        (topTexture / 10) * 0.1f)},
                Vertex{glm::vec3(x + 1.0f, -1.0f, y + 1.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2((topTexture % 10) * 0.1f + 0.1f, (topTexture / 10) * 0.1f)},
                Vertex{glm::vec3(x + 1.0f, -1.0f, y),        glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2((topTexture % 10) * 0.1f + 0.1f,        (topTexture / 10) * 0.1f + 0.1f)},

                // BOTTOM FACE
                Vertex{glm::vec3(x,        0.0f, y),         glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2((bottomTexture % 10) * 0.1f,        (bottomTexture / 10) * 0.1f + 0.1f)},
                Vertex{glm::vec3(x,        0.0f, y + 1.0f),  glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2((bottomTexture % 10) * 0.1f,        (bottomTexture / 10) * 0.1f)},
                Vertex{glm::vec3(x + 1.0f, 0.0f, y + 1.0f),  glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2((bottomTexture % 10) * 0.1f + 0.1f, (bottomTexture / 10) * 0.1f)},
                Vertex{glm::vec3(x + 1.0f, 0.0f, y),         glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2((bottomTexture % 10) * 0.1f + 0.1f,        (bottomTexture / 10) * 0.1f + 0.1f)},
            };

            
            for (const auto& vertex : newVertices)
                mMeshVertices.push_back(vertex);

            for (const auto& index : Indices)
                mMeshIndices.push_back(index + forwardIndices);

            forwardIndices += 24;

            
        }
    std::cout << mMeshVertices.size() / 24 << "\n";
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
    pData = new uint8_t[CHUNKSIZE * CHUNKSIZE];
    allocateChunkData();
}

ChunkData::~ChunkData()
{
    delete[] pData;
}

bool ChunkData::allocateChunkData()
{
    for (size_t i = 0; i < CHUNKSIZE; i++)
        for (size_t j = 0; j < CHUNKSIZE; j++)
            pData[j * CHUNKSIZE + i] = static_cast<uint8_t>(BLOCKTYPE::GRASS);

    return true;
}

uint8_t* ChunkData::getData()
{
    return pData;
}

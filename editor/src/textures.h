#pragma once

#include <cstdint>

struct Texture
{
    void* id;
    int width;
    int height;
};

bool loadTextureFromFile(const char* filename, Texture& outTexture);

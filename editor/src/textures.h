#pragma once

#include <cstdint>
#include <string>

struct Texture
{
    void* id;
    int width;
    int height;
    std::string filename;
};

bool loadTextureFromFile(const char* filename, Texture& outTexture);

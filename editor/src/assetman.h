#pragma once

#include <cstdint>
#include <string>
#include <memory>
#include <filesystem>
#include <unordered_map>
#include <map>

struct Texture
{
    void* id;
    int width;
    int height;
};

class AssetMan
{
public:
    std::shared_ptr<Texture> textureFromAssetPath(const std::filesystem::path& path);
    std::shared_ptr<Texture> textureFromAbsPath(const std::filesystem::path& absPath);
    std::filesystem::path assetPathFromTexture(const std::shared_ptr<Texture>& texture);

private:
    std::unordered_map<std::shared_ptr<Texture>, std::filesystem::path> m_texPathMap;
    // Using map for now because the compiler says path doesn't have a hash function,
    // when it seems like according to the docs it does...
    std::map<std::filesystem::path, std::shared_ptr<Texture>> m_pathTexMap;
};

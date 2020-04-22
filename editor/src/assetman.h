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
    std::filesystem::path filePath;
    std::string shortName;
};

class AssetMan
{
public:
    std::shared_ptr<Texture> loadTexture(const std::filesystem::path& absPath, const std::string& shortName = "");
    std::shared_ptr<Texture> findTextureByShortName(const std::string& shortName);
    const std::vector<std::shared_ptr<Texture>>& getTextures();


    static std::filesystem::path getAssetPathRoot();
    static std::string getAssetPathStr(const std::filesystem::path& path);

private:
    std::vector<std::shared_ptr<Texture>> m_textures;
};

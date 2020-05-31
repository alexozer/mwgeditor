#pragma once

#include <cstdint>
#include <string>
#include <memory>
#include <filesystem>
#include <vector>

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
    // Init function b/c assetman is allocated statically
    void init();

    std::shared_ptr<Texture> loadTexture(const std::filesystem::path& absPath, const std::string& shortName = "");
    std::shared_ptr<Texture> findTextureByShortName(const std::string& shortName);
    const std::vector<std::shared_ptr<Texture>>& getTextures();


    std::filesystem::path getAssetPathRoot();
    std::string getAssetPathStr(const std::filesystem::path& path);

private:
    std::filesystem::path m_assetPathRoot;
    std::vector<std::shared_ptr<Texture>> m_textures;
};

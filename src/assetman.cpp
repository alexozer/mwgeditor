#include "assetman.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

// About Desktop OpenGL function loaders:
//  Modern desktop OpenGL doesn't have a standard portable header file to load OpenGL function pointers.
//  Helper libraries are often used for this purpose! Here we are supporting a few common ones (gl3w, glew, glad).
//  You may use another loader/header of your choice (glext, glLoadGen, etc.), or chose to manually implement your own.
#if defined(IMGUI_IMPL_OPENGL_LOADER_GL3W)
#include <GL/gl3w.h>            // Initialize with gl3wInit()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)
#include <GL/glew.h>            // Initialize with glewInit()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)
#include <glad/glad.h>          // Initialize with gladLoadGL()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLBINDING2)
#define GLFW_INCLUDE_NONE       // GLFW including OpenGL headers causes ambiguity or multiple definition errors.
#include <glbinding/Binding.h>  // Initialize with glbinding::Binding::initialize()
#include <glbinding/gl/gl.h>
using namespace gl;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLBINDING3)
#define GLFW_INCLUDE_NONE       // GLFW including OpenGL headers causes ambiguity or multiple definition errors.
#include <glbinding/glbinding.h>// Initialize with glbinding::initialize()
#include <glbinding/gl/gl.h>
using namespace gl;
#else
#include IMGUI_IMPL_OPENGL_LOADER_CUSTOM
#endif

// Include glfw3.h after our OpenGL definitions
#include <GLFW/glfw3.h>

// [Win32] Our example includes a copy of glfw3.lib pre-compiled with VS2010 to maximize ease of testing and compatibility with old VS compilers.
// To link with VS2010-era libraries, VS2015+ requires linking with legacy_stdio_definitions.lib, which we do using this pragma.
// Your own project should not be affected, as you are likely to link with a newer binary of GLFW that is adequate for your version of Visual Studio.
#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

#include <filesystem>
#include <algorithm>

namespace fs = std::filesystem;

void AssetMan::init()
{
    fs::path currPath = fs::current_path();

    while (!fs::exists(currPath / ".git"))
    {
        currPath = currPath.parent_path();
        if (currPath.root_path() == currPath)
        {
            throw std::runtime_error("Could not locate Milky Way Gourmet git repository root dir");
        }
    }

    m_assetPathRoot = currPath / "assets";
}

// Simple helper function to load an image into a OpenGL texture with common settings
static std::shared_ptr<Texture> loadTextureFromFile(const char* filename)
{
    // Load from file
    int image_width = 0;
    int image_height = 0;
    unsigned char* image_data = stbi_load(filename, &image_width, &image_height, NULL, 4);
    if (image_data == NULL) return nullptr;

    // Create a OpenGL texture identifier
    GLuint image_texture;
    glGenTextures(1, &image_texture);
    glBindTexture(GL_TEXTURE_2D, image_texture);

    // Setup filtering parameters for display
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Upload pixels into texture
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image_width, image_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
    stbi_image_free(image_data);

    auto outTexture = std::make_shared<Texture>();

    outTexture->id = reinterpret_cast<void *>(image_texture);
    outTexture->width = image_width;
    outTexture->height = image_height;

    return outTexture;
}

fs::path AssetMan::getAssetPathRoot()
{
    return m_assetPathRoot;
}

std::shared_ptr<Texture> AssetMan::loadTexture(const std::filesystem::path &absPath, const std::string& shortName)
{
    auto it = std::find_if(m_textures.begin(), m_textures.end(), [&](auto tex) {
        return tex->filePath == absPath;
    });
    if (it == m_textures.end())
    {
        auto tex = loadTextureFromFile(absPath.u8string().c_str());
        if (!tex)
        {
            throw std::runtime_error("Could not load texture file: " + absPath.string());
        }

        tex->filePath = absPath;
        tex->shortName = shortName.empty() ? absPath.filename().u8string() : shortName;
        m_textures.push_back(tex);

        return tex;
    }

    return *it;
}

std::shared_ptr<Texture> AssetMan::findTextureByShortName(const std::string& shortName)
{
    auto it = std::find_if(m_textures.begin(), m_textures.end(), [&](auto tex) {
        return tex->shortName == shortName;
    });

    return it != m_textures.end() ? *it : nullptr;
}

const std::vector<std::shared_ptr<Texture>>& AssetMan::getTextures()
{
    return m_textures;
}

std::string AssetMan::getAssetPathStr(const fs::path &path)
{
    fs::path relative = path.lexically_relative(getAssetPathRoot());
    return relative.generic_u8string();
}

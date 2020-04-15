#include "assetman.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <SDL.h>

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

#include <filesystem>
#include <iostream>

using std::filesystem::path;

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

static path assetPathRoot()
{
    return (std::filesystem::current_path().parent_path() / "assets").lexically_normal();
}

static path absToAssetPath(const path& absPath)
{
    return absPath.lexically_relative(assetPathRoot());
}

static path assetToAbsPath(const path& assetPath)
{
    return assetPathRoot() / assetPath;
}

std::shared_ptr<Texture> AssetMan::textureFromAssetPath(const std::filesystem::path &path)
{
    return textureFromAbsPath(assetToAbsPath(path));
}

std::shared_ptr<Texture> AssetMan::textureFromAbsPath(const std::filesystem::path &absPath)
{
    auto it = m_pathTexMap.find(absPath);
    if (it == m_pathTexMap.end())
    {
        auto tex = loadTextureFromFile(absPath.c_str());
        if (!tex)
        {
            throw std::runtime_error("Could not load texture file: " + absPath.string());
        }
        m_pathTexMap[absPath] = tex;
        m_texPathMap[tex] = absPath;

        return tex;
    }

    return it->second;
}

std::filesystem::path AssetMan::assetPathFromTexture(const std::shared_ptr<Texture> &texture)
{
    return m_texPathMap.at(texture);
}

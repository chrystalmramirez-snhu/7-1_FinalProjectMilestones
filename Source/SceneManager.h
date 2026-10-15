///////////////////////////////////////////////////////////////////////////////
// scenemanager.h
// Manage the preparing and rendering of 3D scenes
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "ShaderManager.h"
#include "ShapeMeshes.h"
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <string>
#include <vector>
#include <cstdint>

// Texture information
struct TEXTURE_INFO
{
    std::string tag;
    GLuint ID = 0;
};

// Object material information
struct OBJECT_MATERIAL
{
    glm::vec3 diffuseColor;
    glm::vec3 specularColor;
    float shininess;
    std::string tag;
};

class SceneManager
{
public:
    // Constructor and destructor
    SceneManager(ShaderManager* pShaderManager);
    ~SceneManager();


    // Scene functions
    void LoadSceneTextures();
    void PrepareScene();
    void RenderScene();

  

private:
    // Manager pointers
    ShaderManager* m_pShaderManager;
    ShapeMeshes* m_basicMeshes;


    // Texture information
    int m_loadedTextures;
    TEXTURE_INFO m_textureIDs[16];

    // Material information
    std::vector<OBJECT_MATERIAL> m_objectMaterials;

    // Transform and shader functions
    void SetTransformations(
        glm::vec3 scaleXYZ,
        float XrotationDegrees,
        float YrotationDegrees,
        float ZrotationDegrees,
        glm::vec3 positionXYZ);

    void SetShaderColor(
        float redColorValue,
        float greenColorValue,
        float blueColorValue,
        float alphaValue);

    // Texture functions
    bool CreateGLTexture(const char* filename, std::string tag);
    void BindGLTextures();
    void DestroyGLTextures();
    int FindTextureID(std::string tag);
    int FindTextureSlot(std::string tag);
    void SetShaderTexture(std::string textureTag);
    void SetShaderTextures(std::string textureTag1, std::string textureTag2);
    void SetTextureUVScale(float u, float v);

    // Material and lighting functions
    bool FindMaterial(std::string tag, OBJECT_MATERIAL& material);
    void DefineObjectMaterials();
    void SetShaderMaterial(std::string materialTag);
    void SetupSceneLights();
    

};

///////////////////////////////////////////////////////////////////////////////
// scenemanager.cpp
// ============
// manage the preparing and rendering of 3D scenes - textures, materials, lighting
//
//  AUTHOR: Chrystal Ramirez 
//	Created for CS-330-Computational Graphics and Visualization,
///////////////////////////////////////////////////////////////////////////////

#include "SceneManager.h"

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#endif

#include <glm/gtx/transform.hpp>
#include <iostream>

// declaration of global variables
namespace
{
	const char* g_ModelName = "model";
	const char* g_ColorValueName = "objectColor";
	const char* g_TextureValueName = "objectTexture";
	const char* g_UseTextureName = "bUseTexture";
	const char* g_UseLightingName = "bUseLighting";
}

/***********************************************************
 *  SceneManager()
 *
 *  The constructor for the class
 ***********************************************************/
SceneManager::SceneManager(ShaderManager* pShaderManager)
{
	m_pShaderManager = pShaderManager;
	m_basicMeshes = new ShapeMeshes();

	// initialize the texture collection
	for (int i = 0; i < 16; i++)
	{
		m_textureIDs[i].tag = "";
		m_textureIDs[i].ID = 0;
	}
	m_loadedTextures = 0;

}

/***********************************************************
 *  ~SceneManager()
 *
 *  The destructor for the class
 ***********************************************************/
SceneManager::~SceneManager()
{
	m_pShaderManager = NULL;
	delete m_basicMeshes;
	m_basicMeshes = NULL;
	// destroy the created OpenGL textures
	DestroyGLTextures();

	// clear the collection of defined materials
	m_objectMaterials.clear();
}
/***********************************************************
 *  FindMaterial()
 *
 *  This method is used for getting a material from the previously
 *  defined materials list that is associated with the passed in tag.
 ***********************************************************/

bool SceneManager::FindMaterial(
	std::string tag,
	OBJECT_MATERIAL& material)
{
	for (size_t index = 0;
		index < m_objectMaterials.size();
		index++)
	{
		if (m_objectMaterials[index].tag == tag)
		{
			material = m_objectMaterials[index];
			return true;
		}
	}

	return false;
}


bool SceneManager::CreateGLTexture(const char* filename, std::string tag)
{
	// Prevent the texture collection from exceeding its 16 slots.
	if (m_loadedTextures >= 16)
	{
		std::cout << "Texture limit reached. Cannot load: "
			<< filename << std::endl;
		return false;
	}

	int width = 0;
	int height = 0;
	int colorChannels = 0;

	// Flip the image vertically so it maps correctly in OpenGL.
	stbi_set_flip_vertically_on_load(true);

	unsigned char* image = stbi_load(
		filename,
		&width,
		&height,
		&colorChannels,
		0
	);

	if (image == nullptr)
	{
		std::cout << "Could not load image: "
			<< filename << std::endl;
		return false;
	}

	std::cout << "Successfully loaded image: " << filename
		<< ", width: " << width
		<< ", height: " << height
		<< ", channels: " << colorChannels
		<< std::endl;

	// Select the correct OpenGL image format.
	GLenum imageFormat = GL_RGB;
	GLint internalFormat = GL_RGB8;

	if (colorChannels == 4)
	{
		imageFormat = GL_RGBA;
		internalFormat = GL_RGBA8;
	}
	else if (colorChannels != 3)
	{
		std::cout << "Unsupported image format: "
			<< colorChannels << " channels."
			<< std::endl;

		stbi_image_free(image);
		return false;
	}

	GLuint textureID = 0;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);

	// Repeat textures across the surface.
	glTexParameteri(
		GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(
		GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// Control texture filtering.
	glTexParameteri(
		GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(
		GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Send image data to the graphics card.
	glTexImage2D(
		GL_TEXTURE_2D,
		0,
		internalFormat,
		width,
		height,
		0,
		imageFormat,
		GL_UNSIGNED_BYTE,
		image
	);

	// Generate mipmaps for the texture.
	glGenerateMipmap(GL_TEXTURE_2D);

	// Release the image data loaded into CPU memory.
	stbi_image_free(image);

	// Unbind the texture.
	glBindTexture(GL_TEXTURE_2D, 0);

	// Save the texture ID and its tag.
	m_textureIDs[m_loadedTextures].ID = textureID;
	m_textureIDs[m_loadedTextures].tag = tag;
	m_loadedTextures++;

	return true;
}
/***********************************************************
 *  BindGLTextures()
 *
 *  This method is used for binding the loaded textures to
 *  OpenGL texture memory slots.  There are up to 16 slots.
 ***********************************************************/
void SceneManager::BindGLTextures()
{
	for (int i = 0; i < m_loadedTextures; i++)
	{
		// bind textures on corresponding texture units
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, m_textureIDs[i].ID);
	}
}

/***********************************************************
 *  DestroyGLTextures()
 *
 *  This method is used for freeing the memory in all the
 *  used texture memory slots.
 ***********************************************************/
void SceneManager::DestroyGLTextures()
{
	for (int i = 0; i < m_loadedTextures; i++)
	{
		glDeleteTextures(1, &m_textureIDs[i].ID);
	}
}

/***********************************************************
 *  FindTextureID()
 *
 *  This method is used for getting an ID for the previously
 *  loaded texture bitmap associated with the passed in tag.
 ***********************************************************/
int SceneManager::FindTextureID(std::string tag)
{
	int textureID = -1;
	int index = 0;
	bool bFound = false;

	while ((index < m_loadedTextures) && (bFound == false))
	{
		if (m_textureIDs[index].tag.compare(tag) == 0)
		{
			textureID = m_textureIDs[index].ID;
			bFound = true;
		}
		else
			index++;
	}

	return(textureID);
}

/***********************************************************
 *  FindTextureSlot()
 *
 *  This method is used for getting a slot index for the previously
 *  loaded texture bitmap associated with the passed in tag.
 ***********************************************************/
int SceneManager::FindTextureSlot(std::string tag)
{
	int textureSlot = -1;
	int index = 0;
	bool bFound = false;

	while ((index < m_loadedTextures) && (bFound == false))
	{
		if (m_textureIDs[index].tag.compare(tag) == 0)
		{
			textureSlot = index;
			bFound = true;
		}
		else
			index++;
	}

	return(textureSlot);
}


/***********************************************************
 *  SetTransformations()
 *
 *  This method is used for setting the transform buffer
 *  using the passed in transformation values.
 ***********************************************************/
void SceneManager::SetTransformations(
	glm::vec3 scaleXYZ,
	float XrotationDegrees,
	float YrotationDegrees,
	float ZrotationDegrees,
	glm::vec3 positionXYZ)
{
	// variables for this method
	glm::mat4 modelView;
	glm::mat4 scale;
	glm::mat4 rotationX;
	glm::mat4 rotationY;
	glm::mat4 rotationZ;
	glm::mat4 translation;

	// set the scale value in the transform buffer
	scale = glm::scale(scaleXYZ);
	// set the rotation values in the transform buffer
	rotationX = glm::rotate(glm::radians(XrotationDegrees), glm::vec3(1.0f, 0.0f, 0.0f));
	rotationY = glm::rotate(glm::radians(YrotationDegrees), glm::vec3(0.0f, 1.0f, 0.0f));
	rotationZ = glm::rotate(glm::radians(ZrotationDegrees), glm::vec3(0.0f, 0.0f, 1.0f));
	// set the translation value in the transform buffer
	translation = glm::translate(positionXYZ);

	// matrix math for calculating the final model matrix
	modelView = translation * rotationZ * rotationY * rotationX * scale;

	if (NULL != m_pShaderManager)
	{
		// pass the model matrix into the shader
		m_pShaderManager->setMat4Value(g_ModelName, modelView);
	}
}
/***********************************************************
 *  SetShaderMaterial()
 *
 *  This method is used for passing the material values
 *  into the shader.
 ***********************************************************/
void SceneManager::SetShaderMaterial(std::string materialTag)
{
	if (m_objectMaterials.size() > 0)
	{
		OBJECT_MATERIAL material;
		bool bReturn = FindMaterial(materialTag, material);

		if (bReturn == true)
		{
			m_pShaderManager->setVec3Value(
				"material.diffuseColor", material.diffuseColor);

			m_pShaderManager->setVec3Value(
				"material.specularColor", material.specularColor);

			m_pShaderManager->setFloatValue(
				"material.shininess", material.shininess);
		}
	}
}
/***********************************************************
 *  SetShaderColor()
 *
 *  This method is used for setting the passed in color
 *  into the shader for the next draw command
 ***********************************************************/
void SceneManager::SetShaderColor(
	float redColorValue,
	float greenColorValue,
	float blueColorValue,
	float alphaValue)
{
	// variables for this method
	glm::vec4 currentColor;

	currentColor.r = redColorValue;
	currentColor.g = greenColorValue;
	currentColor.b = blueColorValue;
	currentColor.a = alphaValue;

	if (NULL != m_pShaderManager)
	{
		// Turn off normal texture and texture overlay
		m_pShaderManager->setIntValue(g_UseTextureName, false);
		m_pShaderManager->setIntValue("bUseTextureOverlay", false);

		// pass the color values into the shader
		m_pShaderManager->setVec4Value(g_ColorValueName, currentColor);
	}
}
/***********************************************************
*  SetShaderTexture()
*
*  This method is used for setting the texture data
*  associated with the passed in ID into the shader.
***********************************************************/

void SceneManager::SetShaderTexture(std::string textureTag)
{
	if (m_pShaderManager == nullptr)
	{
		return;
	}

	int textureSlot = FindTextureSlot(textureTag);

	if (textureSlot < 0)
	{
		std::cout << "Texture not found: "
			<< textureTag << std::endl;
		return;
	}

	// Use the selected texture without an overlay.
	m_pShaderManager->setIntValue(g_UseTextureName, true);
	m_pShaderManager->setIntValue("bUseTextureOverlay", false);

	m_pShaderManager->setSampler2DValue(
		g_TextureValueName,
		textureSlot
	);
}


void SceneManager::SetShaderTextures(
	std::string textureTag1,
	std::string textureTag2)
{
	if (m_pShaderManager == nullptr)
	{
		return;
	}

	int textureSlot1 = FindTextureSlot(textureTag1);
	int textureSlot2 = FindTextureSlot(textureTag2);

	if (textureSlot1 < 0 || textureSlot2 < 0)
	{
		std::cout << "Could not find both textures: "
			<< textureTag1 << ", "
			<< textureTag2 << std::endl;
		return;
	}

	m_pShaderManager->setSampler2DValue(
		g_TextureValueName,
		textureSlot1
	);

	m_pShaderManager->setSampler2DValue(
		"overlayTexture",
		textureSlot2
	);

	m_pShaderManager->setIntValue(g_UseTextureName, true);
	m_pShaderManager->setIntValue("bUseTextureOverlay", true);
}

/***********************************************************
*  SetTextureUVScale()
*
*  This method is used for setting the texture UV scale
*  values into the shader.
***********************************************************/
void SceneManager::SetTextureUVScale(float u, float v)
{
	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setVec2Value("UVscale", glm::vec2(u, v));
	}
}

/***********************************************************
  *  DefineObjectMaterials()
  *
  *  This method is used for configuring the various material
  *  settings for all of the objects within the 3D scene.
  ***********************************************************/
void SceneManager::DefineObjectMaterials()
{
	/*** STUDENTS - add the code BELOW for defining object materials. ***/
	/*** There is no limit to the number of object materials that can ***/
	/*** be defined. Refer to the code in the OpenGL Sample for help  ***/

	OBJECT_MATERIAL plasticMaterial;
	plasticMaterial.diffuseColor = glm::vec3(0.8f, 0.4f, 0.8f);
	plasticMaterial.specularColor = glm::vec3(0.2f, 0.2f, 0.2f);
	plasticMaterial.shininess = 2.0;
	plasticMaterial.tag = "plastic";

	m_objectMaterials.push_back(plasticMaterial);

	OBJECT_MATERIAL woodMaterial;
	woodMaterial.diffuseColor = glm::vec3(0.6f, 0.5f, 0.2f);
	woodMaterial.specularColor = glm::vec3(0.1f, 0.2f, 0.2f);
	woodMaterial.shininess = 1.0;
	woodMaterial.tag = "wood";
	m_objectMaterials.push_back(woodMaterial);

	OBJECT_MATERIAL metalMaterial;
	metalMaterial.diffuseColor = glm::vec3(0.3f, 0.3f, 0.2f);
	metalMaterial.specularColor = glm::vec3(0.7f, 0.7f, 0.8f);
	metalMaterial.shininess = 8.0;
	metalMaterial.tag = "metal";
	m_objectMaterials.push_back(metalMaterial);

	OBJECT_MATERIAL glassMaterial;
	glassMaterial.diffuseColor = glm::vec3(0.3f, 0.3f, 0.2f);
	glassMaterial.specularColor = glm::vec3(0.9f, 0.9f, 0.8f);
	glassMaterial.shininess = 10.0;
	glassMaterial.tag = "glass";
	m_objectMaterials.push_back(glassMaterial);

	OBJECT_MATERIAL tileMaterial;
	tileMaterial.diffuseColor = glm::vec3(0.5f, 0.5f, 0.5f);
	tileMaterial.specularColor = glm::vec3(0.7f, 0.7f, 0.7f);
	tileMaterial.shininess = 6.0;
	tileMaterial.tag = "tile";
	m_objectMaterials.push_back(tileMaterial);

	OBJECT_MATERIAL stoneMaterial;
	stoneMaterial.diffuseColor = glm::vec3(0.5f, 0.5f, 0.5f);
	stoneMaterial.specularColor = glm::vec3(0.73f, 0.3f, 0.3f);
	stoneMaterial.shininess = 6.0;
	stoneMaterial.tag = "stone";
	m_objectMaterials.push_back(stoneMaterial);

	OBJECT_MATERIAL plantMaterial;
	plantMaterial.diffuseColor = glm::vec3(0.15f, 0.4f, 0.1f);
	plantMaterial.specularColor = glm::vec3(0.1f, 0.1f, 0.1f);
	plantMaterial.shininess = 4.0f;
	plantMaterial.tag = "plant";
	m_objectMaterials.push_back(plantMaterial);

	OBJECT_MATERIAL wallMaterial;
	wallMaterial.diffuseColor = glm::vec3(.0f, 0.0f, 0.0f);
	wallMaterial.specularColor = glm::vec3(-1.0f, 0.0f, 0.0f);
	wallMaterial.shininess = .0f;
	wallMaterial.tag = "wall";
	m_objectMaterials.push_back(wallMaterial);

}

/***********************************************************
 *  SetupSceneLights()
 *
 *  This method is called to add and configure the light
 *  sources for the 3D scene.  There are up to 4 light sources.
 ***********************************************************/
void SceneManager::SetupSceneLights()
{
	// this line of code is NEEDED for telling the shaders to render 
	// the 3D scene with custom lighting, if no light sources have
	// been added then the display window will be black - to use the 
	// default OpenGL lighting then comment out the following line

	// =========================================================
	// GLOBAL LIGHTING SETTING
	// Turns lighting calculations on for the entire 3D scene.
	// =========================================================
	m_pShaderManager->setBoolValue(g_UseLightingName, true);

	// =========================================================
	// DIRECTIONAL LIGHT
	// Simulates light coming from a consistent direction.
	// Its direction is set here, NOT its screen position.
	// =========================================================
	// Directional light: soft, even illumination
	m_pShaderManager->setVec3Value("directionalLight.direction", 0.5f, .0f, -0.5f);
	// Controls the light that reaches surfaces indirectly
	m_pShaderManager->setVec3Value("directionalLight.ambient", 0.3f, 0.3f, 0.3f);
	// Controls direct illumination on surfaces
	m_pShaderManager->setVec3Value("directionalLight.diffuse", .5f, .5f, .5f);
	// Controls shiny highlights from this light
	m_pShaderManager->setVec3Value("directionalLight.specular", 0.10f, 0.10f, 0.10f);
	// Enables this light source
	m_pShaderManager->setBoolValue("directionalLight.bActive", true);

	// =========================================================
   // SPOTLIGHT
   // Creates a focused beam of light, like a flashlight.
   // Its actual position and direction are set elsewhere.
   // =========================================================
	//defines a spotlight source
	m_pShaderManager->setVec3Value("spotLight.ambient", 0.0f, 0.0f, 0.0f);
	m_pShaderManager->setVec3Value("spotLight.diffuse", 1.0f, 1.0f, 1.0f);
	m_pShaderManager->setVec3Value("spotLight.specular", .0f, .0f, .0f);
	// Controls how the spotlight's intensity decreases with distance
	m_pShaderManager->setFloatValue("spotLight.constant", 3.0f);
	m_pShaderManager->setFloatValue("spotLight.linear", 0.09f);
	m_pShaderManager->setFloatValue("spotLight.quadratic", 0.032f);
	// Controls the inner and outer edges of the beam
	m_pShaderManager->setFloatValue("spotLight.cutOff", glm::cos(glm::radians(22.5f)));
	m_pShaderManager->setFloatValue("spotLight.outerCutOff",
		glm::cos(glm::radians(28.0f)));
	m_pShaderManager->setBoolValue("spotLight.bActive", true);


	// Point Light 1 - Left side
	m_pShaderManager->setVec3Value("pointLights[0].position", -4.0f, 4.0f, 4.0f);
	m_pShaderManager->setVec3Value("pointLights[0].ambient", 0.02f, 0.02f, 0.02f);
	m_pShaderManager->setVec3Value("pointLights[0].diffuse", 0.30f, 0.30f, 0.30f);
	m_pShaderManager->setVec3Value("pointLights[0].specular", 0.05f, 0.05f, 0.05f);
	m_pShaderManager->setBoolValue("pointLights[0].bActive", true);


	// Point Light 2 - Right side
	m_pShaderManager->setVec3Value("pointLights[1].position", 4.0f, 4.0f, 4.0f);
	m_pShaderManager->setVec3Value("pointLights[1].ambient", 0.02f, 0.02f, 0.02f);
	m_pShaderManager->setVec3Value("pointLights[1].diffuse", 0.30f, 0.30f, 0.30f);
	m_pShaderManager->setVec3Value("pointLights[1].specular", 0.05f, 0.05f, 0.05f);
	m_pShaderManager->setBoolValue("pointLights[1].bActive", true);


	// Point Light 3 - Upper center
	m_pShaderManager->setVec3Value("pointLights[2].position", 0.0f, 1.0f, 2.0f);
	m_pShaderManager->setVec3Value("pointLights[2].ambient", 0.02f, 0.02f, 0.02f);
	m_pShaderManager->setVec3Value("pointLights[2].diffuse", 0.35f, 0.35f, 0.35f);
	m_pShaderManager->setVec3Value("pointLights[2].specular", 0.05f, 0.05f, 0.05f);
	m_pShaderManager->setBoolValue("pointLights[2].bActive", true);


	// Point Light 4 - Back/side fill
	m_pShaderManager->setVec3Value("pointLights[3].position", -3.0f, 6.0f, 6.0f);
	m_pShaderManager->setVec3Value("pointLights[3].ambient", 0.01f, 0.01f, 0.01f);
	m_pShaderManager->setVec3Value("pointLights[3].diffuse", 0.15f, 0.15f, 0.15f);
	m_pShaderManager->setVec3Value("pointLights[3].specular", 0.05f, 0.05f, 0.05f);
	m_pShaderManager->setBoolValue("pointLights[3].bActive", true);

}

/***********************************************************
*  LoadSceneTextures()
*
*  This method is used for preparing the 3D scene by loading
*  the shapes, textures in memory to support the 3D scene
*  rendering
***********************************************************/
void SceneManager::LoadSceneTextures()
{
	bool bReturn = false;

	// Load wood texture
	bReturn = CreateGLTexture("wood.jpg", "wood");

	if (!bReturn)
	{
		std::cout << "Failed to load wood.jpg" << std::endl;
	}

	// Load marble texture
	bReturn = CreateGLTexture("marble.jpg", "marble");

	if (!bReturn)
	{
		std::cout << "Failed to load marble.jpg" << std::endl;
	}
	

	// Load rock texture
	bReturn = CreateGLTexture("rock.jpg", "rock");

	if (!bReturn)
	{
		std::cout << "Failed to load rock.jpg" << std::endl;
	}

	//Load leaf texture
	bReturn = CreateGLTexture("leaf1.jpg", "leaf1");


	if (!bReturn)
	{
		std::cout << "Failed to load leaf1.jpg" << std::endl;
	}

	// Load Leaf Texxture overlay
	bReturn = CreateGLTexture("gradient_green.jpg", "ggreen");

	if (!bReturn)
	{
		std::cout << "Failed to load gradient_green.jpg" << std::endl;
	}

	// Load bottom plane texture
	bReturn = CreateGLTexture("paper.jpg", "paper");

	if (!bReturn)
	{
		std::cout << "Failed to load paper.jpg" << std::endl;
	}

	// Load a dark gray texture for the picture frame.
	bReturn = CreateGLTexture("stone_light.jpg", "frame");

	if (!bReturn)
	{
		std::cout << "Failed to stone_light.jpg" << std::endl;
	}
	// Load a  black texture for the soil.
	bReturn = CreateGLTexture("soil.jpg", "soil");

	if (!bReturn)
	{
		std::cout << "Failed to soil.jpg" << std::endl;
	}

	// Bind all loaded textures
	BindGLTextures();
}
/***********************************************************
 *  PrepareScene()
 *
 *  This method is used for preparing the 3D scene by loading
 *  the shapes, textures in memory to support the 3D scene
 *  rendering
 ***********************************************************/
void SceneManager::PrepareScene()
{
	LoadSceneTextures();

	m_basicMeshes->LoadPlaneMesh();
	m_basicMeshes->LoadConeMesh();
	m_basicMeshes->LoadBoxMesh();
	m_basicMeshes->LoadSphereMesh();
	m_basicMeshes->LoadCylinderMesh();
	m_basicMeshes->LoadTorusMesh();

	DefineObjectMaterials();
	SetupSceneLights();
}

/***********************************************************
 *  RenderScene()
 *
 *  This method is used for rendering the 3D scene by
 *  transforming and drawing the basic 3D shapes
 ***********************************************************/
void SceneManager::RenderScene()
{
	// declare the variables for the transformations
	glm::vec3 scaleXYZ;
	float XrotationDegrees = 0.0f;
	float YrotationDegrees = 0.0f;
	float ZrotationDegrees = 0.0f;
	glm::vec3 positionXYZ;

	//helper lambda to eliminate repetion of textured wood box
	auto DrawTexturedBox = [&](glm::vec3 scale,
		glm::vec3 position,
		std::string textureTag)
		{
			SetTransformations(
				scale,
				0.0f,
				0.0f,
				0.0f,
				position
			);

			SetShaderTexture(textureTag);
			SetTextureUVScale(1.0f, 1.0f);

			m_basicMeshes->DrawBoxMesh();
		};

	/*** Set needed transformations before drawing the basic mesh.  ***/
	/*** This same ordering of code should be used for transforming ***/
	/*** and drawing all the basic 3D shapes.						***/

	/******************************************************************/
	// Bottom Plane - Chrystal Ramirez
	/******************************************************************/

		// set the XYZ scale for the mesh
	scaleXYZ = glm::vec3(20.0f, 1.0f, 10.0f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	//Check this later to see if final needs this position 
	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(0.0f, 1.45f, 10.0f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	// set the color values into the shader- white 
	//SetShaderColor(1.0f, 1.0f, 1.0f, 1.0f);
	SetShaderTexture("paper");
	SetShaderMaterial("wood");
	SetTextureUVScale(1.0f, 1.0f);
	// draw the mesh with transformation values
	m_basicMeshes->DrawPlaneMesh();


	/******************************************************************/
	// Back Plane textured wall
	/******************************************************************/

	// set the XYZ scale for the mesh
	scaleXYZ = glm::vec3(20.0f, 1.0f, 10.0f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 90.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(0.0f, 11.0f, 0.0f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	// set the color values into the shader
	//SetShaderColor(0.7f, 0.7f, 0.7f, 1.0f);

	//Set Color
	SetShaderTexture("paper");
	SetShaderMaterial("stone");
	SetTextureUVScale(1.0f, 1.0f);

	// draw the mesh with transformation values
	m_basicMeshes->DrawPlaneMesh();



	///******************************************************************/
	//Left Object - Wooden Stand Elongated Boxes Center Front Vertical
	/******************************************************************/
	// Wooden stand - center front support
	DrawTexturedBox(
		glm::vec3(0.45f, 3.0f, 0.45f),
		glm::vec3(-5.75f, 3.0f, 1.0f),
		"wood"
	);
	SetShaderMaterial("wood");
	///******************************************************************/
	//Left Object - Wooden Stand Elongated Boxes Left  Vertical
	/******************************************************************/
	// Wooden stand - left support
	DrawTexturedBox(
		glm::vec3(0.45f, 3.0f, 0.45f),
		glm::vec3(-7.25f, 3.0f, 2.5f),
		"wood"
	);
	SetShaderMaterial("wood");
	///******************************************************************/
	//Left Object - Wooden Stand Elongated Boxes Center Back Vertical
	/******************************************************************/


	// Wooden stand - center back support
	DrawTexturedBox(
		glm::vec3(.45f, 3.f, .45f),
		glm::vec3(-5.75f, 3.f, 4.f),
		"wood"
	);
	SetShaderMaterial("wood");
	///******************************************************************/
	//Left Object - Wooden Stand Elongated Boxes Right Vertical-
	/******************************************************************/


	// Wooden stand - right  support
	DrawTexturedBox(
		glm::vec3(.45f, 3.f, .45f),
		glm::vec3(-4.25f, 3.0f, 2.5f),
		"wood"
	);
	SetShaderMaterial("wood");
	///******************************************************************/
	//Left Object - Wooden Stand Elongated Boxes Front to Back Horizonal
	/******************************************************************/

	// Wooden stand - front to back support
	DrawTexturedBox(
		glm::vec3(.45f, .40f, 3.f),
		glm::vec3(-5.75f, 2.5f, 2.5f),
		"wood"
	);
	SetShaderMaterial("wood");
	///******************************************************************/
	//Left Object - Wooden Stand Elongated Boxes Left to Right Horizonal
	/******************************************************************/


	// Wooden stand - left to right support
	DrawTexturedBox(
		glm::vec3(2.5f, .40f, .45f),
		glm::vec3(-5.75f, 2.50f, 2.5f),
		"wood"
	);

	SetShaderMaterial("wood");
	///******************************************************************/
	//Left Object - Grey Cylinder 
	/******************************************************************/

	// set the XYZ scale for the Box
	scaleXYZ = glm::vec3(1.20f, 2.5f, 1.20f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	//set the position for the cylinder on top of the wooden stand
	positionXYZ = glm::vec3(-5.75f, 2.75f, 2.5f);

	// set the transformations into memory
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	//Set Texture to marble for cylinder
	SetShaderTexture("marble");
	SetTextureUVScale(1.0f, 1.0f);
	SetShaderMaterial("stone");
	//Set Color
	//SetShaderColor(.50f, 0.50f, 0.50f, 1.0f);

	//draw the box
	m_basicMeshes->DrawCylinderMesh();

	///******************************************************************/
	//Left Object - Rock Texture Cylinder Layer 
	/******************************************************************/

	// set the XYZ scale for the Box
	scaleXYZ = glm::vec3(.9f, .05f, .9f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	//set the position for the cylinder on top of the wooden stand
	positionXYZ = glm::vec3(-5.75f, 5.25f, 2.5f);

	// set the transformations into memory
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	//Set Texture
	SetShaderTexture("rock");
	SetTextureUVScale(1.0f, 1.0f);

	//Set Color
	//SetShaderColor(1.f, 1.f, 1.f, 1.0f);

	//draw the box
	m_basicMeshes->DrawCylinderMesh();

	///******************************************************************/
	//Left Object - Flattened  Torus  for Cylinder
	/******************************************************************/

	// set the XYZ scale for the Box
	scaleXYZ = glm::vec3(1.f, .5f, 1.f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	//set the position for the cylinder on top of the wooden stand
	positionXYZ = glm::vec3(-5.75f, 5.25f, 2.5f);

	// set the transformations into memory
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	//Set Color
	//SetShaderColor(0.50f, 0.50f, 0.50f, 1.0f);

	//Set Texture to marble for cylinder top turos
	SetShaderTexture("marble");
	SetTextureUVScale(1.0f, 1.0f);
	SetShaderMaterial("stone");
	//draw the box
	m_basicMeshes->DrawTorusMesh();

	/******************************************************************/
	// Green Plant - Main Stem
	/******************************************************************/

	// Thin cylinder that anchors all of the leaves
	scaleXYZ = glm::vec3(0.18f, 1.35f, 0.18f);

	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	// Starts inside the rock layer and extends upward
	positionXYZ = glm::vec3(-5.75f, 5.25f, 2.5f);

	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	// Set Leaf Color
	//SetShaderColor(0.10f, 0.32f, 0.08f, 1.0f);
	SetShaderTextures("leaf1", "ggreen");
	SetTextureUVScale(.5f, 1.f);

	m_basicMeshes->DrawCylinderMesh();

	// Helper for attaching each leaf to a specific point on the stem
	auto SetLeafAtStem = [&](glm::vec3 leafScale,
		float leafX,
		float leafY,
		float leafZ,
		glm::vec3 attachmentPoint)
		{
			// Match the same rotation order used by SetTransformations()
			glm::mat4 leafRotation =
				glm::rotate(glm::radians(leafZ), glm::vec3(0.0f, 0.0f, 1.0f)) *
				glm::rotate(glm::radians(leafY), glm::vec3(0.0f, 1.0f, 0.0f)) *
				glm::rotate(glm::radians(leafX), glm::vec3(1.0f, 0.0f, 0.0f));

			// Bottom of the stretched sphere in its local coordinates
			glm::vec3 leafBaseOffset =
				glm::vec3(leafRotation *
					glm::vec4(0.0f, -leafScale.y, 0.0f, 0.0f));

			// Calculate the exact center position needed to attach the leaf
			positionXYZ = attachmentPoint - leafBaseOffset;

			SetTransformations(
				leafScale,
				leafX,
				leafY,
				leafZ,
				positionXYZ);
		};
	/**************************************************************/
	// LEAF 1 - Back leaf
	/**************************************************************/
	scaleXYZ = glm::vec3(0.22f, 0.70f, 0.16f);

	XrotationDegrees = -30.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	// Attachment point on the stem
	glm::vec3 attachmentPoint = glm::vec3(-5.75f, 5.70f, 2.50f);

	SetLeafAtStem(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		attachmentPoint);

	// Set Leaf Color
	//SetShaderColor(0.10f, 0.32f, 0.08f, 1.0f);
	SetShaderTextures("leaf1", "ggreen");
	SetTextureUVScale(.5f, 1.f);


	m_basicMeshes->DrawSphereMesh();

	SetShaderMaterial("plant");

	/**************************************************************/
	// LEAF 2 - Top Back Leaf
	/**************************************************************/
	scaleXYZ = glm::vec3(0.22f, 0.75f, 0.16f);

	XrotationDegrees = 30.0f;
	YrotationDegrees = 180.0f;
	ZrotationDegrees = 0.0f;

	attachmentPoint = glm::vec3(-5.75f, 5.95f, 2.50f);

	SetLeafAtStem(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		attachmentPoint);

	// Set Leaf Color
	//SetShaderColor(0.10f, 0.32f, 0.08f, 1.0f);
	SetShaderTextures("leaf1", "ggreen");
	SetTextureUVScale(.5f, 1.f);
	SetShaderMaterial("plant");
	m_basicMeshes->DrawSphereMesh();


	/**************************************************************/
	// LEAF 3 - 2nd Right Front  Leaf 
	/**************************************************************/
	scaleXYZ = glm::vec3(0.24f, 0.75f, 0.17f);

	XrotationDegrees = -20.0f;
	YrotationDegrees = 45.0f;
	ZrotationDegrees = -55.0f;

	attachmentPoint = glm::vec3(-5.75f, 6.15f, 2.50f);

	SetLeafAtStem(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		attachmentPoint);

	// Set Leaf Color
	//SetShaderColor(0.10f, 0.32f, 0.08f, 1.0f);
	SetShaderTextures("leaf1", "ggreen");
	SetTextureUVScale(.5f, 1.f);
	SetShaderMaterial("plant");
	m_basicMeshes->DrawSphereMesh();


	/**************************************************************/
	// LEAF 4 - 2nd Left Front Leaf
	/**************************************************************/
	scaleXYZ = glm::vec3(0.24f, 0.60f, 0.17f);

	XrotationDegrees = -20.0f;
	YrotationDegrees = 45.0f;
	ZrotationDegrees = 55.0f;

	attachmentPoint = glm::vec3(-5.75f, 5.85f, 2.50f);

	SetLeafAtStem(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		attachmentPoint);

	// Set Leaf Color
	//SetShaderColor(0.10f, 0.32f, 0.08f, 1.0f);
	SetShaderTextures("leaf1", "ggreen");
	SetTextureUVScale(.5f, 1.f);
	SetShaderMaterial("plant");
	m_basicMeshes->DrawSphereMesh();


	/**************************************************************/
	// LEAF 5 - 1st Right Front Leaf
	/**************************************************************/
	scaleXYZ = glm::vec3(0.24f, 0.85f, 0.17f);

	XrotationDegrees = -20.0f;
	YrotationDegrees = 45.0f;
	ZrotationDegrees = -55.0f;

	attachmentPoint = glm::vec3(-5.75f, 6.40f, 2.50f);

	SetLeafAtStem(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		attachmentPoint);

	// Set Leaf Color
	//SetShaderColor(0.10f, 0.32f, 0.08f, 1.0f);
	SetShaderTextures("leaf1", "ggreen");
	SetTextureUVScale(.5f, 1.f);
	SetShaderMaterial("plant");
	m_basicMeshes->DrawSphereMesh();


	/**************************************************************/
	// LEAF 6 - 1st Left Front leaf
	/**************************************************************/
	scaleXYZ = glm::vec3(0.24f, 0.65f, 0.17f);

	XrotationDegrees = -20.0f;
	YrotationDegrees = 135.0f;
	ZrotationDegrees = 55.0f;

	attachmentPoint = glm::vec3(-5.75f, 6.30f, 2.50f);

	SetLeafAtStem(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		attachmentPoint);

	// Set Leaf Color
	//SetShaderColor(0.10f, 0.32f, 0.08f, 1.0f);
	SetShaderTextures("leaf1", "ggreen");
	SetTextureUVScale(.5f, 1.f);
	SetShaderMaterial("plant");
	m_basicMeshes->DrawSphereMesh();


	/**************************************************************/
	// LEAF 7 - 2nd Right Leaf point toward back
	/**************************************************************/
	scaleXYZ = glm::vec3(0.23f, 0.70f, 0.17f);

	XrotationDegrees = 20.0f;
	YrotationDegrees = 135.0f;
	ZrotationDegrees = -40.0f;

	attachmentPoint = glm::vec3(-5.75f, 5.80f, 2.50f);

	SetLeafAtStem(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		attachmentPoint);

	// Set Leaf Color
	//SetShaderColor(0.10f, 0.32f, 0.08f, 1.0f);
	SetShaderTextures("leaf1", "ggreen");
	SetTextureUVScale(.5f, 1.f);
	SetShaderMaterial("plant");
	m_basicMeshes->DrawSphereMesh();


	/**************************************************************/
	// LEAF 8 - 2nd Left Leaf pointing back
	/**************************************************************/
	scaleXYZ = glm::vec3(0.23f, 0.65f, 0.17f);

	XrotationDegrees = 35.0f;
	YrotationDegrees = 225.0f;
	ZrotationDegrees = 45.0f;

	attachmentPoint = glm::vec3(-5.75f, 6.05f, 2.50f);

	SetLeafAtStem(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		attachmentPoint);

	// Set Leaf Color
	//SetShaderColor(0.10f, 0.32f, 0.08f, 1.0f);
	SetShaderTextures("leaf1", "ggreen");
	SetTextureUVScale(.5f, 1.f);
	SetShaderMaterial("plant");
	m_basicMeshes->DrawSphereMesh();


	/**************************************************************/
	// LEAF 9 - 4th Bottom Front Right Leaf
	/**************************************************************/
	scaleXYZ = glm::vec3(0.22f, 0.75f, 0.16f);

	XrotationDegrees = -35.0f;
	YrotationDegrees = 315.0f;
	ZrotationDegrees = -65.0f;

	attachmentPoint = glm::vec3(-5.75f, 5.55f, 2.50f);

	SetLeafAtStem(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		attachmentPoint);

	// Set Leaf Color
	//SetShaderColor(0.10f, 0.32f, 0.08f, 1.0f);
	SetShaderTextures("leaf1", "ggreen");
	SetTextureUVScale(.5f, 1.f);
	SetShaderMaterial("plant");
	m_basicMeshes->DrawSphereMesh();


	/**************************************************************/
	// LEAF 10 - Back Left Top point back 
	/**************************************************************/
	scaleXYZ = glm::vec3(0.24f, 0.70f, 0.17f);

	XrotationDegrees = -15.0f;
	YrotationDegrees = 315.0f;
	ZrotationDegrees = 65.0f;

	attachmentPoint = glm::vec3(-5.75f, 6.20f, 2.50f);

	SetLeafAtStem(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		attachmentPoint);

	// Set Leaf Color
	//SetShaderColor(0.10f, 0.32f, 0.08f, 1.0f);
	SetShaderTextures("leaf1", "ggreen");
	SetTextureUVScale(.5f, 1.f);
	SetShaderMaterial("plant");
	m_basicMeshes->DrawSphereMesh();
	/**************************************************************/
	// LEAF 11 - 4th Right Leaf Point Back
	/**************************************************************/
	scaleXYZ = glm::vec3(0.23f, 0.70f, 0.17f);

	XrotationDegrees = -25.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = -65.0f;

	// Lower attachment point on the stem
	attachmentPoint = glm::vec3(-5.75f, 5.45f, 2.50f);

	SetLeafAtStem(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		attachmentPoint);

	// Set Leaf Color
	//SetShaderColor(0.10f, 0.32f, 0.08f, 1.0f);
	SetShaderTextures("leaf1", "ggreen");
	SetTextureUVScale(.5f, 1.f);
	SetShaderMaterial("plant");
	m_basicMeshes->DrawSphereMesh();


	/**************************************************************/
	// LEAF 12 - 4th Left Leaf Front 
	//
	/**************************************************************/
	scaleXYZ = glm::vec3(0.22f, 0.85f, 0.16f);

	XrotationDegrees = -25.0f;
	YrotationDegrees = 180.0f;
	ZrotationDegrees = 65.0f;

	// Attach directly to the top of the stem
	attachmentPoint = glm::vec3(-5.75f, 5.45f, 2.50f);

	SetLeafAtStem(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		attachmentPoint);

	// Set Leaf Color
	//SetShaderColor(0.10f, 0.32f, 0.08f, 1.0f);
	SetShaderTextures("leaf1", "ggreen");
	SetTextureUVScale(.5f, 1.f);
	SetShaderMaterial("plant");
	m_basicMeshes->DrawSphereMesh();

	/**************************************************************/
	// LEAF 13 - TOP CENTER
	// Comes straight up from the top of the stem
	/**************************************************************/
	scaleXYZ = glm::vec3(0.22f, 0.80f, 0.16f);

	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	// Attach slightly lower than the point of the stem
	attachmentPoint = glm::vec3(-5.75f, 6.25f, 2.58f);

	SetLeafAtStem(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		attachmentPoint);

	// Set Leaf Color
	//SetShaderColor(0.10f, 0.32f, 0.08f, 1.0f);
	SetShaderTextures("leaf1", "ggreen");
	SetTextureUVScale(.5f, 1.f);
	SetShaderMaterial("plant");
	m_basicMeshes->DrawSphereMesh();



	/**************************************************************/
	// BLACK PLANTER - BOX MESH
	// Positioned in front of the right wooden support.
	/**************************************************************/

	// Planter body: a square black box.
	scaleXYZ = glm::vec3(2.55f, 2.55f, 2.55f);

	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	// The right wooden support is at x = -4.25, z = 2.5.
	// This moves the black planter slightly right and forward.
	positionXYZ = glm::vec3(-2.65f, 2.25f, 5.0f);

	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	// Matte black plastic with scene lighting.
	SetShaderColor(0.035f, 0.030f, 0.035f, 1.0f);
	SetShaderMaterial("wall");
	
	m_basicMeshes->DrawBoxMesh();


	/**************************************************************/
	// BLACK PLANTER - SOIL
	// A dark, flattened box placed inside the planter opening.
	/**************************************************************/

	//scaleXYZ = glm::vec3(1.53f, .05f, 1.53f);

	//positionXYZ = glm::vec3(-2.65f, 2.55f, 4.0f);

	//SetTransformations(
		//scaleXYZ,
		//XrotationDegrees,
		//YrotationDegrees,
		//ZrotationDegrees,
		//positionXYZ);

	//SetShaderTexture("soil");
	//SetShaderMaterial("stone");

	//m_basicMeshes->DrawBoxMesh();


	



    /**************************************************************/
    // BLACK PLANTER SUCCULENT - CENTER STEM
    // A narrow green cylinder supports the surrounding leaves.
    /**************************************************************/

    scaleXYZ = glm::vec3(0.10f, 1.5f, 0.10f);

    XrotationDegrees = 0.0f;
    YrotationDegrees = 0.0f;
    ZrotationDegrees = 0.0f;

    // Centered beneath the main leaf.
    positionXYZ = glm::vec3(-3.65f, 2.45f, 4.0f);

    SetTransformations(
        scaleXYZ,
        XrotationDegrees,
        YrotationDegrees,
        ZrotationDegrees,
        positionXYZ);

    SetShaderColor(0.10f, 0.34f, 0.12f, 1.0f);
    SetShaderMaterial("plant");

    m_basicMeshes->DrawCylinderMesh();



	/**************************************************************/
	// SUCCULENT LEAVES - SEVEN-LEAF ROSETTE
	// Each flattened sphere is positioned around the center stem.
	/**************************************************************/

	// Leaf 1 - Center leaf (your original coordinates)
	scaleXYZ = glm::vec3(0.42f, 0.35f, 0.40f);

	SetTransformations(
		scaleXYZ,
		0.0f, 0.0f, 0.0f,
		glm::vec3(-3.65f, 3.70f, 4.0f));

	SetShaderColor(0.12f, 0.48f, 0.16f, 1.0f);
	SetShaderMaterial("plant");
	m_basicMeshes->DrawSphereMesh();


	// Leaf 2 - Front-left
	scaleXYZ = glm::vec3(0.36f, 0.25f, 0.32f);

	SetTransformations(
		scaleXYZ,
		10.0f, 0.0f, -25.0f,
		glm::vec3(-3.91f, 3.56f, 4.22f));

	SetShaderColor(0.18f, 0.55f, 0.20f, 1.0f);
	SetShaderMaterial("plant");
	m_basicMeshes->DrawSphereMesh();


	// Leaf 3 - Front-right
	scaleXYZ = glm::vec3(0.36f, 0.25f, 0.32f);

	SetTransformations(
		scaleXYZ,
		-10.0f, 0.0f, 25.0f,
		glm::vec3(-3.39f, 3.46f, 4.22f));

	SetShaderColor(0.16f, 0.51f, 0.18f, 1.0f);
	SetShaderMaterial("plant");
	m_basicMeshes->DrawSphereMesh();


	// Leaf 4 - Back-left
	scaleXYZ = glm::vec3(0.36f, 0.25f, 0.32f);

	SetTransformations(
		scaleXYZ,
		15.0f, 0.0f, -30.0f,
		glm::vec3(-3.91f, 3.91f, 3.76f));

	SetShaderColor(0.10f, 0.42f, 0.14f, 1.0f);
	SetShaderMaterial("plant");
	m_basicMeshes->DrawSphereMesh();


	// Leaf 5 - Back-right
	scaleXYZ = glm::vec3(0.36f, 0.25f, 0.32f);

	SetTransformations(
		scaleXYZ,
		-15.0f, 0.0f, 30.0f,
		glm::vec3(-3.39f, 3.95f, 3.76f));

	SetShaderColor(0.14f, 0.47f, 0.16f, 1.0f);
	SetShaderMaterial("plant");
	m_basicMeshes->DrawSphereMesh();


	// Leaf 6 - Left outer leaf
	scaleXYZ = glm::vec3(0.34f, 0.23f, 0.30f);

	SetTransformations(
		scaleXYZ,
		0.0f, 0.0f, -40.0f,
		glm::vec3(-4.02f, 3.88f, 4.0f));

	SetShaderColor(0.20f, 0.58f, 0.22f, 1.0f);
	SetShaderMaterial("plant");
	m_basicMeshes->DrawSphereMesh();


	// Leaf 7 - Right outer leaf
	scaleXYZ = glm::vec3(0.34f, 0.23f, 0.30f);

	SetTransformations(
		scaleXYZ,
		0.0f, 0.0f, 40.0f,
		glm::vec3(-3.28f, 3.88f, 4.0f));

	SetShaderColor(0.17f, 0.53f, 0.19f, 1.0f);
	SetShaderMaterial("plant");
	m_basicMeshes->DrawSphereMesh();

	/**************************************************************/
	// PICTURE FRAME - OUTER DARK BORDER
	// Four textured boxes form the top, bottom, left, and right.
	/**************************************************************/

	
	/**************************************************************/
	// PICTURE FRAME - CONNECTED OUTER BORDER
	/**************************************************************/
	/**************************************************************/
	// TOP RAIL - INDIVIDUAL FACE TEXTURES
	/**************************************************************/

	// Set the size and position of the top rail
	scaleXYZ = glm::vec3(8.50f, .90f, 1.f),
	positionXYZ = glm::vec3(2.95f, 8.46f, 4.50f),

	// Apply the rail's transformation
	SetTransformations(scaleXYZ,0.0f, 160.0f, 0.0f,positionXYZ);

	// Back face = frame
	SetShaderTexture("frame");
	m_basicMeshes->DrawBoxMeshSide(ShapeMeshes::box_back);

	// Bottom face = frame
	SetShaderTexture("paper");
	m_basicMeshes->DrawBoxMeshSide(ShapeMeshes::box_bottom);

	// Left face = frame
	SetShaderTexture("frame");
	m_basicMeshes->DrawBoxMeshSide(ShapeMeshes::box_left);

	// Right face = stone_light
	SetShaderTexture("frame");
	m_basicMeshes->DrawBoxMeshSide(ShapeMeshes::box_right);

	// Top face = frame
	SetShaderTexture("frame");
	m_basicMeshes->DrawBoxMeshSide(ShapeMeshes::box_top);

	// Front face = frame
	SetShaderTexture("frame");
	m_basicMeshes->DrawBoxMeshSide(ShapeMeshes::box_front);

	
	/**************************************************************/
	// BOTTOM RAIL - INDIVIDUAL FACE TEXTURES
	/**************************************************************/

	// Set the size and position of the bottom rail
	scaleXYZ = glm::vec3(8.50f, .90f, 1.f),
	positionXYZ = glm::vec3(2.95f, 1.85f, 4.5f),

	// Apply the rail's transformation
	SetTransformations(scaleXYZ,0.0f, 160.0f, 0.0f,positionXYZ);

	// Back face = frame
	SetShaderTexture("frame");
	m_basicMeshes->DrawBoxMeshSide(ShapeMeshes::box_back);

	// Bottom face = frame
	SetShaderTexture("frame");
	m_basicMeshes->DrawBoxMeshSide(ShapeMeshes::box_bottom);

	// Left face = frame
	SetShaderTexture("frame");
	m_basicMeshes->DrawBoxMeshSide(ShapeMeshes::box_left);

	// Right face = stone_light
	SetShaderTexture("frame");
	m_basicMeshes->DrawBoxMeshSide(ShapeMeshes::box_right);

	// Top face = frame
	SetShaderTexture("paper");
	m_basicMeshes->DrawBoxMeshSide(ShapeMeshes::box_top);

	// Front face = frame
	SetShaderTexture("frame");
	m_basicMeshes->DrawBoxMeshSide(ShapeMeshes::box_front);

	
	/**************************************************************/
	// LEFT RAIL - INDIVIDUAL FACE TEXTURES
	/**************************************************************/

	// Set the size and position of the left rail
	scaleXYZ = glm::vec3(0.90f, 8.18f, 1.f),
	positionXYZ = glm::vec3(-1.25f, 4.85f, 2.95f),
	
	// Apply the rail's transformation
	SetTransformations(scaleXYZ,0.0f, 340.0f, 0.0f,positionXYZ);

	// Back face = frame
	SetShaderTexture("frame");
	m_basicMeshes->DrawBoxMeshSide(ShapeMeshes::box_back);

	// Bottom face = frame
	SetShaderTexture("frame");
	m_basicMeshes->DrawBoxMeshSide(ShapeMeshes::box_bottom);

	// Left face = frame
	SetShaderTexture("frame");
	m_basicMeshes->DrawBoxMeshSide(ShapeMeshes::box_left);

	// Right face = stone_light
	SetShaderTexture("paper");
	m_basicMeshes->DrawBoxMeshSide(ShapeMeshes::box_right);

	// Top face = frame
	SetShaderTexture("frame");
	m_basicMeshes->DrawBoxMeshSide(ShapeMeshes::box_top);

	// Front face = frame
	SetShaderTexture("frame");
	m_basicMeshes->DrawBoxMeshSide(ShapeMeshes::box_front);


	/**************************************************************/
	// RIGHT RAIL - INDIVIDUAL FACE TEXTURES
	/**************************************************************/

	// Set the size and position of the right rail
	scaleXYZ = glm::vec3(0.90f, 8.18f, 1.f),
	positionXYZ = glm::vec3(6.55f, 4.85f, 5.80f);

	// Apply the rail's transformation
	SetTransformations(scaleXYZ,0.0f, 160.0f, 0.0f,positionXYZ);

	// Back face = frame
	SetShaderTexture("frame");
	m_basicMeshes->DrawBoxMeshSide(ShapeMeshes::box_back);

	// Bottom face = frame
	SetShaderTexture("frame");
	m_basicMeshes->DrawBoxMeshSide(ShapeMeshes::box_bottom);

	// Left face = frame
	SetShaderTexture("paper");
	m_basicMeshes->DrawBoxMeshSide(ShapeMeshes::box_left);

	// Right face = stone_light
	SetShaderTexture("frame");
	m_basicMeshes->DrawBoxMeshSide(ShapeMeshes::box_right);

	// Top face = frame
	SetShaderTexture("frame");
	m_basicMeshes->DrawBoxMeshSide(ShapeMeshes::box_top);

	// Front face = frame
	SetShaderTexture("frame");
	m_basicMeshes->DrawBoxMeshSide(ShapeMeshes::box_front);

	/**************************************************************/
	// PICTURE FRAME - SUCCULENT ARTWORK
	// The picture.jpg texture is applied to the front of a thin box.
	/**************************************************************/

	SetTransformations(
		glm::vec3(8.5f, 8.f, 0.025f),
		0.0f, -20.0f, 0.0f,
		glm::vec3(2.95f, 4.85f, 4.f));
	// The picture texture supplies the image; the shader supplies lighting.
	
	SetShaderMaterial("stone");
	SetShaderColor(1.f, 1.f, 1.f, 1.0f);
	m_basicMeshes->DrawBoxMesh();

	///******************************************************************/
	//Right Object - Rock Texture Cylinder Layer 
	/******************************************************************/

	// set the XYZ scale for the Box
	scaleXYZ = glm::vec3(1.20f, 2.5f, 1.20f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	//set the position for the cylinder on top of the wooden stand
	positionXYZ = glm::vec3(8.75f, 2.55f, 6.5f);

	// set the transformations into memory
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	//Set Texture to marble for cylinder
	SetShaderTexture("marble");
	SetTextureUVScale(1.0f, 1.0f);
	SetShaderMaterial("stone");
	//Set Color
	//SetShaderColor(.50f, 0.50f, 0.50f, 1.0f);

	//draw the box
	m_basicMeshes->DrawCylinderMesh();












}

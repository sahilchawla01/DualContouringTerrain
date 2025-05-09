#pragma once
#include <memory>
#include <vector>

#include <Enums/EShaderOption.h>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include "UActorComponent.h"

class AActor;
class Shader;

class UMeshComponent : public UActorComponent
{
public:

	UMeshComponent(const std::vector<float>& vertices, const std::vector<float>& normals, const std::vector<unsigned int>& indices, const std::vector<float>& colors, const std::weak_ptr<const AActor> owningActor);
	~UMeshComponent() override;

public:

	std::vector<float> GetVertices() const;

	//IMP!! This is called from AActor when AActor::Init() is called | Sets up buffers and shaders
	void Init(EShaderOption shaderOption);
	void SetObjectColor(glm::vec3 color);
	void SetObjectColor(glm::vec4 color);
	virtual void Render();

protected:

	//Buffer Object ids
	unsigned int VAO;
	unsigned int vertices_VBO;
	unsigned int normal_VBO;
	unsigned int colors_VBO;
	unsigned int EBO;

	// MESH DETAILS
	std::vector<float> vertices;
	std::vector<float> normals;
	std::vector<float> colors;
	std::vector<unsigned int> indices;
	//By default, the object color is white
	glm::vec4 objectColor = glm::vec4(1.f);

	//MISC:
	std::unique_ptr<Shader> currentShader;
	EShaderOption e_currentShaderOption;

protected:
	virtual void SetupBuffers();
	virtual void SetupShader(EShaderOption shaderOption);
	virtual void UseShader();


private:
	std::weak_ptr<AActor> ownerActor; 

};

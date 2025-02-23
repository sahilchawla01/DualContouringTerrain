#pragma once

#include <memory>
#include <string>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

enum class EShaderOption;
class Shader;
class ACamera;
class UMeshComponent;

//This is a base class for all those entities that have a 3D transform
class AActor : public std::enable_shared_from_this<AActor>
{

public:
	AActor(const std::string& name, const std::weak_ptr<ACamera> currentCamera, const glm::vec3& model_position = glm::vec3(0.0f), const glm::vec3& model_scale = glm::vec3(1.f));
	virtual ~AActor();

public:
	// GETTERS
	std::vector<float> GetVertices() const;
	glm::mat4 GetModelMatrix() const;

	//Creates and sets shaders, and buffers for mesh component
	void SetupMeshComponent(EShaderOption e_shaderOption, const std::vector<float>& model_vertices, const std::vector<float>& model_normals, const std::vector<unsigned int>& model_indices);
	virtual void Render();

private:
	std::string actorName;

protected:

	// TRANSFORM
	glm::mat4 m_modelTransformMatrix;

	//MISC
	std::weak_ptr<ACamera> currCamera;

	//COMPONENTS
	std::shared_ptr<UMeshComponent> meshComponent;


protected:

	glm::mat4 GetMVPMatrix() const;
	glm::mat4 GetModelViewMatrix() const;


	friend class UActorComponent;
	friend class UMeshComponent;
};


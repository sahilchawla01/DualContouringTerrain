#pragma once

#include <memory>
#include <string>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class USDFComponent;
enum class EShaderOption;
class Shader;
class ACamera;
class UMeshComponent;

//This is a base class for all those entities that have a 3D transform
class AActor : public std::enable_shared_from_this<AActor>
{

public:
	AActor(const std::string& name, const std::weak_ptr<ACamera> currentCamera, const glm::vec3& model_position = glm::vec3(0.0f), const glm::vec3& model_scale = glm::vec3(1.f), const glm::vec3& model_rotation = glm::vec3(0.f));
	virtual ~AActor();

public:
	// -- TRANSFORM RELATED FUNCTIONS --
	void SetWorldPosition(const glm::vec3& worldPosition);
	void SetWorldRotation(const glm::vec3& worldRotation);
	void SetWorldRotation(const glm::mat4& worldRotationMatrix);

	// GETTERS
	const glm::vec3 GetWorldPosition() const { return m_worldPosition; }
	std::vector<float> GetVertices() const;
	glm::mat4 GetModelMatrix() const;

	//COMPONENT GETTERS
	std::weak_ptr<USDFComponent> GetSDFComponent() const;

	//Creates and sets shaders, and buffers for mesh component
	void SetupMeshComponent(EShaderOption e_shaderOption, const std::vector<float>& model_vertices, const std::vector<float>& model_normals, const std::vector<unsigned int>& model_indices, const std::vector<float>& model_colors = std::vector<float>{});
	//Add the SDF component 
	void SetupSDFComponent();

	virtual void Render();

private:
	std::string actorName;

protected:

	// TRANSFORM
	glm::mat4 m_modelTransformMatrix;
	glm::vec3 m_worldPosition;
	// Stored as radian
	glm::vec3 m_worldRotation;
	glm::vec3 m_worldScale;

	//MISC
	std::weak_ptr<ACamera> currCamera;

	//COMPONENTS
	std::shared_ptr<UMeshComponent> meshComponent;
	std::shared_ptr<USDFComponent> sdfComponent;


protected:

	glm::mat4 GetMVPMatrix() const;
	glm::mat4 GetModelViewMatrix() const;


	friend class UActorComponent;
	friend class UMeshComponent;
};


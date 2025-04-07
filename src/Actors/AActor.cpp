#include <Actors/AActor.h>
#include <Actors/ACamera.h>
#include <Helpers/Shader.h>

#include "Components/UMeshComponent.h"
#include "Components/USDFComponent.h"

AActor::AActor(const std::string& name, const std::weak_ptr<ACamera> currentCamera , const glm::vec3& model_position, const glm::vec3& model_scale, const glm::vec3& model_rotation)
{
	this->actorName = name;
	this->currCamera = currentCamera;

	//Store transforms
	this->m_worldPosition = model_position;
	this->m_worldScale = model_scale;
	//Store rotation in the form of radians
	this->m_worldRotation = glm::vec3(glm::radians(model_rotation.x), glm::radians(model_rotation.y), glm::radians(model_rotation.z));


	//Setup transform matrix
	m_modelTransformMatrix = glm::mat4(1.f);
	//Translate the model matrix
	m_modelTransformMatrix = glm::translate(m_modelTransformMatrix, model_position);
	//Rotate the matrix in the X,Y,Z axis
	m_modelTransformMatrix = glm::rotate(m_modelTransformMatrix, this->m_worldRotation.x, glm::vec3(1.0f, 0, 0));
	m_modelTransformMatrix = glm::rotate(m_modelTransformMatrix, this->m_worldRotation.y, glm::vec3(0, 1.0f, 0));
	m_modelTransformMatrix = glm::rotate(m_modelTransformMatrix, this->m_worldRotation.z, glm::vec3(0, 0, 1.0f));
	//Scale the model matrix
	m_modelTransformMatrix = glm::scale(m_modelTransformMatrix, model_scale);
	
}


std::vector<float> AActor::GetVertices() const
{
	if (!meshComponent)
	{
		std::cout<<"\nERROR | " << this->actorName << ": Mesh component null when getting vertices\n";
		return {};
	}

	return meshComponent->GetVertices();
	
}

glm::mat4 AActor::GetModelMatrix() const
{
	return m_modelTransformMatrix;
}

std::weak_ptr<USDFComponent> AActor::GetSDFComponent() const
{
	return sdfComponent;
}

glm::mat4 AActor::GetMVPMatrix() const
{
	//If camera doesn't exist, HUUUUGE error
	if (currCamera.expired())
	{
		std::cout << "\nERROR | " << this->actorName << " camera weak_ptr in getting MVP Matrix function expired\n";
		return glm::mat4(1.0);
	}

	std::shared_ptr<ACamera> curCamera{ currCamera.lock() };

	return curCamera->GetProjectionMatrix() * curCamera->GetViewMatrix() * m_modelTransformMatrix;

}

glm::mat4 AActor::GetModelViewMatrix() const
{
	//If camera doesn't exist, HUUUUGE error
	if (currCamera.expired())
	{
		std::cout << "\nERROR | " << this->actorName << " camera weak_ptr in getting Model View Matrix function expired\n";
		return glm::mat4(1.0);
	}

	std::shared_ptr<ACamera> curCamera{ currCamera.lock() };

	return curCamera->GetViewMatrix() * m_modelTransformMatrix;
}


void AActor::Render()
{
	if (meshComponent)
	{
		meshComponent->Render();
	} else
	{
		std::cout << "\nWARNING | " << this->actorName << " doesn't have an attached mesh component when calling Render()\n";
	}
	

}


AActor::~AActor()
{

}

//Sets world position of the model matrix directly, scaled appropriately 
void AActor::SetWorldPosition(const glm::vec3& worldPosition)
{

	//Reset translation part of model matrix
	m_modelTransformMatrix[3][0] = worldPosition.x;
	m_modelTransformMatrix[3][1] = worldPosition.y;
	m_modelTransformMatrix[3][2] = worldPosition.z;

	//Store new position
	this->m_worldPosition = worldPosition;
}

void AActor::SetWorldRotation(const glm::vec3& worldRotation)
{
	//Store rotation in the form of radians
	this->m_worldRotation = glm::vec3(glm::radians(worldRotation.x), glm::radians(worldRotation.y), glm::radians(worldRotation.z));

	//Unset the rotation
	m_modelTransformMatrix = glm::mat4(1.0);

	//Re-position the matrix
	m_modelTransformMatrix = glm::translate(m_modelTransformMatrix, m_worldPosition);

	//Rotate the model matrix w.r.t X, Y and Z axis respectively
	m_modelTransformMatrix = glm::rotate(m_modelTransformMatrix, this->m_worldRotation.z, glm::vec3(0, 0, 1.0f));
	m_modelTransformMatrix = glm::rotate(m_modelTransformMatrix, this->m_worldRotation.y, glm::vec3(0, 1.0f, 0));
	m_modelTransformMatrix = glm::rotate(m_modelTransformMatrix, this->m_worldRotation.x, glm::vec3(1.0f, 0, 0));

	//Re-scale the matrix
	m_modelTransformMatrix = glm::scale(m_modelTransformMatrix, m_worldScale);

}

void AActor::SetWorldRotation(const glm::mat4& worldRotationMatrix)
{
	m_modelTransformMatrix = glm::mat4(1.0f);

	//Re-position the matrix
	m_modelTransformMatrix = glm::translate(m_modelTransformMatrix, m_worldPosition);
	//Rotate the model matrix
	m_modelTransformMatrix *= worldRotationMatrix;
	//Re-scale the matrix
	m_modelTransformMatrix = glm::scale(m_modelTransformMatrix, m_worldScale);

	//TODO: Store rotation

}

void AActor::SetupMeshComponent(EShaderOption e_shaderOption, const std::vector<float>& model_vertices,
                                const std::vector<float>& model_normals, const std::vector<unsigned int>& model_indices, const std::vector<float>& model_colors)
{
	//Create mesh and attach mesh component
	meshComponent = std::make_shared<UMeshComponent>(model_vertices, model_normals, model_indices, model_colors, shared_from_this());

	meshComponent->Init(e_shaderOption);

}

void AActor::SetupSDFComponent()
{
	sdfComponent = std::make_shared<USDFComponent>(shared_from_this());
}


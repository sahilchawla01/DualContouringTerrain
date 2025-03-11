#include <Actors/AActor.h>
#include <Actors/ACamera.h>
#include <Helpers/Shader.h>

#include "Components/UMeshComponent.h"
#include "Components/USDFComponent.h"

AActor::AActor(const std::string& name, const std::weak_ptr<ACamera> currentCamera , const glm::vec3& model_position, const glm::vec3& model_scale)
{
	this->actorName = name;
	this->currCamera = currentCamera;

	//Setup transform matrix
	m_modelTransformMatrix = glm::mat4(1.f);
	m_modelTransformMatrix = glm::translate(m_modelTransformMatrix, model_position);
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


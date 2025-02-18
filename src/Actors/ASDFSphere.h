#pragma once
#include "AActor.h"

class ACamera;

class ASDFSphere : public AActor
{
public:
	ASDFSphere(const std::string& name, const std::vector<float>& model_vertices, const std::vector<float>& model_normals, const std::weak_ptr<ACamera> currentCamera, const glm::vec3& model_position = glm::vec3(0.0f), const glm::vec3& model_scale = glm::vec3(1.f)) : AActor(name, model_vertices, model_normals, currentCamera, model_position, model_scale) {}

	ASDFSphere(const std::string& name, const std::vector<float>& model_vertices, const std::vector<float>& model_normals, const std::vector<unsigned int>& model_indices, const std::weak_ptr<ACamera> currentCamera, const glm::vec3& model_position = glm::vec3(0.0f), const glm::vec3& model_scale = glm::vec3(1.f)) : AActor(name, model_vertices, model_normals, model_indices, currentCamera, model_position, model_scale) {}


	virtual ~ASDFSphere() = default;

private:
	void SetupShader() override;
	void UseShader() override;

};

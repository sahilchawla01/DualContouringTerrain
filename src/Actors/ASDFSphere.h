#pragma once
#include "AActor.h"

class ACamera;

class ASDFSphere : AActor
{
public:
	ASDFSphere(const std::string& name, const std::vector<glm::vec3>& model_vertices, const std::vector<glm::vec3>& model_normals, const glm::vec3& model_position = glm::vec3(0.0f), const glm::vec3& model_scale = glm::vec3(1.f), const std::weak_ptr<ACamera> currentCamera) : AActor(name, model_vertices, model_normals, model_position, model_scale, currentCamera) {}

	ASDFSphere(const std::string& name, const std::vector<glm::vec3>& model_vertices, const std::vector<glm::vec3>& model_normals, const std::vector<unsigned int>& model_indices, const glm::vec3& model_position = glm::vec3(0.0f), const glm::vec3& model_scale = glm::vec3(1.f), const std::weak_ptr<ACamera> currentCamera) : AActor(name, model_vertices, model_normals, model_indices, model_position, model_scale, currentCamera) {}


	virtual ~ASDFSphere() = default;

private:
	void SetupShader() override;
	void UseShader() override;

};

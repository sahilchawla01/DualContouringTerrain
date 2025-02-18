#pragma once

#include <memory>
#include <string>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class Shader;
class ACamera;

//This is a base class for all those entities that have a 3D transform
//IMP!! Call Init after object is created
class AActor
{
public:
	AActor(const std::string& name, const std::vector<float>& model_vertices, const std::vector<float>& model_normals, const std::weak_ptr<ACamera> currentCamera, const glm::vec3& model_position = glm::vec3(0.0f), const glm::vec3& model_scale = glm::vec3(1.f));
	AActor(const std::string& name, const std::vector<float>& model_vertices, const std::vector<float>& model_normals, const std::vector<unsigned int>& model_indices, const std::weak_ptr<ACamera> currentCamera, const glm::vec3& model_position = glm::vec3(0.0f), const glm::vec3& model_scale = glm::vec3(1.f));

	//Buffer Object ids
	unsigned int VAO;
	unsigned int vertices_VBO;
	unsigned int normal_VBO;
	unsigned int EBO;

public:

	//IMP!! Call after object is created | Initializes buffers, shaders, etc 
	void Init();
	glm::mat4 GetModelMatrix() const;
	virtual void Render();

private:
	std::string actorName; 

protected:
	// MESH DETAILS
	std::vector<float> vertices;
	std::vector<float> normals;
	std::vector<unsigned int> indices;

	// TRANSFORM
	glm::mat4 m_modelTransformMatrix;

	//MISC
	std::unique_ptr<Shader> currentShader;
	std::weak_ptr<ACamera> currCamera;


protected:

	glm::mat4 GetMVPMatrix() const;
	glm::mat4 GetModelViewMatrix() const;

	virtual void SetupBuffers();
	virtual void SetupShader();
	virtual void UseShader();

	virtual ~AActor();
};


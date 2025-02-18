#include <Actors/AActor.h>
#include <Actors/ACamera.h>
#include <glad/glad.h>
#include <Helpers/Shader.h>

AActor::AActor(const std::string& name, const std::vector<glm::vec3>& model_vertices, const std::vector<glm::vec3>& model_normals, const glm::vec3& model_position, const glm::vec3& model_scale, const std::weak_ptr<ACamera> currentCamera)
{
	this->actorName = name;
	this->vertices = model_vertices;
	this->normals = model_normals;
	this->currCamera = currentCamera;

	//Setup transform matrix
	m_modelTransformMatrix = glm::mat4(1.f);
	glm::translate(m_modelTransformMatrix, model_position);
	glm::scale(m_modelTransformMatrix, model_scale);

	
}

AActor::AActor(const std::string& name, const std::vector<glm::vec3>& model_vertices, const std::vector<glm::vec3>& model_normals,
	const std::vector<unsigned int>& model_indices, const glm::vec3& model_position, const glm::vec3& model_scale, const std::weak_ptr<ACamera> currentCamera)
{
	this->actorName = name;
	this->vertices = model_vertices;
	this->normals = model_normals;
	this->indices = model_indices;
	this->currCamera = currentCamera;

	//Setup transform matrix
	m_modelTransformMatrix = glm::mat4(1.f);
	glm::translate(m_modelTransformMatrix, model_position);
	glm::scale(m_modelTransformMatrix, model_scale);
	
}

void AActor::Init()
{
	SetupBuffers();

	SetupShader();
}

glm::mat4 AActor::GetModelMatrix() const
{
	return m_modelTransformMatrix;
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

void AActor::SetupBuffers()
{
	bool bShouldSetupEBO = !(this->indices.empty());
	bool bShouldBindNormals = !(this->normals.empty());

	//Generate Vertex Array Object
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &vertices_VBO);
	if (bShouldBindNormals) glGenBuffers(1, &normal_VBO);
	//Generate Element Buffer Object
	if (bShouldSetupEBO) glGenBuffers(1, &EBO);

	//Bind VAO
	glBindVertexArray(VAO);

	// 0. copy our vertices array in a buffer for OpenGL to use
	//Binds a buffer object to the current buffer type, only 1 can be set at one time
	glBindBuffer(GL_ARRAY_BUFFER, vertices_VBO);
	//Copy data to the buffer
	glBufferData(GL_ARRAY_BUFFER, this->vertices.size() * sizeof(float), this->vertices.data(), GL_STATIC_DRAW);
	// 1. Copy index array in an element buffer for OpenGL to use.
	if (bShouldSetupEBO)
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, this->indices.size() * sizeof(unsigned int), this->indices.data(), GL_STATIC_DRAW);
	}

	// 2. then set the vertex attributes pointers
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	if (bShouldBindNormals)
	{
		//Copy normal array in a buffer
		glBindBuffer(GL_ARRAY_BUFFER, normal_VBO);
		//Copy data to the buffer
		glBufferData(GL_ARRAY_BUFFER, this->normals.size() * sizeof(float), this->normals.data(), GL_STATIC_DRAW);
		// 2. then set the vertex attributes pointers
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
	}


	//Release buffer data

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	if (bShouldSetupEBO)
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glBindVertexArray(0);
}

void AActor::SetupShader()
{
	//Default implementation

	//Default shader is unlit
	currentShader = std::make_unique<Shader>(Shader("../../../src/Shaders/Test/test.vert", "../../../src/Shaders/Test/test.frag"));
}

void AActor::Render()
{

	const glm::mat4 mvp = GetMVPMatrix();

	UseShader();

	//Activate VAO
	glBindVertexArray(VAO);

	bool bShouldDrawEBO = !(this->indices.empty());

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // Normal shading

	if (bShouldDrawEBO)
	{

		glDrawElements(GL_TRIANGLES, static_cast<int>(indices.size()), GL_UNSIGNED_INT, 0);
		
	} else
	{
		glDrawArrays(GL_TRIANGLES, 0, static_cast<int>(vertices.size()) * 3);
	}

}

void AActor::UseShader()
{
	//Set shader uniforms
	currentShader->use();
	currentShader->setMat4("mvp", GetMVPMatrix());
	//By default, color is white
	currentShader->setVec3("color", glm::vec3(1.0, 1.0, 1.0));
}



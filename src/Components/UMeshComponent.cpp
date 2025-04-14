#include "UMeshComponent.h"
#include <glad/glad.h>

#include "Actors/AActor.h"
#include "Helpers/Shader.h"


UMeshComponent::UMeshComponent(const std::vector<float>& vertices, const std::vector<float>& normals,
                               const std::vector<unsigned int>& indices, const std::vector<float>& colors, const std::weak_ptr<const AActor>  owningActor) : UActorComponent(owningActor)
{
	this->vertices = vertices;
	this->normals = normals;
	this->indices = indices;
	this->colors = colors;
}

std::vector<float> UMeshComponent::GetVertices() const
{
	return this->vertices;
}

void UMeshComponent::Init(EShaderOption shaderOption)
{
	SetupBuffers();
	SetupShader(shaderOption);
}

void UMeshComponent::SetObjectColor(glm::vec3 color)
{
	objectColor = glm::vec4(color, 1.0f);
}

void UMeshComponent::SetObjectColor(glm::vec4 color)
{
	objectColor = color;
}

void UMeshComponent::Render()
{
	UseShader();

	//Activate VAO
	glBindVertexArray(VAO);

	bool bShouldDrawEBO = !(this->indices.empty());

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // Normal shading

	//std::cout << "\nVertices: " << vertices.size();
	//std::cout << "\nNormals: " << normals.size();

	if (bShouldDrawEBO)
	{

		glDrawElements(GL_TRIANGLES, static_cast<int>(indices.size()), GL_UNSIGNED_INT, 0);

	}
	else
	{
		glDrawArrays(GL_TRIANGLES, 0, static_cast<int>(vertices.size() / 3));
	}
}

void UMeshComponent::SetupBuffers()
{
	//Delete buffers in-case already setup
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &vertices_VBO);
	glDeleteBuffers(1, &normal_VBO);
	glDeleteBuffers(1, &colors_VBO);


	bool bShouldSetupEBO = !(this->indices.empty());
	bool bShouldBindNormals = !(this->normals.empty());
	bool bShouldBindColors = !(this->colors.empty());

	//Generate Vertex Array Object
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &vertices_VBO);
	if (bShouldBindNormals) glGenBuffers(1, &normal_VBO);
	//Generate VBO for colors
 	if (bShouldBindColors) glGenBuffers(1, &colors_VBO);
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

	if (bShouldBindColors)
	{
		//Copy normal array in a buffer
		glBindBuffer(GL_ARRAY_BUFFER, colors_VBO);
		//Copy data to the buffer
		glBufferData(GL_ARRAY_BUFFER, this->colors.size() * sizeof(float), this->colors.data(), GL_STATIC_DRAW);
		// 2. then set the vertex attributes pointers
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(2);
	}


	//Release buffer data

	glBindVertexArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void UMeshComponent::SetupShader(EShaderOption shaderOption)
{
	e_currentShaderOption = shaderOption;

	switch (shaderOption)
	{
		case EShaderOption::unlit:
		{
			currentShader = std::make_unique<Shader>("../../../src/Shaders/Test/test.vert", "../../../src/Shaders/Test/test.frag");
			break;
		}
		case EShaderOption::lit:
		{
			//Set current shader
			currentShader = std::make_unique<Shader>("../../../src/Shaders/simple-lit.vert", "../../../src/Shaders/simple-lit.frag");
			break;
		}
		case EShaderOption::flat_shade:
		{
			//Set current shader
			currentShader = std::make_unique<Shader>("../../../src/Shaders/flat-shade.vert", "../../../src/Shaders/flat-shade.frag");
			break;
		}
		default: //Default shader is unlit
		{
			currentShader = std::make_unique<Shader>("../../../src/Shaders/Test/test.vert", "../../../src/Shaders/Test/test.frag");
		}
	}
}


void UMeshComponent::UseShader()
{
	if (owningActor.expired())
	{
		std::cout << "UMeshComponent ERROR: On UseShader() call, owning actor didn't exist. Returning..";
		return;
	}

	//Get required uniform data
	glm::mat4 mvpMatrix = owningActor.expired() ? glm::mat4(1.f) : owningActor.lock()->GetMVPMatrix();
	glm::mat4 modelViewMatrix = owningActor.expired() ? glm::mat4(1.f) : owningActor.lock()->GetModelViewMatrix();


	//Set shader uniforms
	currentShader->use();

	switch (e_currentShaderOption)
	{
		case EShaderOption::unlit:
		{
			currentShader->setMat4("mvp", mvpMatrix);
			//By default, color is white
			currentShader->setVec4("color", objectColor);
			break;
		}
	    case EShaderOption::flat_shade:
		{
			currentShader->setMat4("mvp", mvpMatrix);
			break;
		}
		case EShaderOption::lit:
		{
			currentShader->setVec4("objectColor", objectColor);
			currentShader->setMat4("modelViewMatrix", modelViewMatrix);
			currentShader->setMat4("mvp", mvpMatrix);
			//Set shader values
			currentShader->setVec3("dirLight.direction", glm::vec3(-0.2f, -1.0f, -0.3f));
			currentShader->setVec3("dirLight.ambient", glm::vec3(1.0f, 1.0f, 1.0f));
			currentShader->setVec3("dirLight.diffuse", glm::vec3(0.4f, 0.4f, 0.4f));
			currentShader->setVec3("dirLight.specular", glm::vec3(0.6f, 0.6f, 0.6f));

			////Set material values
			currentShader->setFloat("mat.shine", 64.0f);
			currentShader->setVec3("mat.ambient", glm::vec3(1.0f, 1.0f, 1.0f));
			currentShader->setVec3("mat.diffuse", glm::vec3(1.0f, 1.0f, 1.0f));
			currentShader->setVec3("mat.specular", glm::vec3(0.5f, 0.5f, 0.5f));
			break;
		}
		default:	
		{
			currentShader->setMat4("mvp", mvpMatrix);
			//By default, color is white
			currentShader->setVec3("color", glm::vec3(1.0, 1.0, 1.0));
		}
	}

}

UMeshComponent::~UMeshComponent()
{
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &vertices_VBO);
	glDeleteBuffers(1, &normal_VBO);
	glDeleteBuffers(1, &colors_VBO);

}

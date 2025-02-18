#include "ASDFSphere.h"
#include "Helpers/Shader.h"

void ASDFSphere::SetupShader()
{
	//Set current shader
	currentShader = std::make_unique<Shader>("../../../src/Shaders/simple-lit.vert", "../../../src/Shaders/simple-lit.frag");

}

void ASDFSphere::UseShader()
{
	currentShader->use();
	currentShader->setVec3("objectColor", glm::vec3(1.0));
	currentShader->setMat4("modelViewMatrix", GetModelViewMatrix());
	currentShader->setMat4("mvp", GetMVPMatrix());
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


}

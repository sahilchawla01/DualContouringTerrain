#pragma once

#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>

#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class Shader
{
public:

	//The program ID
	unsigned int ID;

	//Constructor reads and builds the path || IMP!! Path is relative to the working directory of the project, i.e. the project directory
	Shader(const char* vertexPath, const char* fragmentPath);
	//Activate the shader
	void use();
	//Utility uniform functions
	void setBool(const std::string& name, bool value) const;
	void setFloat(const std::string& name, float value) const;
	void setInt(const std::string& name, int value) const;
	void setVec3(const std::string& name, glm::vec3 vectorToSet) const;
	void setVec4(const std::string& name, glm::vec4 vectorToSet) const;
	void setMat4(const std::string& name, glm::mat4 matrixToSet) const;

private:
private:
	// utility function for checking shader compilation/linking errors.
	// ------------------------------------------------------------------------
	void checkCompileErrors(unsigned int shader, std::string type);
};

#endif

